/**
 * @ingroup receptor
 *
 * @{
 * @file process.c
 * @brief implementation of ceptr processing: instructions and run tree reduction
 *
 * @copyright Copyright (C) 2013-2014, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 */
#include "process.h"
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
void _p_interpolate_from_match(Tnode *t,Tnode *match_results,Tnode *match_tree) {
    int i,c = _t_children(t);
    if (_t_symbol(t) == INTERPOLATE_SYMBOL) {
	Symbol s = *(Symbol *)_t_surface(t);
	Tnode *m = _t_get_match(match_results,s);
	int *path = (int *)_t_surface(_t_child(m,1));
	int sibs = *(int*)_t_surface(_t_child(m,2));
	Tnode *x = _t_get(match_tree,path);

	if (!x) {
	    raise_error0("expecting to get a value from match!!");
	}
	_t_morph(t,x);
   }
    for (i=1;i<=c;i++) {
	_p_interpolate_from_match(_t_child(t,i),match_results,match_tree);
    }
}

/**
 * reduce a run tree by executing the instructions in it and replacing the tree values in place
 *
 * a run_tree is expected to have a code tree as the first child, and parameters as the second
 *
 * @param[in] run_tree the run tree to be reduced
 */
void _p_reduce(Tnode *run_tree) {
    Tnode *code = _t_child(run_tree,1);
    if (!code) {
	raise_error0("expecting code tree as first child of run tree!");
    }
    Process s = _t_symbol(code);

    // if this isn't a process then we've reduced the tree
    if (!is_process(s)) return;
    Tnode *params,*match_results,*match_tree,*t;
    int b;
    Tnode *x;
    switch(s) {
    case IF:
	params = _t_child(run_tree,2);
	t = _t_child(params,1);
	//@todo check to see if we need to reduce the condition
	if (0) {
	    // _p_reduce(t) the condition expression
	}
	b = (*(int *)_t_surface(t)) ? 2 : 3;
	x = _t_detach_by_idx(params,b);
	_t_replace(run_tree,1,x);
	break;
    case RESPOND:
	// for now we just remove the RESPOND instruction and replace it with it's own child
	x = _t_detach_by_idx(code,1);
	_t_replace(run_tree,1,x);
	_p_reduce(run_tree);
	break;
    case INTERPOLATE_FROM_MATCH:
	params = _t_child(run_tree,2);
	match_results = _t_child(params,1);
	match_tree = _t_child(params,2);
	x = _t_detach_by_idx(code,1);
	_p_interpolate_from_match(x,match_results,match_tree);
	_t_replace(run_tree,1,x);
	break;
    default:
	raise_error("unknown instruction: %s",_r_get_symbol_name(0,s));
    }
}

/**
 * Build a run tree from a code tree and params
 *
 * @param[in] processes processes trees
 * @param[in] process Process tree node to be turned into run tree
 * @param[in] num_params the number of parameters to add to the parameters child
 * @param[in] ... Tnode params
 * @returns Tnode RUN_TREE tree
 */
Tnode *_p_make_run_tree(Tnode *processes,Tnode *process,int num_params,...) {
    va_list params;
    int i;

    Tnode *t = _t_new_root(RUN_TREE);

    Process p = *(Process *)_t_surface(process);
    if (!is_process(p)) {
	raise_error("%s is not a Process",_d_get_process_name(processes,p));
    }
    if (is_sys_process(p)) {
	raise_error0("can't handle sys_processes!");
    }

    Tnode *code_def = _t_child(processes,-p);
    Tnode *code = _t_child(code_def,3);

    Tnode *c = _t_clone(code);
    _t_add(t,c);
    Tnode *ps = _t_newr(t,PARAMS);
    va_start(params,num_params);
    for(i=0;i<num_params;i++) {
	_t_add(ps,_t_clone(va_arg(params,Tnode *)));
    }
    va_end(params);
    return t;
}
/** @}*/
