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

/**
 * Create a new tree node
 *
 * @param[in] parent handle to parent node
 * @param[in] symbol semantic symbol for the node to be create
 * @param[in] surface pointer to node's data
 * @param[in] size size in bytes of the surface
 * @returns updated handle
 */
M *_m_new(Mpath parent,Symbol symbol,void *surface,size_t size) {
    M *m =  malloc(sizeof(M));
    m->magic = matrixImpl;
    m->size = size;
    return m;
}

size_t _m_size(M *addr) {return addr->size;}

void _m_free(M *m) { free(m);}

int _m_children(M *m) { return 0;}

void * _m_surface(M *m) {
    return "fish";
}

/** @}*/
