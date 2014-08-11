#include "receptor.h"

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
void _r_add_expect(Receptor *r,Aspect aspect,Symbol carrier,Tnode *expectation,Tnode *action) {
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

/* send a signal to a receptor on a given aspect */
void _r_send(Receptor *r,Receptor *from,Aspect aspect, Tnode *signal_contents) {
    Tnode *s = _t_new_root(SIGNAL);
    _t_add(s,signal_contents);
    Tnode *as = __r_get_signals(r,aspect);
    _t_add(as,s);
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
