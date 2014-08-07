#include "receptor.h"

/*****************  create and destroy receptors */

/* creates a new receptor allocating all memory needed */
Receptor *_r_new() {
    Receptor *r = malloc(sizeof(Receptor));
    r->root = _t_new_root(RECEPTOR);
    r->flux = _t_newi(r->root,FLUX,0);
    _t_newi(r->flux,ASPECT,DEFAULT_ASPECT);
    r->listeners = _t_newi(r->root,LISTENERS,0);
    return r;
}

/* adds an expectation/action pair to a receptor */
void _r_add_expect(Receptor *r,Symbol carrier,Tnode *expectation,Tnode *action) {
    Tnode *e = _t_newi(0,LISTENER,carrier);
    _t_add(e,expectation);
    _t_add(e,action);
    _t_add(r->listeners,e);
}

/* destroys a receptor freeing all memory it uses */
void _r_free(Receptor *r) {
    _t_free(r->flux);
    free(r);
}


/******************  receptor signaling */

Tnode *__r_get_aspect(Receptor *r,Aspect aspect) {
    return _t_child(r->flux,aspect);
}

/* send a signal to a receptor on a given aspect */
void _r_send(Receptor *r,Receptor *from,Aspect aspect, Tnode *signal) {
    Tnode *a = __r_get_aspect(r,aspect);
    _t_add(a,signal);
}

/******************  receptor actions */
