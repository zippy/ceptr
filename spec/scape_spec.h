/**
 * @file scape_spec.h
 * @copyright Copyright (C) 2013-2015, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 * @ingroup tests
 */

#include "../src/ceptr.h"
#include "../src/scape.h"
#include "../src/receptor.h"

void testScapeNew() {
    //! [testScapeNew]
    Scape *s = _s_new(TEST_INT_SYMBOL,TEST_STR_SYMBOL);
    spec_is_symbol_equal(0,s->key_source,TEST_INT_SYMBOL);
    spec_is_symbol_equal(0,s->data_source,TEST_STR_SYMBOL);
    _s_free(s);
    //! [testScapeNew]
}

void testScapeAddElement() {
    //! [testScapeAddElement]
    Scape *s = _s_new(TEST_INT_SYMBOL,TEST_STR_SYMBOL);
    Xaddr x = {1,3};  // DUMMY XADDR
    T *t = _t_newi(0,TEST_INT_SYMBOL,31415);
    TreeHash h = _t_hash(0,0,t);
    _s_add(s,h,x);

    // check that element was added
    Xaddr xg = _s_get(s,h);
    spec_is_sem_equal(xg.symbol,x.symbol);
    spec_is_equal(xg.addr,x.addr);

    // check that a different hash returns the null xaddr
    t->contents.symbol = TEST_INT_SYMBOL2;
    h = _t_hash(0,0,t);
    xg = _s_get(s,h);
    spec_is_true(is_null_xaddr(xg));

    _s_free(s);
    _t_free(t);
    //! [testScapeAddElement]
}

void testScape() {
    testScapeNew();
    testScapeAddElement();
}
