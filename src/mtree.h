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

M *_m_new(Mpath parent,Symbol symbol,void *surface,size_t size);
size_t _m_size(M *addr);
void _m_free(M *m);
int _m_children(M *m);
void * _m_surface(M *m);


#endif
/** @}*/
