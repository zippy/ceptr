/**
 * @file group_spec.h
 * @copyright Copyright (C) 2013-2015, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 * @ingroup tests
 */

#include "../src/group.h"

void testGroupCreate() {
    Receptor *r = makeGroup(G_vm,"ceptr chat");
    spec_is_str_equal(_r_get_symbol_name(G_vm->r,_t_symbol(r->root)),"ceptr chat");
}

void testGroup() {
    G_vm = _v_new();
    testGroupCreate();
    _v_free(G_vm);
}
