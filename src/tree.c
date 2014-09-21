/**
 * @ingroup tree
 *
 * @{
 *
 * @file tree.c
 * @brief semantic tree implementation
 *
 * @copyright Copyright (C) 2013-2014, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 */

#include "tree.h"
#include "ceptr_error.h"

/*****************  Node creation */
void __t_append_child(Tnode *t,Tnode *c) {
    if (t->structure.child_count == 0) {
	t->structure.children = malloc(sizeof(Tnode *)*TREE_CHILDREN_BLOCK);
    } else if (!(t->structure.child_count % TREE_CHILDREN_BLOCK)){
	int b = t->structure.child_count/TREE_CHILDREN_BLOCK + 1;
	t->structure.children = realloc(t->structure.children,sizeof(Tnode *)*(TREE_CHILDREN_BLOCK*b));
    }

    t->structure.children[t->structure.child_count++] = c;
}

void __t_init(Tnode *t,Tnode *parent,Symbol symbol) {
    t->structure.child_count = 0;
    t->structure.parent = parent;
    t->contents.symbol = symbol;
    t->context.flags = 0;
    if (parent != NULL) {
	__t_append_child(parent,t);
    }
}


/**
 * Create a new tree node
 *
 * @param[in] parent parent node for the node to be created.  Can be 0 if this is a root node
 * @param[in] symbol semantic symbol for the node to be create
 * @param[in] surface pointer to node's data
 * @param[in] size size in bytes of the surface
 * @returns pointer to node allocated on the heap
*/
Tnode * _t_new(Tnode *parent,Symbol symbol,void *surface,size_t size) {
    Tnode *t = malloc(sizeof(Tnode));
    __t_init(t,parent,symbol);
    if (size) {
	t->context.flags |= TFLAG_ALLOCATED;
	t->contents.surface = malloc(size);
	if (surface)
	    memcpy(t->contents.surface,surface,size);
    }
    t->contents.size = size;
    return t;
}

/**
 * Create a new tree node with an integer surface
 *
 * @param[in] parent parent node for the node to be created.  Can be 0 if this is a root node
 * @param[in] symbol semantic symbol for the node to be create
 * @param[in] surface integer value to store in the surface
 * @returns pointer to node allocated on the heap
 */
Tnode * _t_newi(Tnode *parent,Symbol symbol,int surface) {
    Tnode *t = malloc(sizeof(Tnode));
    *((int *)&t->contents.surface) = surface;
    t->contents.size = sizeof(int);
    __t_init(t,parent,symbol);
    return t;
}

/**
 * Create a new tree node with a tree as it's surface
 *
 * @param[in] parent parent node for the node to be created.  Can be 0 if this is a root node
 * @param[in] symbol semantic symbol for the node to be create
 * @param[in] surface pointer to tree to store as an orthogonal tree in the surface
 * @returns pointer to node allocated on the heap
 */
Tnode * _t_newt(Tnode *parent,Symbol symbol,Tnode *surface) {
    Tnode *t = malloc(sizeof(Tnode));
    *((Tnode **)&t->contents.surface) = surface;
    t->contents.size = sizeof(Tnode *);
    __t_init(t,parent,symbol);
    t->context.flags |= TFLAG_SURFACE_IS_TREE;
    return t;
}

/**
 * Create a new tree root node (with null surface and no parent)
 *
 * @param[in] symbol semantic symbol for the node to be create
 * @returns pointer to node allocated on the heap
 */
Tnode *_t_new_root(Symbol symbol) {
    return _t_new(0,symbol,0,0);
}

/**
 * Create a new tree sub-root node (with null surface)
 *
 * @param[in] parent parent node for the node to be created.
 * @param[in] symbol semantic symbol for the node to be create
 * @returns pointer to node allocated on the heap
 */
Tnode *_t_newr(Tnode *parent,Symbol symbol) {
    return _t_new(parent,symbol,0,0);
}

/**
 * Create a new tree node with a receptor as it's surface
 *
 * this is just like _t_newt except that it uses a different flag because
 * when cleaning up we'll need to know that this is a full receptor, not just
 * a plain tree
 *
 * @param[in] parent parent node for the node to be created.
 * @param[in] symbol semantic symbol for the node to be create
 * @param[in] r receptor
 * @returns pointer to node allocated on the heap
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/tree_spec.h testTreeNewReceptor
 */
Tnode *_t_new_receptor(Tnode *parent,Symbol symbol,Receptor *r) {
    Tnode *t = _t_newt(parent,symbol,(Tnode *)r);
    t->context.flags |= TFLAG_SURFACE_IS_TREE+TFLAG_SURFACE_IS_RECEPTOR;
    return t;
}

/**
 * Create a new tree node with a scape as it's surface
 *
 * this is just like _t_newt except that it uses a different flag because
 * when cleaning up we'll need to know that this is a scape, not just
 * a plain tree
 *
 * @param[in] parent parent node for the node to be created.
 * @param[in] symbol semantic symbol for the node to be create
 * @param[in] s scape
 * @returns pointer to node allocated on the heap
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/tree_spec.h testTreeNewScae
 */
Tnode *_t_new_scape(Tnode *parent,Symbol symbol,Scape *s) {
    Tnode *t = _t_newt(parent,symbol,(Tnode *)s);
    t->context.flags |= TFLAG_SURFACE_IS_TREE+TFLAG_SURFACE_IS_SCAPE;
    return t;
}
/**
 * Create a new tree node with a Process surface
 *
 * @param[in] parent parent node for the node to be created.  Can be 0 if this is a root node
 * @param[in] symbol semantic symbol for the node to be create
 * @param[in] surface Process value
 * @returns pointer to node allocated on the heap
 */
Tnode * _t_newp(Tnode *parent,Symbol symbol,Process surface) {
    return _t_newi(parent,symbol,surface);  // for now we can just do this because Process is an int
}

/**
 * add an existing tree onto another appending it as a child
 *
 * @param[in] t tree onto which c will be added
 * @param[in] c tree to add onto t
 */
void _t_add(Tnode *t,Tnode *c) {
    root_check(c);
    c->structure.parent = t;
    __t_append_child(t,c);
}

/**
 * Detatch the specified child from a node and return it
 *
 * @note does not free the memory occupied by the child
 * @param[in] t node to detach from
 * @param[in] i index of the child to detach
 * @returns the detatched child
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/tree_spec.h testTreeDetach
 */
Tnode *_t_detach_by_idx(Tnode *t,int i) {
    Tnode *x = _t_child(t,i);
    _t_detach_by_ptr(t,x);
    return x;
}

/**
 * search for a given node in the child list of a node and detatch it if found
 *
 * @note does not free the memory occupied by c
 * @param[in] t node to search
 * @param[in] c node to search for in child list
 */
void _t_detach_by_ptr(Tnode *t,Tnode *c) {
    int i;
    int l = _t_children(t);

    // search for the child to be removed
    for(i=1;i<=l;i++) {
	if (_t_child(t,i) == c) break;
    }

    // if found remove it by decreasing the child count and shift all the other children down
    if (i <= l) {
	t->structure.child_count--;
	if (t->structure.child_count == 0) {
	    free(t->structure.children);
	}
	for(;i<l;i++) {
	    t->structure.children[i-1] = t->structure.children[i];
	}
    }
    c->structure.parent = 0;
}

/**
 * Convert the surface of a node
 *
 * Frees the original surface value if it was allocated.
 *
 * @note only converts the type and surface, not the children!
 * @param[in] t Node to be morphed
 * @param[in] s Symbol to change the surface to
 * @param[in] surface data to copy into the surface
 * @param[in] size of data
 * @param[in] allocate boolean to know whether to allocate surface or copy the surface as an int
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/tree_spec.h testTreeMorphLowLevel
 */
void __t_morph(Tnode *t,Symbol s,void *surface,size_t size,int allocate) {
    t->contents.size = size;
    if (t->context.flags & TFLAG_ALLOCATED) {
	free(t->contents.surface);
    }

    if (allocate) {
	t->contents.surface = malloc(size);
	memcpy(t->contents.surface,surface,size);
	t->context.flags = TFLAG_ALLOCATED; /// @todo Handle the case where the surface of the node to be morphed is itself a tree
    }
    else {
	*((int *)&t->contents.surface) = *(int *)surface;
	t->context.flags = 0;
    }

    t->contents.symbol = s;
}

/**
 * Convert the surface of one node to that of another
 *
 * Frees the original surface value if it was allocated.
 *
 * @note only converts the type and surface, not the children!
 * @param[in] dst Node to be morphed
 * @param[in] src Node type that dst should be morephed to
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/tree_spec.h testTreeMorph
 */
void _t_morph(Tnode *dst,Tnode *src) {
    __t_morph(dst,_t_symbol(src),_t_surface(src),_t_size(src),src->context.flags & TFLAG_ALLOCATED);
}

/**
 * Replace the specified child with the given node.

 * @note frees the replaced child
 * @param[in] t input node on which to operate
 * @param[in] i index to child be replaced
 * @param[in] r node to replace
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/tree_spec.h testTreeReplace
 */
void _t_replace(Tnode *t,int i,Tnode *r) {
    Tnode *c = _t_child(t,i);
    if (!c) {raise_error("tree doesn't have child %d",i);}
    _t_free(c);
    t->structure.children[i-1] = r;
    r->structure.parent = t;
}

/*****************  Node deletion */

void __t_free_children(Tnode *t) {
    int c = t->structure.child_count;
    if (c > 0) {
	while(--c>=0) {
	    _t_free(t->structure.children[c]);
	}
	free(t->structure.children);
    }
    t->structure.child_count = 0;
}

/**
 * free the memory occupied by a tree
 *
 * free walks the tree freeing all the children and any orthogonal trees.
 *
 * @param[in] t tree to be freed
 * @todo make this remove the child from the parent's child-list?
 */
void _t_free(Tnode *t) {
    __t_free_children(t);
    if (t->context.flags & TFLAG_ALLOCATED)
	free(t->contents.surface);
    else if (t->context.flags & TFLAG_SURFACE_IS_TREE) {
	if (t->context.flags & TFLAG_SURFACE_IS_RECEPTOR)
	    _r_free((Receptor *)t->contents.surface);
	else if (t->context.flags & TFLAG_SURFACE_IS_SCAPE)
	    _s_free((Scape *)t->contents.surface);
	else
	    _t_free((Tnode *)t->contents.surface);
    }
    free(t);
}

Tnode *__t_clone(Tnode *t,Tnode *p) {
    int i,c=_t_children(t);
    Tnode *nt;
    if (t->context.flags & TFLAG_ALLOCATED)
	nt = _t_new(p,_t_symbol(t),_t_surface(t),_t_size(t));
    else
	nt = _t_newi(p,_t_symbol(t),*(int *)_t_surface(t));
    for(i=1;i<=c;i++) {
	__t_clone(_t_child(t,i),nt);
    }
    return nt;
}

/**
 * make a copy of a tree
 *
 * @param[in] t tree to clone
 * @returns Tnode duplicated tree
 * @todo make this work with trees that have orthogonal trees!
 * @bug doesn't properly clone trees with orthogonal trees
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/tree_spec.h testTreeClone
 */
Tnode *_t_clone(Tnode *t) {
    return __t_clone(t,0);
}

/******************** Node data accessors */
/**
 * get the number of children of a given node
 *
 * @param[in] t the node
 * @returns number of children
 */
int _t_children(Tnode *t) {
    return t->structure.child_count;
}

/**
 * get the data of a given node
 *
 * @param[in] t the node
 * @returns pointer to node's surface
 */
void * _t_surface(Tnode *t) {
    if (t->context.flags & (TFLAG_ALLOCATED|TFLAG_SURFACE_IS_TREE))
	return t->contents.surface;
    else
	return &t->contents.surface;
}

/**
 * Get a tree node's semantic symbol
 *
 * @param[in] t the node
 * @returns Symbol for the given node
 */
Symbol _t_symbol(Tnode *t) {
    return t->contents.symbol;
}

/**
 * Get a tree node's surface symbol
 *
 * @param[in] t the node
 * @returns size
 */
size_t _t_size(Tnode *t) {
    return t->contents.size;
}

/*****************  Tree navigation */

Tnode *_t_child(Tnode *t,int i) {
    if (i>t->structure.child_count || i < 1) return 0;
    return t->structure.children[i-1];
}

Tnode * _t_parent(Tnode *t) {
    return t->structure.parent;
}

Tnode * _t_root(Tnode *t) {
    Tnode *p;
    while ((p = _t_parent(t)) != 0) t = p;
    return t;
}

int _t_node_index(Tnode *t) {
	int c;
    int i;
    Tnode *p = _t_parent(t);
    if (p==0) return 0;
    c = _t_children(p);
    for(i=0;i<c;i++) {
	if (p->structure.children[i] == t) {
	    return i+1;
	}
    }
    return 0;
}

/// @todo  this is very expensive if called all the time!!!
Tnode * _t_next_sibling(Tnode *t) {
	int c;
    int i;
    Tnode *p = _t_parent(t);
    if (p==0) return 0;
    c = _t_children(p);
    for(i=0;i<c;i++) {
	if (p->structure.children[i] == t) {
	    i++;
	    return i<c ? p->structure.children[i] : 0;
	}
    }
    return 0;
}

/*****************  Tree path based accesses */
/**
 * compare equality of two paths
 *
 * @param[in] p1 first path
 * @param[in] p2 second path
 * @returns 1 if equal 0 if not
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/tree_spec.h testTreePathEqual
 */
int _t_path_equal(int *p1,int *p2){
    while(*p1 != TREE_PATH_TERMINATOR && *p2 != TREE_PATH_TERMINATOR)
	if (*(p1++) != *(p2++)) return 0;
    return *p1 == TREE_PATH_TERMINATOR && *p2 == TREE_PATH_TERMINATOR;
}

/**
 * return the depth of a given path
 *
 * @param[in] p path
 * @returns int value of depth
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/tree_spec.h testTreePathDepth
 */
int _t_path_depth(int *p) {
    int i=0;
    while(*p++ != TREE_PATH_TERMINATOR) i++;
    return i;
}

/**
 * return the tree path of a given node
 *
 * this function allocates a buffer to hold a path of the given node which
 * it calculates by walkinga back up the tree
 *
 * @todo  implement jumping into orthogonal trees  (i.e. return paths with 0 in them)
 * @param[in] t tree node
 * @returns path of node
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/tree_spec.h testTreePathGetPath
 */
int * _t_get_path(Tnode *t) {
    Tnode *n;
    // allocate an array to hold the
    int s = sizeof(int)*10; // assume most trees are shallower than 10 nodes to prevent realloc
    int *p = malloc(s);
    int j,k,l,i=0,temp;

    // store the children's path index values into the array as we walk back up the tree to the root
    for(n=t;n;) {
	p[i] = _t_node_index(n);
	n =_t_parent(n);
	if (++i >= s) {
	    s*=2;p=realloc(p,s);} // realloc array if tree too deep
    }
    if (i > 2) {
	// reverse the list by swapping elements going from the outside to the center
	i-=2;
	l = i+1;
	k = i/2+1;
	for(j=0;j<k;j++) {
	    temp = p[j];
	    p[j] = p[i];
	    p[i--] = temp;
	}
    }
    else l = i-1;
    p[l]= TREE_PATH_TERMINATOR;
    return p;
}

/**
 * copy a path
 *
 * @param[inout] dst_p pointer to buffer holding destination path
 * @param[in] src_p path
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/tree_spec.h testTreePathCopy
 */
void _t_pathcpy(int *dst_p,int *src_p) {
    while(*src_p != TREE_PATH_TERMINATOR) {
	*dst_p++ = *src_p++;
    }
    *dst_p = TREE_PATH_TERMINATOR;
}

/**
 * get a node by path
 *
 * @param[in] t the tree to search
 * @param[in] p the path to search for
 * @returns pointer to a Tnode
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/tree_spec.h testTreePathGet
 */
Tnode * _t_get(Tnode *t,int *p) {
    int i = *p++;
    Tnode *c;
    if (i == TREE_PATH_TERMINATOR)
	return t;
    else if (i == 0) {
	if (!(t->context.flags & TFLAG_SURFACE_IS_TREE)) {
	    raise_error0("surface is not an tree!");
	}
	c = (Tnode *)(_t_surface(t));
    }
    else
	c = _t_child(t,i);
    if (c == NULL ) return NULL;
    if (*p == TREE_PATH_TERMINATOR) return c;
    return _t_get(c,p);
}

/**
 * get the surface of a node by path
 *
 * @param[in] t the tree to search
 * @param[in] p the path to search for
 * @returns pointer to surface or NULL if path not found
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/tree_spec.h testTreePathGetSurface
 */
void * _t_get_surface(Tnode *t,int *p) {
    Tnode *c = _t_get(t,p);
    if (c == NULL) return NULL;
    return _t_surface(c);
}

/**
 * print a path to a string
 *
 * @param[in] p the path to print
 * @param[inout] buf buffer to copy the string text to
 * @returns pointer to the bufer
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/tree_spec.h testTreePathSprint
 */
char * _t_sprint_path(int *fp,char *buf) {
    char *b = buf;
    int d=_t_path_depth(fp);
    if (d > 0) {
	int i;
	for(i=0;i<d;i++) {
	    sprintf(b,"/%d",fp[i]);
	    b += strlen(b);
	}
    }
    else buf[0] = 0;

    return buf;
}


/*****************  Tree hashing utilities */

/**
 * reduce a tree to a hash value
 *
 * @param[in] symbols declarations of symbols
 * @param[in] structures definitions of structures
 * @param[in] t the tree to hash
 * @returns TreeHash value
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/tree_spec.h testTreeHash
 */
TreeHash _t_hash(Tnode *symbols,Tnode *structures,Tnode *t) {
    int i,c = _t_children(t);
    TreeHash result;
    if (c == 0) {
	uint32_t h[2];
	void *surface = _t_surface(t);
	h[0] = _t_symbol(t);
	h[1] = hashfn((char *)surface,_d_get_symbol_size(symbols,structures,h[0]));
	result = hashfn(h,sizeof(uint32_t)*2);
    }
    else {
	uint32_t l = sizeof(uint32_t)*c+sizeof(Symbol);
	uint32_t *h,*hashes = malloc(l);
	h = hashes;
	for(i=1;i<=c;i++) {
	    *h = _t_hash(symbols,structures,_t_child(t,i));
	    h++;
	}
	(*(Symbol *)h) = _t_symbol(t);
	result = hashfn(hashes,l);
	free(hashes);
    }
    return result;
}

/**
 * comparison function for hash tree equality
 *
 * we have this because TreeHash may become a larger structure
 * not subject to direct equality testing.
 */
int _t_hash_equal(TreeHash h1,TreeHash h2) {
    return h1 == h2;
}

/** @}*/
