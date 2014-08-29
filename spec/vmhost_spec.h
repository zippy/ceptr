/**
 * @file vmhost_spec.h
 * @copyright Copyright (C) 2013-2014, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 * @ingroup tests
 */

#include "../src/ceptr.h"
#include "../src/vmhost.h"

void testVMHostCreate() {
    //! [testVMHostCreate]
    VMHost *v = _v_new();

    spec_is_symbol_equal(v->r,_t_symbol(v->r->root),RECEPTOR);

    _v_free(v);
    //! [testVMHostCreate]
}

void testVMHostInstallReceptor() {
    //! [testVMHostInstallReceptor]
    VMHost *v = _v_new();

    // create a test receptor that knows about house locations to be installed into
    // the vmhost
    Receptor *r = _r_new();
    Symbol lat = _r_def_symbol(r,FLOAT,"latitude");
    Symbol lon = _r_def_symbol(r,FLOAT,"longitude");
    Structure latlong = _r_def_structure(r,"latlong",2,lat,lon);
    Symbol house_loc = _r_def_symbol(r,latlong,"house location");

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
void testVMHost() {
    testVMHostCreate();
    testVMHostInstallReceptor();
    //   testVMHostActivateReceptor();
}
