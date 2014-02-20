#ifndef _TREE_H
#define _TREE_H

#include "ceptr_error.h"

#define TREE_CHILDREN_BLOCK 5

typedef int Symbol;
#define TREE_PATH_TERMINATOR 10

#define TREE_ROOT_NOUN -999

typedef struct {
    int placeholder;
} Troot;

struct Tnode {
    struct Tnode *parent;
    int child_count;
    struct Tnode **children;
    Symbol noun;
    char surface;
};
typedef struct Tnode Tnode;

int __t_append_child(Tnode *t,Tnode *c) {
    if (t->child_count == 0) {
	t->children = malloc(sizeof(Tnode *)*TREE_CHILDREN_BLOCK);
    } else if (!(t->child_count % TREE_CHILDREN_BLOCK)){
	int b = t->child_count/TREE_CHILDREN_BLOCK + 1;
	t->children = realloc(t->children,sizeof(Tnode *)*(TREE_CHILDREN_BLOCK*b));
    }

    t->children[t->child_count++] = c;
}

Tnode * _t_new(Tnode *parent,Symbol noun,void *surface,size_t size) {
    Tnode *t = malloc(sizeof(Tnode)+size);
    t->child_count = 0;
    memcpy(&t->surface,surface,size);
    t->parent = parent;
    t->noun = noun;
    if (parent != NULL) {
	__t_append_child(parent,t);
    }
    return t;
}

int _t_children(Tnode *t) {
    return t->child_count;
}

void * _t_surface(Tnode *t) {
    return &t->surface;
}

Tnode * _t_parent(Tnode *t) {
    return t->parent;
}

Symbol _t_noun(Tnode *t) {
    return t->noun;
}

Tnode *_t_child(Tnode *t,int i) {
    if (i>t->child_count || i < 1) return 0;
    return t->children[i-1];
}

//TODO: make this remove the child from the parent's child-list?
void _t_free(Tnode *t) {
    int c = t->child_count;
    if (c > 0) {
	while(--c>=0) {
	    _t_free(t->children[c]);
	}
	free(t->children);
    }
    free(t);
}

Tnode * _t_get(Tnode *t,int *p) {
    int i = *p++;
    Tnode *c;
    if (i == 0)
	//TODO: semantic check to make sure surface is a tree?
	c = *(Tnode **)(_t_surface(t));
    else
	c = _t_child(t,i);
    if (c == NULL ) return NULL;
    if (*p == TREE_PATH_TERMINATOR) return c;
    return _t_get(c,p);
}

void * _t_get_surface(Tnode *t,int *p) {
    Tnode *c = _t_get(t,p);
    if (c == NULL) return NULL;
    return _t_surface(c);
}

Tnode *_t_get_child(Tnode *t,int c) {
    int p[2] = {c,TREE_PATH_TERMINATOR};
    return _t_get(t,p);
}

void *_t_get_child_surface(Tnode *t,int i) {
    Tnode *c = _t_get_child(t,i);
    if (c) {
	return _t_surface(c);
    }
    return NULL;
}

Tnode *_t_new_root() {
    Troot t;
    return _t_new(0,TREE_ROOT_NOUN,&t,sizeof(Troot));
}

typedef void (*tIterSurfaceFn)(void *, int, void *param);
typedef void (*tIterFn)(Tnode *, int, void *param);

void _t_iter_children_surface(Tnode *t, tIterSurfaceFn fn, void * param) {
    int count = _t_children(t);
    for (int i = 1; i <= count; i++) {
	Tnode *c = _t_get_child(t,i);
	(fn)(_t_surface(c),i,param);
    }
}

void _t_iter_children(Tnode *t, tIterFn fn, void * param) {
    int count = _t_children(t);
    for (int i = 1; i <= count; i++) {
	Tnode *c = _t_get_child(t,i);
	(fn)(c,i,param);
    }
}

#endif
