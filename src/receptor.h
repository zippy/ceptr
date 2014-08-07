#ifndef _CEPTR_RECEPTOR_H
#define _CEPTR_RECEPTOR_H

#include "tree.h"
#include "label.h"

struct Receptor {
    Tnode *root;
    Tnode *flux;
    Tnode *listeners;
};
typedef struct Receptor Receptor;

// for now aspects are just identified as the child index in the flux receptor
enum {DEFAULT_ASPECT=1};
typedef int Aspect;

/******************  create and destroy receptors */
Receptor * _r_new();
void _r_add_expect(Receptor *r,Symbol carrier,Tnode *semtrex,Tnode *action);
void _r_free(Receptor *r);

/******************  receptor signaling */
//*TODO: for now the signal is appended directly to the flux.  Later it should probably be copied
void _r_send(Receptor *r,Receptor *from,Aspect aspect, Tnode *signal);

#endif
