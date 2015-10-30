/**
 * @ingroup tree
 *
 * @{
 * @file mtree.h
 * @brief semantic tree matrix header file
 *``
 * @copyright Copyright (C) 2013-2015, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 *
 */

#ifndef _CEPTR_MTREE_H
#define _CEPTR_MTREE_H

#include <string.h>
#include <stdlib.h>
#include "ceptr_error.h"
#include "sys_defs.h"
#include "ceptr_types.h"

typedef struct MwalkState {
    Mindex i;
    union {
	Mindex pi;
	T *t;
    } user;
} MwalkState;

H __m_new(H parent,Symbol symbol,void *surface,size_t size,uint32_t flags);
#define _m_new(parent,symbol,surface,size) __m_new(parent,symbol,surface,size,0)
#define _m_newt(parent,symbol,h) __m_new(parent,symbol,&(h),sizeof(H),TFLAG_SURFACE_IS_TREE)

H _m_newi(H h,Symbol symbol,int surface);
H _m_new_from_t(T *t);
T *_t_new_from_m(H h);
N *__m_get(H h);
size_t _m_size(H h);
#define _m_free(h) __m_free(h,1)
void __m_free(H h,int free_surface);
int _m_children(H h);
void * _m_surface(H h);
Maddr _m_parent(H h);
Maddr _m_child(H h,Mindex i);
Symbol _m_symbol(H h);
Maddr _m_next_sibling(H h);
H _m_new_root(Symbol s);
H _m_newr(H parent,Symbol s);
H _m_add(H parent,H h);
H _m_detatch(H h);
S * _m_serialize(M *m);
H _m_unserialize(S *);

void _m_walk(H h,void (*walkfn)(H ,N *,void *,MwalkState *,Maddr),void *user_data);

const H null_H;

#define maddreq(x,y) ((x).i == (y).i && (x).l == (y).l)


/*****************  Misc... */

#define DO_KIDSM(h,x) {int i,_c=_m_children(h);for(i=1;i<=_c;i++){x;}}

#define _GET_LEVEL(h,l) (&(h).m->lP[l])
#define _GET_NODE(h,l,i) (&(l)->nP[i])
#define GET_LEVEL(h) _GET_LEVEL(h,(h).a.l)
#define GET_NODE(h,l) _GET_NODE(h,l,(h).a.i)

#define spec_is_maddr_equal(got, expected) spec_total++; if (maddreq(expected,got)){putchar('.');} else {putchar('F');sprintf(failures[spec_failures++],"%s:%d expected %s to be %d.%d but was %d.%d",__FUNCTION__,__LINE__,#got,expected.l,expected.i,got.l,got.i);}

#endif
/** @}*/
