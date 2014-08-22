/**
 * @file tree.h
 * @brief semantic trees header file
 */

#ifndef _CEPTR_TREE_H
#define _CEPTR_TREE_H

#include <string.h>
#include <stdlib.h>
#include "ceptr_error.h"
#include "symbol.h"
#include "structure.h"

#define TREE_CHILDREN_BLOCK 5
#define TREE_PATH_TERMINATOR -9999

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
#define TFLAG_SURFACE_IS_TREE 0x0002

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
Tnode * _t_newi(Tnode *parent,Symbol symbol,int surface);
Tnode * _t_newt(Tnode *parent,Symbol symbol,Tnode *t);
Tnode *_t_new_root(Symbol symbol);
Tnode *_t_newr(Tnode *parent,Symbol symbol);
void _t_add(Tnode *t,Tnode *c);
void _t_detach_by_ptr(Tnode *t,Tnode *c);
Tnode *_t_detach_by_idx(Tnode *t,int i);
void _t_replace(Tnode *t,int i,Tnode *r);
void _t_morph(Tnode *dst,Tnode *src);
void __t_morph(Tnode *t,Symbol s,void *surface,size_t length,int allocate);
void _t_free(Tnode *t);
Tnode *_t_clone(Tnode *t);

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
int _t_node_index(Tnode *t);

/*****************  Tree path based accesses */
int _t_path_equal(int *p1,int *p2);
int _t_path_depth(int *p);
void _t_pathcpy(int *dst_p,int *src_p);
void _t_path_parent(int *n,int *p);
Tnode * _t_get(Tnode *t,int *p);
int *_t_get_path(Tnode *t);
void * _t_get_surface(Tnode *t,int *p);
char * _t_sprint_path(int *fp,char *buf);

/*
_t_new();
_t_free()
_t_get_child(t,nth_child)
_t_get_surface()
_t_get_parent()
_t_get_root()
_t_get_next_sibling()

_t_parse(); //semtrex style parsing
_t_match()
*/
#endif
