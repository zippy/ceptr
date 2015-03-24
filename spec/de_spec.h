/**
 * @file de_spec.h
 * @copyright Copyright (C) 2013-2014, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 * @ingroup tests
 */

#include "../src/ceptr.h"
#include "../src/de.h"

void testBootStrap() {
    spec_is_ptr_equal(G_vm,NULL);
    _de_boot();
    spec_is_symbol_equal(G_vm->r,_t_symbol(G_vm->r->root),VM_HOST_RECEPTOR);
    _v_free(G_vm);
}

void testDataEngine() {
    testBootStrap();
}
