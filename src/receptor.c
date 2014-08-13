#include "receptor.h"
#include "semtrex.h"
#include <stdarg.h>
/*****************  create and destroy receptors */

/* creates a new receptor allocating all memory needed */
Receptor *_r_new() {
    Receptor *r = malloc(sizeof(Receptor));
    r->root = _t_new_root(RECEPTOR);
    r->flux = _t_newi(r->root,FLUX,0);
    Tnode *a = _t_newi(r->flux,ASPECT,DEFAULT_ASPECT);
    _t_newi(a,LISTENERS,0);
    _t_newi(a,SIGNALS,0);
    return r;
}

/* adds an expectation/action pair to a receptor's aspect */
void _r_add_listener(Receptor *r,Aspect aspect,Symbol carrier,Tnode *expectation,Tnode *action) {
    Tnode *e = _t_newi(0,LISTENER,carrier);
    _t_add(e,expectation);
    _t_add(e,action);
    Tnode *a = __r_get_listeners(r,aspect);
    _t_add(a,e);
}

/* destroys a receptor freeing all memory it uses */
void _r_free(Receptor *r) {
    _t_free(r->root);
    free(r);
}

/******************  receptor signaling */

Tnode * _r_interpolate_from_match(Tnode *t,Tnode *mr,Tnode *v) {
    int i,c = _t_children(t);
    if (_t_symbol(t) == INTERPOLATE_SYMBOL) {
	Symbol s = *(Symbol *)_t_surface(t);
	Tnode *m = _t_get_match(mr,s);
	int *path = (int *)_t_surface(_t_child(m,1));
	int sibs = *(int*)_t_surface(_t_child(m,2));
	Tnode *x = _t_get(v,path);

	if (!x) {
	    raise_error0("expecting to get a value from match!!");
	}

	//TODO: fix, this should be in tree.c and should deal with children too
	size_t l = t->contents.size = _t_size(x);
        if (x->context.flags & TFLAG_ALLOCATED) {
	    t->contents.surface = malloc(l);
	    memcpy(t->contents.surface,_t_surface(x),l);
	}
	else t->contents.surface = x->contents.surface;

	t->context.flags = x->context.flags;
	t->contents.symbol = s;
    }
    for (i=1;i<=c;i++) {
	_r_interpolate_from_match(_t_child(t,i),mr,v);
    }
    return t;
}

Tnode *_r_reduce(Tnode *t) {
    Tnode *x,*code = _t_child(t,1);
    if (!code) {
	raise_error0("expecting code tree as first child of run tree!");
    }
    Symbol s = _t_symbol(code);
    Tnode *p,*m,*v;
    switch(s) {
    case RESPOND:
	// for now we just remove the RESPOND instruction and replace it with it's own child
	x = _t_detach(code,1);
	_t_replace(t,1,x);
	return _r_reduce(t);
	break;
    case INTERPOLATE_FROM_MATCH:
	p = _t_child(t,2);
	m = _t_child(p,1);
	v = _t_child(p,2);
	x = _t_detach(code,1);
	_t_replace(t,1,_r_interpolate_from_match(x,m,v));
	break;
    default:
	raise_error("unknown instruction: %s",_s_get_symbol_name(s));
    }
    return _t_child(t,1);
}

Tnode *_r_make_run_tree(Tnode *code,int num_params,...) {
    va_list params;
    int i;

    Tnode *t = _t_new_root(RUN_TREE);
    Tnode *c = _t_clone(_t_child(code,1));
    _t_add(t,c);
    Tnode *p = _t_newi(t,PARAMS,0);
    va_start(params,num_params);
    for(i=0;i<num_params;i++) {
	_t_add(p,_t_clone(va_arg(params,Tnode *)));
    }
    va_end(params);
    return t;
}

/* send a signal to a receptor on a given aspect */
Tnode * _r_send(Receptor *r,Receptor *from,Aspect aspect, Tnode *signal_contents) {
    Tnode *m,*e,*l,*result = 0;

    Tnode *as = __r_get_signals(r,aspect);
    Tnode *s = _t_newt(as,SIGNAL,signal_contents);

    // walk through all the listeners on the aspect and see if any expectations match this incoming signal
    Tnode *ls = __r_get_listeners(r,aspect);
    int i,c = _t_children(ls);
    for(i=1;i<=c;i++) {
	l = _t_child(ls,i);
	e = _t_child(l,1);
	// if we get a match, create a run tree from the action, using the match and signal as the parameters
	if (_t_matchr(_t_child(e,1),signal_contents,&m)) {
	    result = _r_make_run_tree(_t_child(l,2),2,m,signal_contents);
	    _t_add(s,result);
	    // for now just reduce the tree in place
	    // TODO: move this to adding the runtree to the thread pool
	    result = _r_reduce(result);
	}
    }
    //TODO: results should actually be a what? success/failure of send

    return result;
}


/******************  internal utilities */

Tnode *__r_get_aspect(Receptor *r,Aspect aspect) {
    return _t_child(r->flux,aspect);
}
Tnode *__r_get_listeners(Receptor *r,Aspect aspect) {
    return _t_child(__r_get_aspect(r,aspect),1);
}
Tnode *__r_get_signals(Receptor *r,Aspect aspect) {
    return _t_child(__r_get_aspect(r,aspect),2);
}
