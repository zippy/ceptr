#ifndef _CEPTR_TREE_H
#define _CEPTR_TREE_H

#include "ceptr_error.h"
#include <ctype.h>

#define TREE_CHILDREN_BLOCK 5

typedef int Symbol;
#define TREE_PATH_TERMINATOR -9999

#define TFLAG_ALLOCATED 0x0001

struct Tnode {
    struct Tnode *parent;
    size_t size;
    int flags;
    int child_count;
    struct Tnode **children;
    Symbol noun;
    void *surface;
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

void __t_init(Tnode *t,Tnode *parent,Symbol noun) {
    t->child_count = 0;
    t->parent = parent;
    t->noun = noun;
    t->flags = 0;
    if (parent != NULL) {
	__t_append_child(parent,t);
    }
}

Tnode * _t_new(Tnode *parent,Symbol noun,void *surface,size_t size) {
    Tnode *t = malloc(sizeof(Tnode));
    __t_init(t,parent,noun);
    if (size) {
	t->flags |= TFLAG_ALLOCATED;
	t->surface = malloc(size);
	if (surface)
	    memcpy(t->surface,surface,size);
    }
    t->size = size;
    return t;
}

Tnode * _t_newi(Tnode *parent,Symbol noun,int surface) {
    Tnode *t = malloc(sizeof(Tnode));
    *((int *)&t->surface) = surface;
    t->size = sizeof(int);
    __t_init(t,parent,noun);
    return t;
}

Tnode * _t_newp(Tnode *parent,Symbol noun,void *surface) {
    Tnode *t = malloc(sizeof(Tnode));
    t->surface = surface;
    t->size = sizeof(void *);
    __t_init(t,parent,noun);
    return t;
}

int _t_children(Tnode *t) {
    return t->child_count;
}

void * _t_surface(Tnode *t) {
    if (t->flags & TFLAG_ALLOCATED)
	return t->surface;
    else
	return &t->surface;
}

Tnode * _t_parent(Tnode *t) {
    return t->parent;
}

Symbol _t_noun(Tnode *t) {
    return t->noun;
}

size_t _t_size(Tnode *t) {
    return t->size;
}

Tnode *_t_child(Tnode *t,int i) {
    if (i>t->child_count || i < 1) return 0;
    return t->children[i-1];
}

void _t_free(Tnode *t);

void __t_free_children(Tnode *t) {
    int c = t->child_count;
    if (c > 0) {
	while(--c>=0) {
	    _t_free(t->children[c]);
	}
	free(t->children);
    }
    t->child_count = 0;
}

//TODO: make this remove the child from the parent's child-list?
void _t_free(Tnode *t) {
    __t_free_children(t);
    if (t->flags & TFLAG_ALLOCATED)
	free(t->surface);
    free(t);
}

void _t_path_parent(int *n,int *p) {
    if (*p == TREE_PATH_TERMINATOR) {
	raise_error0("unable to take parent of root\n");
    }
    while(*p != TREE_PATH_TERMINATOR) {
	*n++ = *p++;
    }
    *--n = TREE_PATH_TERMINATOR;
}

Tnode * _t_get(Tnode *t,int *p) {
    int i = *p++;
    Tnode *c;
    if (i == TREE_PATH_TERMINATOR)
	return t;
    else if (i == 0)
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

Tnode *_t_new_root(Symbol noun) {
    return _t_new(0,noun,0,0);
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


#define __lspc(l) for(int i=0;i<l;i++) printf("   ");

void __t_dump(Tnode *t,int level) {
    __lspc(level);
    printf("noun: %d; sfc: %p; children: %d\n",_t_noun(t),t->surface,_t_children(t));
    for(int i=1;i<=_t_children(t);i++) __t_dump(_t_get_child(t,i),level+1);
}

void _t_dump(Tnode *t) { printf("Tree Dump:\n");__t_dump(t,0);}

void _t_become(Tnode *t, Tnode *src_t) {
    // free my own children but not if my child is what I'm about to become
    // NOTE: this means that you have to detach a child of a child or it will be double freed!
    for(int i=1; i<=_t_children(t);i++) {
	Tnode *c = _t_get_child(t,i);
	if (c != src_t) _t_free(c);
    }

    // copy over src children
    t->child_count = src_t->child_count;
    t->noun = src_t->noun;
    for(int i=0; i< t->child_count;i++) {
	t->children[i] = src_t->children[i];
    }

    // free my surface if it was alloced
    if (t->flags & TFLAG_ALLOCATED)
	free(t->surface);
    t->surface = src_t->surface;
    t->flags = src_t->flags;

    // free the source because it's now me, but not it's children (i.e. not _t_free)
    free(src_t);
}

int _t_path_depth(int *p) {
    int i=0;
    while(*p++ != TREE_PATH_TERMINATOR) i++;
    return i;
}

Tnode *_t_next_df(Tnode *t,int *p) {
    Tnode *i = _t_get(t,p);
    int d = _t_path_depth(p);

    //   raise(SIGINT);
    if (!i) return 0;
    // if the current node has a children then the next node
    // is the first of those children.
    if (_t_children(i) > 0) {
	p[d]=1;
	p[d+1]=TREE_PATH_TERMINATOR;
	return _t_get(t,p);
    }

    // if the current node has no children, then the next node is the next
    // sibling of the current node or of a parent node that has a next sibling
    else {
	while(d > 0) {
	    p[d-1]++;
	    i = _t_get(t,p);
	    if (i) return i;
	    d--;
	    p[d]=TREE_PATH_TERMINATOR;
	}
    }
    return NULL;
}

enum {WALK_DEPTH_FIRST,WALK_BREADTH_FIRST};

typedef struct {
    int type;
    int p[20];
} TreeWalker;

void _t_init_walk(Tnode *t,TreeWalker *w, int type) {
    w->type = type;
    w->p[0] = TREE_PATH_TERMINATOR;
}

Tnode *__t_next_bf(Tnode *t,int *p) {
    raise_error0("NOT IMPLEMENTED!\n");
}

Tnode *_t_walk(Tnode *t,TreeWalker *w) {
    if (w->type == WALK_DEPTH_FIRST)
	return _t_next_df(t,w->p);
    else
	return __t_next_bf(t,w->p);
}

enum {PTR_NOUN = -999999};

Tnode *_t_build_one(Tnode *parent,Tnode *t,Tnode *m) {
    Tnode *n,*src;
    if (_t_noun(t) == PTR_NOUN) {
	src = _t_get_child(m,*(int *)_t_surface(t));
    }
    else {
	src = t;
    }
    if (src->flags & TFLAG_ALLOCATED) {
	n = _t_new(parent,_t_noun(src),_t_surface(src),_t_size(src));
    } else {
	n = _t_newi(parent,_t_noun(src),*(int *)_t_surface(src));
    }
    return n;
}

Tnode *_t_build(Tnode *t,Tnode *m) {
    int p[20];
    Tnode *i,*n = _t_build_one(0,t,m);
    TreeWalker w;
    _t_init_walk(t,&w,WALK_DEPTH_FIRST);
    while(i = _t_walk(t,&w)) {
	_t_path_parent(p,w.p);
	Tnode *pp = _t_get(n,p);
	Tnode *c = _t_build_one(pp,i,m);
    }
    return n;
}

Tnode *_t_buildx(Tnode *parent,Tnode *t,Tnode *m) {
    Tnode *n,*src;
    Symbol noun;
    if (_t_noun(t) == PTR_NOUN) {
	src = _t_get_child(m,*(int *)_t_surface(t));
    }
    else {
	src = t;
    }
    if (src->flags & TFLAG_ALLOCATED) {
	n = _t_new(parent,_t_noun(src),_t_surface(src),_t_size(src));
    } else {
	n = _t_newi(parent,_t_noun(src),*(int *)_t_surface(src));
    }
    for(int i=0; i< t->child_count;i++) {
	_t_buildx(n,src->children[i],m);
    }
    return n;
}


#endif
