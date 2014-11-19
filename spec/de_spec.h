/**
 * @file de_spec.h
 * @copyright Copyright (C) 2013-2014, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 * @ingroup tests
 */

#include "../src/ceptr.h"
#include "../src/de.h"

void testBootStrap() {
    _de_boot()
    spec_is_true(0);
}

void testDataEngine() {
    testBootStrap();
}
