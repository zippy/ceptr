#include "process.h"
#include "semtrex.h"
#include <stdarg.h>


/// @todo what to do if match has sibs??
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

void _p_reduce(Tnode *run_tree) {
    Tnode *code = _t_child(run_tree,1);
    if (!code) {
	raise_error0("expecting code tree as first child of run tree!");
    }
    Symbol s = _t_symbol(code);
    Tnode *params,*match_results,*match_tree;
    Tnode *x;
    switch(s) {
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
	raise_error("unknown instruction: %s",_s_get_symbol_name(0,s));
    }
}

Tnode *_p_make_run_tree(Tnode *code,int num_params,...) {
    va_list params;
    int i;

    Tnode *t = _t_new_root(RUN_TREE);
    Tnode *c = _t_clone(_t_child(code,1));
    _t_add(t,c);
    Tnode *p = _t_newr(t,PARAMS);
    va_start(params,num_params);
    for(i=0;i<num_params;i++) {
	_t_add(p,_t_clone(va_arg(params,Tnode *)));
    }
    va_end(params);
    return t;
}
