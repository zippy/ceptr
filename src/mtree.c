/**
 * @ingroup tree
 *
 * @{
 *
 * @file mtree.c
 * @brief semantic tree matrix implementation
 *
 * @copyright Copyright (C) 2013-2014, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 */

#include "mtree.h"
#include "ceptr_error.h"
#include "hashfn.h"
#include "def.h"

const H null_H = {0,{NULL_ADDR,NULL_ADDR}};

// @todo make this not realloc each time?
void __m_add_level(M *m) {
    if (!m->levels++) {
	m->lP = malloc(sizeof(L));
    }
    else {
	m->lP = realloc(m->lP,sizeof(L)*m->levels);
    }
    int i = m->levels-1;
    m->lP[i].nodes = 0;
}

// @todo make this not realloc each time!!
N *__m_add_nodes(H h,L *l,int c) {
    N *n;
    Mindex i = l->nodes;
    if (!i) {
    	l->nP = malloc(sizeof(N)*c);
	l->nodes = c;
    }
    else {
	l->nodes += c;
    	l->nP = realloc(l->nP,sizeof(N)*l->nodes);
    }
    n = _GET_NODE(h,l,i);
    return n;
}

void __m_new_init(H parent,H *h,L **l) {
    h->m = parent.m;
    h->a.l = parent.a.l+1;

    if (parent.a.l >= parent.m->levels) {
	raise_error0("address too deep!");
    }
    else if (parent.a.l == parent.m->levels-1) {
	h->a.i = 0;
	__m_add_level(h->m);
	*l = GET_LEVEL(*h);
    }
    else {
	*l = GET_LEVEL(*h);
	h->a.i = (*l)->nodes;
    }
}

void __m_new_root(H *h, L **l) {
    M *m = h->m = malloc(sizeof(M));
    m->magic = matrixImpl;
    m->levels = 0;
    h->a.l = 0;
    h->a.i = 0;
    __m_add_level(m);
    *l = GET_LEVEL(*h);
}

/**
 * Create a new tree node
 *
 * @param[in] parent handle to parent node
 * @param[in] symbol semantic symbol for the node to be create
 * @param[in] surface pointer to node's data
 * @param[in] size size in bytes of the surface
 * @returns updated handle
 */
H _m_new(H parent,Symbol symbol,void *surface,size_t size) {
    H h;
    L *l = 0;

    if (parent.m) {
	__m_new_init(parent,&h,&l);
    }
    else {
	__m_new_root(&h,&l);
    }

    // add a node
    N *n,*nl;
    n = __m_add_nodes(h,l,1);

    n->symbol = symbol;
    n->size = size;
    n->parenti = parent.m ? parent.a.i : 0;
    n->flags = 0;
    if (size) {
	if (size == sizeof(int)) {
	    *((int *)&n->surface) = *(int *)surface;
	}
	else {
	    n->flags |= TFLAG_ALLOCATED;
	    n->surface = malloc(size);
	    if (surface)
		memcpy(n->surface,surface,size);
	}
    }
    else {
	n->flags = 0;
    }

    return h;
}

H _m_newi(H parent,Symbol symbol,int surface) {
    return _m_new(parent,symbol,&surface,sizeof(int));
}

N *__m_get(H h) {
    L *l = GET_LEVEL(h);
    N *n = GET_NODE(h,l);
    return n;
}

size_t _m_size(H h) {return __m_get(h)->size;}

void __m_free(H h,int free_surface) {
    int i = h.m->levels;
    while(i--) {
	L *l = _GET_LEVEL(h,i);
	Mindex j = l->nodes;
	if (free_surface) {
	    while(j--) {
		N *n = _GET_NODE(h,l,j);
		if (n->flags & TFLAG_ALLOCATED) {
		    free(n->surface);
		}
	    }
	}
	free(l->nP);
    }
    free(h.m->lP);
    free(h.m);
}

int _m_children(H h) {
    Mlevel levels = h.m->levels;

    if (h.a.l >= levels) {
	raise_error0("address too deep!");
    }
    else if (h.a.l == levels-1) {
	return 0;
    }
    L *l = _GET_LEVEL(h,h.a.l+1);
    Mindex c = 0;
    Mindex i = 0,pi = h.a.i;
    Mindex max = l->nodes;
    N *n = &l->nP[0];

/* this works if nodes are sorted
    while (i < max && n->parenti != h.a.i) {
	n++;i++;
    }
    while (i < max && n->parenti == h.a.i) {
	n++;i++;c++;
    }
*/
    while (i < max) {
	if (!(n->flags & TFLAG_DELETED) && pi == n->parenti) c++;
	n++;i++;
    }

    return c;
}

void * _m_surface(H h) {
    N *n = __m_get(h);
    if (n->flags & TFLAG_ALLOCATED)
	return n->surface;
    else
	return &n->surface;
}

Maddr _m_parent(H h) {
    Maddr a = {NULL_ADDR,NULL_ADDR};
    if (h.a.l > 0) {
	N *n = __m_get(h);
	a.l = h.a.l-1;
	a.i = n->parenti;
    }
    return a;
}

Maddr _m_child(H h,Mindex c) {
    Maddr a = {NULL_ADDR,NULL_ADDR};
    Mlevel levels = h.m->levels;
    if (h.a.l >= levels) {
	raise_error0("address too deep!");
    }
    else if (h.a.l == levels-1) {
	return a;
    }
    a.l = h.a.l+1;
    L *l = &h.m->lP[a.l];
    Mindex ci = 0,max = l->nodes;
    N *n = &l->nP[0];
    if (c == NULL_ADDR) {
	a.i = NULL_ADDR;
	while(ci < max) {
	    if (n->parenti == h.a.i) a.i = ci;
	    ci++;
	    n++;
	}
	if (a.i == NULL_ADDR)
	    a.l = NULL_ADDR;
    }
    else {
	a.i = 0;
	while(a.i < max) {
	    if (n->parenti == h.a.i) ci++;
	    if (ci == c) return a;
	    a.i++;n++;
	}
	a.l = NULL_ADDR;
	a.i = NULL_ADDR;
    }
/* works if nodes are sorted
    //skip past nodes of children of parents before our parent
    while (a.i < max && n->parenti < h.a.i) {
	a.i++;n++;
    }

    // if you pass in NULL_ADDR for the child,
    // this routine returns the last child address
    if (c == NULL_ADDR) {
	while (a.i < max && n->parenti == h.a.i) {
	    a.i++;n++;
	}
	a.i--;
	if (a.i == -1) a.l = NULL_ADDR;
    }
    else {
	if (a.i+c > l->nodes) {
	    raise_error0("address too deep!");
	}
	a.i += c-1;
    }
*/

    return a;
}

Symbol _m_symbol(H h) {
    N *n = __m_get(h);
    return n->symbol;
}

Maddr _m_next_sibling(H h) {
    L *l = GET_LEVEL(h);
    int i = h.a.i+1;
    Maddr r;
    N *n = GET_NODE(h,l);
    Mindex pi = n->parenti;
    while (i<l->nodes) {
	n++;
	if (n->parenti == pi) {
	    r.l = h.a.l;
	    r.i = i;
	    return r;
	}
	i++;
    }
    return null_H.a;
}

H _m_new_root(Symbol s) {
    return _m_new(null_H,s,0,0);
}

H _m_newr(H parent,Symbol s) {
    return _m_new(parent,s,0,0);
}

H _m_add(H parent,H h) {
    L *pl,*l;
    H r;
    int x = _m_children(parent)+1;
    int i,levels = h.m->levels;
    H p = parent;
    Mindex d = parent.a.i;
    for (i=0;i<levels;i++) {
	__m_new_init(p,&r,&pl);
	l = _GET_LEVEL(h,i);
	N *np = __m_add_nodes(r,pl,l->nodes);
	N *n = &l->nP[0];
	Mindex j = l->nodes;
	while (j--)  {
	    *np = *n;
	    np->parenti += d;
	    np++; n++;
	}
	d = pl->nodes-l->nodes;
	p.a.l++;
    }
    r.a = _m_child(parent,x);

    __m_free(h,0);  //free h but not it's surfaces which were copied into the parent
    return r;
}

void _m_walk(H h,void (*walkfn)(H ,N *,void *,Maddr,Maddr),void *user_data,Maddr ac,Maddr ap) {
    int levels = h.m->levels;
    L *l = GET_LEVEL(h);
    N *n = GET_NODE(h,l);
    (*walkfn)(h,n,user_data,ac,ap);
    h.a.l++;
    Maddr a = ac;
    Mindex pi = h.a.i;
    if(h.a.l<levels) {
	a.l++;

	l = GET_LEVEL(h);
	h.a.i = 0;
	n = GET_NODE(h,l);
	int f = 1;
	while(h.a.i<l->nodes) {
	    if (n->parenti == pi) {
		_m_walk(h,walkfn,user_data,a,ac);
		a.i++;
	    }
	    n++;
	    h.a.i++;
	    f = 0;
	}
    }
}

void _m_detatchfn(H oh,N *on,void *data,Maddr ac,Maddr ap) {
    struct {M *m;} *d = data;
    //    printf("\noh:%d.%d  ",oh.a.l,oh.a.i);
    // printf("a:%d.%d",ac.l,ac.i);

    H parent;

    H h;
    L *l;
    if (!d->m) {
	__m_new_root(&h,&l);
	parent.m = 0;
	d->m = h.m;
    }
    else {
	parent.m = d->m;
	parent.a = ap;
	__m_new_init(parent,&h,&l);
    }

    N *n,*nl;
    n = __m_add_nodes(h,l,1);

    // everything is the same except the parenti
    *n = *on;
    on->flags = TFLAG_DELETED;
    on->surface = 0;
    n->parenti = parent.m ? parent.a.i : 0;
    //   mtd(h);

}

H _m_detatch(H oh) {
    struct {M *m;} d = {NULL};
    Maddr ac = {0,0};
    _m_walk(oh,_m_detatchfn,&d,ac,null_H.a);
    H h = {d.m,{0,0}};
    return h;
}

/** @}*/
