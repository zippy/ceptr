/**
 * @ingroup tree
 *
 * @{
 * @file mtree.h
 * @brief semantic tree matrix header file
 *
 * @copyright Copyright (C) 2013-2014, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 *
 */

#ifndef _CEPTR_MTREE_H
#define _CEPTR_MTREE_H

#include <string.h>
#include <stdlib.h>
#include "ceptr_error.h"
#include "sys_symbols.h"
#include "sys_structures.h"
#include "sys_processes.h"
#include "ceptr_types.h"

H _m_new(H parent,Symbol symbol,void *surface,size_t size);
H _m_newi(H h,Symbol symbol,int surface);
size_t _m_size(H h);
void _m_free(H h);
int _m_children(H h);
void * _m_surface(H h);
Maddr _m_parent(H h);
Maddr _m_child(H h,Mindex i);
Symbol _m_symbol(H h);
Maddr _m_next_sibling(H h);


const H null_H;

#define maddreq(x,y) ((x).i == (y).i && (x).l == (y).l)

#define spec_is_maddr_equal(got, expected) spec_total++; if (maddreq(expected,got)){putchar('.');} else {putchar('F');sprintf(failures[spec_failures++],"%s:%d expected %s to be %d.%d but was %d.%d",__FUNCTION__,__LINE__,#got,expected.l,expected.i,got.l,got.i);}

#endif
/** @}*/
