/**
 * @file de_spec.h
 * @copyright Copyright (C) 2013-2014, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 * @ingroup tests
 */

#include "../src/ceptr.h"
#include "../src/accumulator.h"

void testAccBootStrap() {
    spec_is_ptr_equal(G_vm,NULL);
    _a_boot();
    spec_is_symbol_equal(G_vm->r,_t_symbol(G_vm->r->root),VM_HOST_RECEPTOR);
    _v_free(G_vm);
}

void testAccInstances() {
    Instances i = NULL;
    T *t = _t_new_root(PARAMS);

    Xaddr x = _a_new_instance(&i,t);
    spec_is_ptr_equal(t,_a_get_instance(&i,x));

    _a_free_instances(&i);
}

void testAccPersistInstances() {
    Instances i = NULL;

    T *it = _t_newi(0,TEST_INT_SYMBOL,1);
    _a_new_instance(&i,it);

    it = _t_newi(0,TEST_INT_SYMBOL,2);
    Xaddr z = _a_new_instance(&i,it);

    T *t = _t_new_root(PARAMS);
    _t_newi(t,TEST_INT_SYMBOL,314);
    Xaddr x = _a_new_instance(&i,t);

    T *ht = _makeTestHTTPRequestTree(); // GET /groups/5/users.json?sort_by=last_name?page=2 HTTP/1.0
    T *htc = _t_clone(ht);
    Xaddr y = _a_new_instance(&i,ht);

    char *file = "test/test.instances";

    _a_serialize_instances(&i,file);
    _a_free_instances(&i);

    _a_unserialize_instances(&i,file);

    spec_is_str_equal(_t2s(&test_HTTP_defs,_a_get_instance(&i,x)),"(PARAMS (TEST_INT_SYMBOL:314))");
    spec_is_str_equal(_t2s(&test_HTTP_defs,_a_get_instance(&i,y)),_t2s(&test_HTTP_defs,htc));
    spec_is_str_equal(_t2s(&test_HTTP_defs,_a_get_instance(&i,z)),"(TEST_INT_SYMBOL:2)");

    _a_free_instances(&i);
    _t_free(htc);
}

void testAccumulator() {
    _setup_HTTPDefs();
    testAccBootStrap();
    testAccInstances();
    testAccPersistInstances();
    _cleanup_HTTPDefs();
}
