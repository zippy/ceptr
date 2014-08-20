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

Tnode * _t_newi(Tnode *parent,Symbol symbol,int surface) {
    Tnode *t = malloc(sizeof(Tnode));
    *((int *)&t->contents.surface) = surface;
    t->contents.size = sizeof(int);
    __t_init(t,parent,symbol);
    return t;
}

Tnode * _t_newt(Tnode *parent,Symbol symbol,Tnode *surface) {
    Tnode *t = malloc(sizeof(Tnode));
    *((Tnode **)&t->contents.surface) = surface;
    t->contents.size = sizeof(Tnode *);
    __t_init(t,parent,symbol);
    t->context.flags |= TFLAG_SURFACE_IS_TREE;
    return t;
}

Tnode *_t_new_root(Symbol symbol) {
    return _t_new(0,symbol,0,0);
}

void _t_add(Tnode *t,Tnode *c) {
    if (c->structure.parent != 0) {
	raise_error0("can't add a node that isn't a root!");
    }
    c->structure.parent = t;
    __t_append_child(t,c);
}


// detach and return a child
Tnode *_t_detach(Tnode *t,int i) {
    Tnode *x = _t_child(t,i);
    _t_remove(t,x);
    return x;
}

// search for a child (c) of a node and remove it if found
// NOTE: does not free the memory occupied by c
void _t_remove(Tnode *t,Tnode *c) {
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

void __t_morph(Tnode *t,Symbol s,void *surface,size_t size,int allocate) {
    t->contents.size = size;
    if (t->context.flags & TFLAG_ALLOCATED) {
	free(t->contents.surface);
    }

    if (allocate) {
	t->contents.surface = malloc(size);
	memcpy(t->contents.surface,surface,size);
	t->context.flags = TFLAG_ALLOCATED; //TODO: what if the surface is a tree?
    }
    else {
	*((int *)&t->contents.surface) = *(int *)surface;
	t->context.flags = 0;
    }

    t->contents.symbol = s;
}

// convert the surface of one node to that of the other
// this will free the original surface value if it was allocated.
// it does nothing about the children or parent, just changes the surface and symbol
void _t_morph(Tnode *dst,Tnode *src) {
    __t_morph(dst,_t_symbol(src),_t_surface(src),_t_size(src),src->context.flags & TFLAG_ALLOCATED);
}

// replaces the specified child with the given node.
// Note: frees the replaced child.
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

//TODO: make this remove the child from the parent's child-list?
void _t_free(Tnode *t) {
    __t_free_children(t);
    if (t->context.flags & TFLAG_ALLOCATED)
	free(t->contents.surface);
    if (t->context.flags & TFLAG_SURFACE_IS_TREE)
	_t_free((Tnode *)t->contents.surface);
    free(t);
}

Tnode *__t_clone(Tnode *t,Tnode *p) {
    int i,c=_t_children(t);
    Tnode *nt = _t_new(p,_t_symbol(t),_t_surface(t),_t_size(t));
    for(i=1;i<=c;i++) {
	__t_clone(_t_child(t,i),nt);
    }
    return nt;
}

Tnode *_t_clone(Tnode *t) {
    return __t_clone(t,0);
}

/******************** Node data accessors */
int _t_children(Tnode *t) {
    return t->structure.child_count;
}

void * _t_surface(Tnode *t) {
    if (t->context.flags & (TFLAG_ALLOCATED|TFLAG_SURFACE_IS_TREE))
	return t->contents.surface;
    else
	return &t->contents.surface;
}

Symbol _t_symbol(Tnode *t) {
    return t->contents.symbol;
}

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

//TODO: this is very expensive if called all the time!!!
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
int _t_path_equal(int *p1,int *p2){
    while(*p1 != TREE_PATH_TERMINATOR && *p2 != TREE_PATH_TERMINATOR)
	if (*(p1++) != *(p2++)) return 0;
    return *p1 == TREE_PATH_TERMINATOR && *p2 == TREE_PATH_TERMINATOR;
}

int _t_path_depth(int *p) {
    int i=0;
    while(*p++ != TREE_PATH_TERMINATOR) i++;
    return i;
}

//TODO: implement jumping into orthogonal trees  (i.e. return paths with 0 in them)
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
	    raise(SIGINT);
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

void _t_pathcpy(int *dst_p,int *src_p) {
    while(*src_p != TREE_PATH_TERMINATOR) {
	*dst_p++ = *src_p++;
    }
    *dst_p = TREE_PATH_TERMINATOR;
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
