/**
 * @file vmhost_spec.h
 * @copyright Copyright (C) 2013-2014, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 * @ingroup tests
 */

#include "../src/ceptr.h"
#include "../src/vmhost.h"
#include "tree_http_example.h"

void testVMHostCreate() {
    //! [testVMHostCreate]
    VMHost *v = _v_new();

    spec_is_symbol_equal(v->r,_t_symbol(v->r->root),VM_HOST_RECEPTOR);
    Tnode *ar_tree = _t_child(v->r->root,4);
    spec_is_symbol_equal(v->r,_t_symbol(ar_tree),ACTIVE_RECEPTORS);
    spec_is_ptr_equal(v->active_receptors,ar_tree);
    spec_is_equal(_t_children(ar_tree),0);
    _v_free(v);
    //! [testVMHostCreate]
}

/**
 * generate a test receptor with house location symbols
 *
 * @snippet spec/vmhost_spec.h makeTestHouseLocReceptor
 */
//! [makeTestHouseLocReceptor]
Receptor * _makeTestHouseLocReceptor() {
    Receptor *r = _r_new(TEST_RECEPTOR_SYMBOL);
    Symbol lat = _r_def_symbol(r,FLOAT,"latitude");
    Symbol lon = _r_def_symbol(r,FLOAT,"longitude");
    Structure latlong = _r_def_structure(r,"latlong",2,lat,lon);
    Symbol house_loc = _r_def_symbol(r,latlong,"house location");
    return r;
}
//! [makeTestHouseLocReceptor]

void testVMHostInstallReceptor() {
    //! [testVMHostInstallReceptor]
    VMHost *v = _v_new();

    Receptor *r = _makeTestHouseLocReceptor();
    Xaddr x = _v_install_r(v,r,"house locator");

    // installing the receptor should create a RECEPTOR_PACKAGE symbol for it in the context of the vmhost
    spec_is_equal(x.symbol,_r_get_symbol_by_label(v->r,"house locator"));

    // and the instance should be a RECEPTOR_PACKAGE that holds the serialized receptor
    Instance i = _r_get_instance(v->r,x);

    void *surface;
    size_t length;
    _r_serialize(r,&surface,&length);

    spec_is_buffer_equal(i.surface,surface,length);

    _v_free(v);
    free(surface);
    _r_free(r);
    //! [testVMHostInstallReceptor]
}

void testVMHostActivateReceptor() {
    //! [testVMHostActivateReceptor]
    VMHost *v = _v_new();

    Receptor *r = _makeTestHouseLocReceptor();
    Xaddr x = _v_install_r(v,r,"house locator");

    _v_activate(v,x);
    spec_is_equal(_t_children(v->active_receptors),1);
    Receptor *active_r = *(Receptor **)_t_surface(_t_child(v->active_receptors,1));

    // a token test to make sure the active receptor looks like the one we installed
    spec_is_equal(_r_get_symbol_by_label(r,"latitude"),_r_get_symbol_by_label(active_r,"latitude"));

    _r_free(r);
    _v_free(v);

    //! [testVMHostActivateReceptor]
}

void testVMHost() {
    testVMHostCreate();
    testVMHostInstallReceptor();
    testVMHostActivateReceptor();
}
