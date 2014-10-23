/**
 * @ingroup receptor
 *
 * @{
 * @file process.c
 * @brief implementation of ceptr processing: instructions and run tree reduction
 * @todo implement a way to define sys_processes input and output signatures
 *
 * @copyright Copyright (C) 2013-2014, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 */
#include "process.h"
#include "def.h"
#include "semtrex.h"
#include <stdarg.h>

/**
 * implements the INTERPOLATE_FROM_MATCH process
 *
 * replaces the interpolation tree with the matched sub-parts from a semtrex match results tree
 *
 * @param[in] t interpolation tree to be scanned for INTERPOLATE_SYMBOL nodes
 * @param[in] match_results SEMTREX_MATCH_RESULTS tree
 * @param[in] match_tree original tree that was matched (needed to grab the data to interpolate)
 * @todo what to do if match has sibs??
 */
void _p_interpolate_from_match(T *t,T *match_results,T *match_tree) {
    if (semeq(_t_symbol(t),INTERPOLATE_SYMBOL)) {
	Symbol s = *(Symbol *)_t_surface(t);
	T *m = _t_get_match(match_results,s);
	int *path = (int *)_t_surface(_t_child(m,2));
	int sibs = *(int*)_t_surface(_t_child(m,3));
	T *x = _t_get(match_tree,path);

	if (!x) {
	    raise_error0("expecting to get a value from match!!");
	}
	_t_morph(t,x);
    }
    DO_KIDS(t,_p_interpolate_from_match(_t_child(t,i),match_results,match_tree));
}

/**
 * check a group of parameters to see if they match a process input signature
 *
 * @param[in] defs definition trees needed for the checking
 * @param[in] p the Process we are checking against
 * @param[in] params list of parameters
 *
 * @returns Error code
  */
Error __p_check_signature(Defs defs,Process p,T *params) {
    T *def = _d_get_process_code(defs.processes,p);
    T *input = _t_child(def,4);
    int i = _t_children(input);
    int c = _t_children(params);
    if (i > c) return tooFewParamsReductionErr;
    if (i < c) return tooManyParamsReductionErr;
    for(i=1;i<=c;i++) {
	T *sig = _t_child(_t_child(input,i),1);
	if(semeq(_t_symbol(sig),SIGNATURE_STRUCTURE)) {
	    Structure ss = *(Symbol *)_t_surface(sig);
	    if (!semeq(_d_get_symbol_structure(defs.symbols,_t_symbol(_t_child(params,i))),ss) && !semeq(ss,TREE))
		return badSignatureReductionErr;
	}
	else {
	    raise_error("unknown signature checking symbol: %s",_d_get_symbol_name(0,_t_symbol(sig)));
	}
    }
    return 0;
}

/**
 * reduce a run tree by executing the instructions in it and replacing the tree values in place
 *
 * a run_tree is expected to have a code tree as the first child, and parameters as the second
 *
 * @param[in] processes context of defined processes
 * @param[in] run_tree the run tree being reduced
 * @param[in] code the current node in the run tree we are on (for recursion)
 * @returns Error status of the reduction
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/process_spec.h testProcessReduceDefinedProcess
 */
Error __p_reduce(Defs defs,T *run_tree, T *code) {
    Process s = _t_symbol(code);

    T *param,*match_results,*match_tree,*t,*x;

    T *parent = _t_parent(code);
    int idx = _t_node_index(code);

    if (semeq(s,PARAM_REF)) {
	param = _t_get(run_tree,(int *)_t_surface(code));
	if (!param) {
	    raise_error0("request for non-existent param");
	}
	x = _t_clone(param);
	_t_replace(parent,idx,x);
	code = x;
	s = _t_symbol(code);
    }

    // if this isn't a process then we've reduced the tree
    if (!is_process(s)) return;

    // otherwise, first reduce all the children
    DO_KIDS(code,__p_reduce(defs,run_tree,_t_child(code,i)));

    // then, if this isn't a basic system level process, it's the equivalent
    // of a function call, so we need to create a new execution context (RUN_TREE)
    // for the process, which clones the code and adds in the params, and then
    // recursively reduce the tree
    if (!is_sys_process(s)) {
	Error e = __p_check_signature(defs,s,code);
	if (e) return e;
	T *rt = __p_make_run_tree(defs.processes,s,code);
	__p_reduce(defs,rt,_t_child(rt,1));
	x = _t_detach_by_idx(rt,1);
	_t_free(rt);
    }
    else {
	int b,c;
	switch(s.id) {
	case IF_ID:
	    t = _t_child(code,1);
	    b = (*(int *)_t_surface(t)) ? 2 : 3;
	    x = _t_detach_by_idx(code,b);
	    break;
	case ADD_INT_ID:
	    x = _t_detach_by_idx(code,1);
	    c = *(int *)_t_surface(_t_child(code,1));
	    *((int *)&x->contents.surface) = c+*((int *)&x->contents.surface);
	    break;
	case SUB_INT_ID:
	    x = _t_detach_by_idx(code,1);
	    c = *(int *)_t_surface(_t_child(code,1));
	    *((int *)&x->contents.surface) = *((int *)&x->contents.surface)-c;
	    break;
	case MULT_INT_ID:
	    x = _t_detach_by_idx(code,1);
	    c = *(int *)_t_surface(_t_child(code,1));
	    *((int *)&x->contents.surface) = *((int *)&x->contents.surface)*c;
	    break;
	case DIV_INT_ID:
	    x = _t_detach_by_idx(code,1);
	    c = *(int *)_t_surface(_t_child(code,1));
	    *((int *)&x->contents.surface) = *((int *)&x->contents.surface)/c;
	    break;
	case MOD_INT_ID:
	    x = _t_detach_by_idx(code,1);
	    c = *(int *)_t_surface(_t_child(code,1));
	    *((int *)&x->contents.surface) = *((int *)&x->contents.surface)%c;
	    break;
	case EQ_INT_ID:
	    x = _t_detach_by_idx(code,1);
	    c = *(int *)_t_surface(_t_child(code,1));
	    *((int *)&x->contents.surface) = *((int *)&x->contents.surface)==c;
	    x->contents.symbol = TRUE_FALSE;
	    break;
	case LT_INT_ID:
	    x = _t_detach_by_idx(code,1);
	    c = *(int *)_t_surface(_t_child(code,1));
	    *((int *)&x->contents.surface) = *((int *)&x->contents.surface)<c;
	    x->contents.symbol = TRUE_FALSE;
	    break;
	case GT_INT_ID:
	    x = _t_detach_by_idx(code,1);
	    c = *(int *)_t_surface(_t_child(code,1));
	    *((int *)&x->contents.surface) = *((int *)&x->contents.surface)>c;
	    x->contents.symbol = TRUE_FALSE;
	    break;
	case LTE_INT_ID:
	    x = _t_detach_by_idx(code,1);
	    c = *(int *)_t_surface(_t_child(code,1));
	    *((int *)&x->contents.surface) = *((int *)&x->contents.surface)<=c;
	    x->contents.symbol = TRUE_FALSE;
	    break;
	case GTE_INT_ID:
	    x = _t_detach_by_idx(code,1);
	    c = *(int *)_t_surface(_t_child(code,1));
	    *((int *)&x->contents.surface) = *((int *)&x->contents.surface)>=c;
	    x->contents.symbol = TRUE_FALSE;
	    break;
	case RESPOND_ID:
	    // for now we just remove the RESPOND instruction and replace it with it's own child
	    x = _t_detach_by_idx(code,1);
	    break;
	case INTERPOLATE_FROM_MATCH_ID:
	    match_results = _t_child(code,2);
	    match_tree = _t_child(code,3);
	    x = _t_detach_by_idx(code,1);
	    _p_interpolate_from_match(x,match_results,match_tree);
	    break;
	default:
	    raise_error("unimplemented instruction: %s",_d_get_process_name(defs.processes,s));
	}
    }
    _t_replace(parent,idx,x);
    return noReductionErr;
}

Error _p_reduce(Defs defs,T *run_tree) {
    T *code = _t_child(run_tree,1);
    if (!code) {
	raise_error0("expecting code tree as first child of run tree!");
    }
    return __p_reduce(defs,run_tree,code);
}

T *__p_make_run_tree(T *processes,Process p,T *params) {
    T *t = _t_new_root(RUN_TREE);
    T *code_def = _d_get_process_code(processes,p);
    T *code = _t_child(code_def,3);

    T *c = _t_clone(code);
    _t_add(t,c);
    T *ps = _t_newr(t,PARAMS);
    int i,num_params = _t_children(params);
    for(i=1;i<=num_params;i++) {
	_t_add(ps,_t_detach_by_idx(params,1));
    }
    return t;
}

/**
 * Build a run tree from a code tree and params
 *
 * @param[in] processes processes trees
 * @param[in] process Process tree node to be turned into run tree
 * @param[in] num_params the number of parameters to add to the parameters child
 * @param[in] ... T params
 * @returns T RUN_TREE tree
 */
T *_p_make_run_tree(T *processes,T *process,int num_params,...) {
    va_list params;
    int i;

    T *t = _t_new_root(RUN_TREE);

    Process p = *(Process *)_t_surface(process);
    if (!is_process(p)) {
	raise_error("%s is not a Process",_d_get_process_name(processes,p));
    }
    if (is_sys_process(p)) {
	raise_error0("can't handle sys_processes!");
    }

    T *code_def = _d_get_process_code(processes,p);
    T *code = _t_child(code_def,3);

    T *c = _t_clone(code);
    _t_add(t,c);
    T *ps = _t_newr(t,PARAMS);
    va_start(params,num_params);
    for(i=0;i<num_params;i++) {
	_t_add(ps,_t_clone(va_arg(params,T *)));
    }
    va_end(params);
    return t;
}
/** @}*/
