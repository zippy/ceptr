#ifndef _CEPTR_TREE_H
#define _CEPTR_TREE_H

#include "ceptr_error.h"

#define TREE_CHILDREN_BLOCK 5

typedef int Symbol;
#define TREE_PATH_TERMINATOR -9999

#define TFLAG_ALLOCATED 0x0001

struct T {
    struct T *parent;
    size_t size;
    int flags;
    int child_count;
    struct T **children;
    Symbol noun;
    void *surface;
};
typedef struct T T;


typedef int (*treeMapFn)(T *,T *);

int __t_append_child(T *t,T *c) {
    if (t->child_count == 0) {
	t->children = malloc(sizeof(T *)*TREE_CHILDREN_BLOCK);
    } else if (!(t->child_count % TREE_CHILDREN_BLOCK)){
	int b = t->child_count/TREE_CHILDREN_BLOCK + 1;
	t->children = realloc(t->children,sizeof(T *)*(TREE_CHILDREN_BLOCK*b));
    }

    t->children[t->child_count++] = c;
}

void __t_init(T *t,T *parent,Symbol noun) {
    t->child_count = 0;
    t->parent = parent;
    t->noun = noun;
    t->flags = 0;
    if (parent != NULL) {
	__t_append_child(parent,t);
    }
}

T * _t_new(T *parent,Symbol noun,void *surface,size_t size) {
    T *t = malloc(sizeof(T));
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

T * _t_newi(T *parent,Symbol noun,int surface) {
    T *t = malloc(sizeof(T));
    *((int *)&t->surface) = surface;
    t->size = sizeof(int);
    __t_init(t,parent,noun);
    return t;
}

T * _t_newp(T *parent,Symbol noun,void *surface) {
    T *t = malloc(sizeof(T));
    t->surface = surface;
    t->size = sizeof(void *);
    __t_init(t,parent,noun);
    return t;
}

int _t_children(T *t) {
    return t->child_count;
}

void * _t_surface(T *t) {
    if (t->flags & TFLAG_ALLOCATED)
	return t->surface;
    else
	return &t->surface;
}

T * _t_parent(T *t) {
    return t->parent;
}

Symbol _t_noun(T *t) {
    return t->noun;
}

size_t _t_size(T *t) {
    return t->size;
}

T *_t_child(T *t,int i) {
    if (i>t->child_count || i < 1) return 0;
    return t->children[i-1];
}

void _t_free(T *t);

void __t_free_children(T *t) {
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
void _t_free(T *t) {
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

T * _t_get(T *t,int *p) {
    int i = *p++;
    T *c;
    if (i == TREE_PATH_TERMINATOR)
	return t;
    else if (i == 0)
	//TODO: semantic check to make sure surface is a tree?
	c = *(T **)(_t_surface(t));
    else
	c = _t_child(t,i);
    if (c == NULL ) return NULL;
    if (*p == TREE_PATH_TERMINATOR) return c;
    return _t_get(c,p);
}

void * _t_get_surface(T *t,int *p) {
    T *c = _t_get(t,p);
    if (c == NULL) return NULL;
    return _t_surface(c);
}

T *_t_get_child(T *t,int c) {
    int p[2] = {c,TREE_PATH_TERMINATOR};
    return _t_get(t,p);
}

void *_t_get_child_surface(T *t,int i) {
    T *c = _t_get_child(t,i);
    if (c) {
	return _t_surface(c);
    }
    return NULL;
}

T *_t_new_root(Symbol noun) {
    return _t_new(0,noun,0,0);
}

typedef void (*tIterSurfaceFn)(void *, int, void *param);
typedef void (*tIterFn)(T *, int, void *param);

void _t_iter_children_surface(T *t, tIterSurfaceFn fn, void * param) {
    int count = _t_children(t);
    for (int i = 1; i <= count; i++) {
	T *c = _t_get_child(t,i);
	(fn)(_t_surface(c),i,param);
    }
}

void _t_iter_children(T *t, tIterFn fn, void * param) {
    int count = _t_children(t);
    for (int i = 1; i <= count; i++) {
	T *c = _t_get_child(t,i);
	(fn)(c,i,param);
    }
}


#define __lspc(l) for(int i=0;i<l;i++) printf("   ");

void __t_dump(T *t,int level) {
    __lspc(level);
    printf("noun: %d; sfc: %p; children: %d\n",_t_noun(t),t->surface,_t_children(t));
    for(int i=1;i<=_t_children(t);i++) __t_dump(_t_get_child(t,i),level+1);
}

void _t_dump(T *t) { printf("Tree Dump:\n");__t_dump(t,0);}

void _t_become(T *t, T *src_t) {
    // free my own children but not if my child is what I'm about to become
    // NOTE: this means that you have to detach a child of a child or it will be double freed!
    for(int i=1; i<=_t_children(t);i++) {
	T *c = _t_get_child(t,i);
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

T *_t_next_df(T *t,int *p) {
    T *i = _t_get(t,p);
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

void _t_init_walk(T *t,TreeWalker *w, int type) {
    w->type = type;
    w->p[0] = TREE_PATH_TERMINATOR;
}

T *__t_next_bf(T *t,int *p) {
    raise_error0("NOT IMPLEMENTED!\n");
}

T *_t_walk(T *t,TreeWalker *w) {
    if (w->type == WALK_DEPTH_FIRST)
	return _t_next_df(t,w->p);
    else
	return __t_next_bf(t,w->p);
}

enum {PTR_NOUN = -999999};

T *_t_build_one(T *parent,T *t,T *m) {
    T *n,*src;
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

T *_t_build(T *t,T *m) {
    int p[20];
    T *i,*n = _t_build_one(0,t,m);
    TreeWalker w;
    _t_init_walk(t,&w,WALK_DEPTH_FIRST);
    while(i = _t_walk(t,&w)) {
	_t_path_parent(p,w.p);
	T *pp = _t_get(n,p);
	T *c = _t_build_one(pp,i,m);
    }
    return n;
}

T *_t_buildx(T *parent,T *t,T *m) {
    T *n,*src;
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

int _t_nouns_eq(T *t1,T *t2) {
    int c;
    if ((_t_noun(t1) != _t_noun(t2)) || ((c = _t_children(t1)) != _t_children(t2))) return 0;
    for (int i=1;i<=c;i++) {
	if (!_t_nouns_eq(_t_get_child(t1,i),_t_get_child(t2,i))) return 0;
    }
    return 1;
}

int _t_map(T *t1,T *t2,treeMapFn mf) {
    int c;
    if (((c = _t_children(t1)) != _t_children(t2)) || !(mf)(t1,t2)) return 0;
    for (int i=1;i<=c;i++) {
	if (!_t_map(_t_get_child(t1,i),_t_get_child(t2,i),mf)) return 0;
    }
    return 1;
}



#endif
