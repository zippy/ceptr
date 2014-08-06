#include "tree.h"
#include "ceptr_error.h"

/*****************  Node creation */
int __t_append_child(Tnode *t,Tnode *c) {
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

// NOTE: does not free the memory occupied by c
void _t_remove(Tnode *t,Tnode *c) {
    int i;
    int l = _t_children(t);

    // search for the child to be removed
    for(i=0;i<=l;i++) {
	if (_t_child(t,i) == c) break;
    }

    // if found remove it by decreasing the child count and shift all the other children down
    if (i <= l) {
	t->structure.child_count--;
	for(;i<l;i++) {
	    t->structure.children[i-1] = t->structure.children[i];
	}
    }
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
    free(t);
}

/******************** Node data accessors */
int _t_children(Tnode *t) {
    return t->structure.child_count;
}

void * _t_surface(Tnode *t) {
    if (t->context.flags & TFLAG_ALLOCATED)
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
    Tnode *p = _t_parent(t);
    if (p==0) return 0;
    int c = _t_children(p);
    int i;
    for(i=0;i<c;i++) {
	if (p->structure.children[i] == t) {
	    return i+1;
	}
    }
    return 0;
}

//TODO: this is very expensive if called all the time!!!
Tnode * _t_next_sibling(Tnode *t) {
    Tnode *p = _t_parent(t);
    if (p==0) return 0;
    int c = _t_children(p);
    int i;
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
	if (++i >= s) {s*=2;p=realloc(p,s);} // realloc array if tree too deep
    }
    if (i > 2) {
	// reverse the list by swapping elements going from the outside to the center
	i-=2;
	l = i+1;
	k = i/2;
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
    int i;
    for(i=0;i<d;i++) {
	sprintf(b,"/%d",fp[i]);
	b += strlen(b);
    }
    return buf;
}

/*****************  Tree debugging utilities */

char __t_dump_buf[10000];

void __t_dump(Tnode *t,int level,char *buf) {
    Symbol s = _t_symbol(t);
    char b[255];
    switch(s) {
    case TEST_SYMBOL:
	sprintf(buf," (TEST_SYMBOL:%s",(char *)_t_surface(t));
	break;
    case TREE_PATH:
	sprintf(buf," (TREE_PATH:%s",_t_sprint_path((int *)_t_surface(t),b));
	break;
    case SEMTREX_SYMBOL_LITERAL:
	sprintf(buf," (LITERAL:%d",*(int *)_t_surface(t));
	break;
    case SEMTREX_MATCH:
	sprintf(buf," (STX-MATCH:%d",*(int *)_t_surface(t));
	break;
    case SEMTREX_MATCH_SIBLINGS_COUNT:
	sprintf(buf," (STX-M-SIBS:%d",*(int *)_t_surface(t));
	break;
    case SEMTREX_MATCH_RESULTS:
	sprintf(buf," (STX-MATCH-RESULTS");
	break;
    case RECEPTOR:
	sprintf(buf," (RECEPTOR");
	break;
    case FLUX:
	sprintf(buf," (FLUX");
	break;
    case ASPECT:
	sprintf(buf," (ASPECT:%d",*(int *)_t_surface(t));
	break;
    case ACTION:
	sprintf(buf," (ACTION");
	break;
    case EXPECTATION:
	sprintf(buf," (EXPECTATION");
	break;
    case EXPECTATIONS:
	sprintf(buf," (EXPECTATIONS");
	break;
    default:
	sprintf(buf," (%d",s);
    }
    int i;
    for(i=1;i<=_t_children(t);i++) __t_dump(_t_child(t,i),level+1,buf+strlen(buf));
    sprintf(buf+strlen(buf),")");
}

char *_td(Tnode *t) {
    __t_dump(t,0,__t_dump_buf);
    return __t_dump_buf;
}
