/**
 * @ingroup receptor
 *
 * @{
 * @file scape.c
 * @brief implementation of scapes
 *
 * @copyright Copyright (C) 2013-2014, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 */

#include "scape.h"

Scape *__s_new(Symbol sym,Tnode *key_spec,Tnode *data_spec,Tnode *geometry_spec) {
    Scape *s = malloc(sizeof(Scape));
    s->root = _t_new_root(sym);
    s->data = NULL;
    return s;
}

/**
 * create a new scape
 *
 * @params[in] name a label for the scape
 * @returns a pointer to a newly allocated Scape
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/scape_spec.h testScapeNew
 */
Scape *_s_new(Symbol sym) {
    return __s_new(sym,0,0,0);
}

// free the hash table data
void scapedataFree(ScapeData *i) {
    scape_elem *cur,*tmp;
    HASH_ITER(hh, *i, cur, tmp) {
	//@todo free the contents of the element here
	HASH_DEL(*i,cur);  /* delete; cur advances to next */
	free(cur);
    }
}

/**
 * free the memory allocated to a scape
 */
void _s_free(Scape *s) {
    _t_free(s->root);
    scapedataFree(&s->data);
    free(s);
}
/**
 * add a new element into a scape
 *
 * @params[in] xaddr the instance address to be scaped
 * @params[in] key the key which will provide access to the instance
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/scape_spec.h testScapeAddElement
 */

/** @}*/
