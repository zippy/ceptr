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
    L *l;

    if (parent.m) {
	h.m = parent.m;
	h.a.l = parent.a.l+1;

	if (parent.a.l >= parent.m->levels) {
	    raise_error0("address too deep!");
	}
	else if (parent.a.l == parent.m->levels-1) {
	    h.a.i = 0;
	    __m_add_level(h.m);
	}
	else {
	    Maddr x = _m_child(parent,NULL_ADDR);
	    h.a.i = x.i+1;
	}
	l = &h.m->lP[h.a.l];
    }
    else {
	h.m  = malloc(sizeof(M));
	h.m->magic = matrixImpl;
	h.m->levels = 0;
	h.a.l = 0;
	h.a.i = 0;
	__m_add_level(h.m);
	l = &h.m->lP[h.a.l];
    }

    // add a node
    // @todo make this not realloc each time!!
    N *n,*nl;
    if (!l->nodes++) {
    	l->nP = malloc(sizeof(N));
	n = &l->nP[h.a.i];
    }
    else {
    	l->nP = realloc(l->nP,sizeof(N)*l->nodes);
	n = &l->nP[h.a.i];
	nl =  &l->nP[l->nodes-1];
	while (nl != n) {
	    *nl = *(nl-1);
	    nl--;
	}
    }
    n->symbol = symbol;
    n->size = size;
    n->parenti = parent.m ? parent.a.i : 0;
    if (size) {
	if (size == sizeof(int)) {
	    n->flags = 0;
	    *((int *)&n->surface) = *(int *)surface;
	}
	else {
	    n->flags = TFLAG_ALLOCATED;
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
    L *l = &h.m->lP[h.a.l];
    N *n = &l->nP[h.a.i];
    return n;
}

size_t _m_size(H h) {return __m_get(h)->size;}

void _m_free(H h) { free(h.m);}

int _m_children(H h) {
    Mlevel levels = h.m->levels;

    if (h.a.l >= levels) {
	raise_error0("address too deep!");
    }
    else if (h.a.l == levels-1) {
	return 0;
    }
    L *l = &h.m->lP[h.a.l+1];
    Mindex c = 0;
    Mindex i = 0;
    Mindex max = l->nodes;
    N *n = &l->nP[0];
    while (i < max && n->parenti != h.a.i) {
	n++;i++;
    }
    while (i < max && n->parenti == h.a.i) {
	n++;i++;c++;
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
    a.i = 0;
    Mindex max = l->nodes;
    N *n = &l->nP[0];

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
    return a;
}

Symbol _m_symbol(H h) {
    N *n = __m_get(h);
    return n->symbol;
}

Maddr _m_next_sibling(H h) {
    L *l = &h.m->lP[h.a.l];
    int i = h.a.i+1;
    Maddr r;
    if (i<l->nodes) {
	N *n = &l->nP[i], *no = &l->nP[h.a.i];
	if (n->parenti == no->parenti) {
	    r.l = h.a.l;
	    r.i = i;
	    return r;
	}
    }
    return null_H.a;
}
/** @}*/
