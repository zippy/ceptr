/**
 * @file scape_spec.h
 * @copyright Copyright (C) 2013-2014, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 * @ingroup tests
 */

#include "../src/ceptr.h"
#include "../src/scape.h"
#include "../src/receptor.h"

void testScapeNew() {
    Scape *s = _s_new(TEST_ALPHABETIZE_SCAPE_SYMBOL);
    spec_is_symbol_equal(0,_t_symbol(s->root),TEST_ALPHABETIZE_SCAPE_SYMBOL);
    _s_free(s);
}

void testScapeAddElement() {
    //! [testScapeAddElement]
    Receptor *r = _r_new(TEST_RECEPTOR_SYMBOL);
    Scape *s = _s_new(TEST_ALPHABETIZE_SCAPE_SYMBOL);
    Xaddr x;
    //    _s_add(x);
    //! [testScapeAddElement]
    _r_free(r);
}

void testScape() {
    testScapeNew();
    testScapeAddElement();
}
