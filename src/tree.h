#ifndef _CEPTR_TREE_H
#define _CEPTR_TREE_H

#include "ceptr_error.h"
#define TREE_CHILDREN_BLOCK 5

typedef int Symbol;

struct Tstruct {
    struct Tnode *parent;
    int child_count;
    struct Tnode **children;
};
typedef struct Tstruct Tstruct;

struct Tcontents {
    Symbol symbol;
    size_t size;
    void *surface;
};
typedef struct Tcontents Tcontents;

//Flags:
#define TFLAG_ALLOCATED 0x0001

struct Tcontext {
    int flags;
};
typedef struct Tcontext Tcontext;

struct Tnode {
    Tstruct structure;
    Tcontext context;
    Tcontents contents;
};
typedef struct Tnode Tnode;


/*****************  Node creation and deletion*/
Tnode * _t_new(Tnode *t,Symbol symbol, void *surface, size_t size);
void _t_free(Tnode *t);

/******************** Node data accessors */
int _t_children(Tnode *t);
void * _t_surface(Tnode *t);
Symbol _t_symbol(Tnode *t);
size_t _t_size(Tnode *t);

/*****************  Tree navigation */
Tnode * _t_parent(Tnode *t);
Tnode *_t_child(Tnode *t,int i);
Tnode * _t_root(Tnode *t);
Tnode * _t_next_sibling(Tnode *t);


/*
_t_new();
_t_free();
_t_parse(); //semtrex style parsing
_t_get_child(t,nth_child)
_t_get_surface()
_t_get_parent()
_t_get_root()
_t_get_next_sibling()

_t_match()
*/
#endif
