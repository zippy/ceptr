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

/**
 * create a new scape
 *
 * @params[in] key_source the symbol type key hashes are of
 * @params[in] data_source the symbol type of xaddrs to be associated with keys
 * @returns a pointer to a newly allocated Scape
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/scape_spec.h testScapeNew
 */
Scape *_s_new(Symbol key_source,Symbol data_source) {
    Scape *s = malloc(sizeof(Scape));
    s->key_source = key_source;
    s->data_source = data_source;
    s->data = NULL;
    return s;
}

// free the hash table data
void scapedataFree(ScapeData *i) {
    scape_elem *cur,*tmp;
    HASH_ITER(hh, *i, cur, tmp) {
	HASH_DEL(*i,cur);  /* delete; cur advances to next */
	free(cur);
    }
}

/**
 * free the memory allocated to a scape
 */
void _s_free(Scape *s) {
    scapedataFree(&s->data);
    free(s);
}
/**
 * add a new element into a scape
 *
 * @params[in] s the scape
 * @params[in] key the tree node which is the key
 * @params[in] xaddr the instance address to be scaped
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/scape_spec.h testScapeAddElement
 */
void _s_add(Scape *s,TreeHash h,Xaddr x) {
    ScapeData *data = &s->data;
    scape_elem *e;

    HASH_FIND_INT( *data, &h, e );
    if (e) {
	raise_error0("allready there!");
    }
    else {
	e = malloc(sizeof(struct scape_elem));
	e->key = h;
	e->value = x;
	HASH_ADD_INT(*data,key,e);
    }
}

/**
 * retrieve an Xaddr from the scape
 *
 * @params[in] s the scape
 * @params[in] key the tree node which is the key
 * @returns Xaddr to the scape item
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/scape_spec.h testScapeAddElement
 */
Xaddr _s_get(Scape *s,TreeHash h) {
    Xaddr x = {0,0};
    scape_elem *e = 0;
    ScapeData *data = &s->data;

    HASH_FIND_INT( *data, &h, e );
    if (e) return e->value;
    return x;
}

/** @}*/
