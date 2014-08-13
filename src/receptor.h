#ifndef _CEPTR_RECEPTOR_H
#define _CEPTR_RECEPTOR_H

#include "tree.h"
#include "label.h"

struct Receptor {
    Tnode *root;
    Tnode *flux;
};
typedef struct Receptor Receptor;

// for now aspects are just identified as the child index in the flux receptor
enum {DEFAULT_ASPECT=1};
typedef int Aspect;

/******************  create and destroy receptors */
Receptor * _r_new();
void _r_add_listener(Receptor *r,Aspect aspect,Symbol carrier,Tnode *semtrex,Tnode *action);
void _r_free(Receptor *r);

/******************  receptor signaling */
Tnode *_r_reduce(Tnode *run_tree);
Tnode *_r_make_run_tree(Tnode *code,int num_params,...);
//*TODO: for now the signal is appended directly to the flux.  Later it should probably be copied
Tnode * _r_send(Receptor *r,Receptor *from,Aspect aspect, Tnode *signal);

/******************  internal utilities */
Tnode *__r_get_aspect(Receptor *r,Aspect aspect);
Tnode *__r_get_listeners(Receptor *r,Aspect aspect);
Tnode *__r_get_signals(Receptor *r,Aspect aspect);

#endif
