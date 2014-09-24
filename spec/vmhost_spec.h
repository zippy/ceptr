/**
 * @file vmhost_spec.h
 * @copyright Copyright (C) 2013-2014, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 * @ingroup tests
 */

#include "../src/ceptr.h"
#include "../src/vmhost.h"
#include "http_example.h"

void testVMHostCreate() {
    //! [testVMHostCreate]
    VMHost *v = _v_new();

    spec_is_symbol_equal(v->r,_t_symbol(v->r->root),VM_HOST_RECEPTOR);
    Tnode *ar_tree = _t_child(v->r->root,3);
    spec_is_symbol_equal(v->r,_t_symbol(ar_tree),ACTIVE_RECEPTORS);
    spec_is_ptr_equal(v->active_receptors,ar_tree);
    spec_is_equal(_t_children(ar_tree),0);
    spec_is_equal(v->installed_receptors->key_source,RECEPTOR_IDENTIFIER);
    spec_is_equal(v->installed_receptors->data_source,RECEPTOR);
    _v_free(v);
    //! [testVMHostCreate]
}

/**
 * generate a receptor package
 *
 * @snippet spec/vmhost_spec.h makeTestReceptorPackage
 */
//! [makeTestReceptorPackage]
Tnode *_makeTestReceptorPackage(int uuid,Tnode *symbols,Tnode *structures) {
    Tnode *p = _t_new_root(RECEPTOR_PACKAGE);
    Tnode *m = _t_newr(p,MANIFEST);

    _t_newi(p,RECEPTOR_IDENTIFIER,uuid);
    Tnode *defs = _t_newr(p,DEFINITIONS);
    if (structures) _t_add(defs,_t_clone(structures));
    else _t_newr(defs,STRUCTURES);

    if (symbols) _t_add(defs,_t_clone(symbols));
    else _t_newr(defs,SYMBOLS);

    _t_newr(defs,PROCESSES); // for now we don't have any processes
    _t_newr(defs,SCAPES); // for now we don't have any scapes

    return p;
}
//! [makeTestReceptorPackage]

#define HTTP_RECEPTOR_UUID 12345
/**
 * generate an HTTP receptor package
 *
 * @snippet spec/vmhost_spec.h makeTestHTTPReceptorPackage
 */
//! [makeTestHTTPReceptorPackage]
Tnode *_makeTestHTTPReceptorPackage() {
    Tnode *p = _t_new_root(RECEPTOR_PACKAGE);
    Tnode *m = _t_newr(p,MANIFEST);
    Tnode *mp = _t_newr(m,MANIFEST_PAIR);
    _t_new(mp,MANIFEST_LABEL,"ip_socket",10);
    _t_newi(mp,MANIFEST_SPEC,RECEPTOR_XADDR);

    _t_newi(p,RECEPTOR_IDENTIFIER,HTTP_RECEPTOR_UUID);

    Tnode *defs = _t_newr(p,DEFINITIONS);
    _t_add(defs,_t_clone(test_HTTP_structures));
    _t_add(defs,_t_clone(test_HTTP_symbols));
    Tnode *procs = _t_newr(defs,PROCESSES);
    // a process that simply reduces to an HTTP_RESPONSE indicating an off-line status
    Tnode *resp = _t_new_root(RESPOND);
    Tnode *http_resp = _t_newr(resp,TSYM_HTTP_RESPONSE);
    _t_new(http_resp,TSYM_HTTP_RESPONSE_CONTENT_TYPE,"Text/Plain",11);
    _t_new(http_resp,TEST_STR_SYMBOL,"System offline!",16);
    Tnode *input = _t_new_root(INPUT);
    Tnode *output = _t_new_root(OUTPUT_SIGNATURE);
    _d_code_process(procs,resp,"off-line response","respond with system offline",input,output);

    _t_newr(defs,SCAPES); // for now we don't have any scapes

    return p;
}
//! [makeTestHTTPReceptorPackage]

void testVMHostLoadReceptorPackage() {
    //! [testVMHostLoadReceptorPackage]
    VMHost *v = _v_new();
    Tnode *p = _makeTestHTTPReceptorPackage();

    Xaddr x = _v_load_receptor_package(v,p);
    Tnode *p1 = _r_get_instance(v->c,x);

    spec_is_ptr_equal(p,p1);
    _v_free(v);
    //! [testVMHostLoadReceptorPackage]
}

void testVMHostInstallReceptor() {
    //! [testVMHostInstallReceptor]
    VMHost *v = _v_new();

    Tnode *p = _makeTestHTTPReceptorPackage();
    Xaddr xp = _v_load_receptor_package(v,p);

    Xaddr x = _v_install_r(v,xp,0,"http server");

    // installing the receptor should instantiate a receptor from the package with the given bindings and symbol label
    Tnode *r = _r_get_instance(v->r,x);
    spec_is_symbol_equal(v->r,x.symbol,INSTALLED_RECEPTOR);
    spec_is_symbol_equal(v->r,_t_symbol(_t_child(r,1)),_r_get_symbol_by_label(v->r,"http server"));

    // and the definition labels of the instantiated receptor should all be set up properly
    Receptor *httpr = (Receptor *)_t_surface(_t_child(r,1));
    spec_is_equal(_r_get_symbol_by_label(httpr,"HTTP_REQUEST"),TSYM_HTTP_REQUEST);
    spec_is_equal(_r_get_symbol_by_label(httpr,"off-line response"),-1); // hard-coded process symbol

    // trying to re-install the receptor should fail
    x = _v_install_r(v,xp,0,"http server");
    spec_is_true(is_null_xaddr(x));

    _v_free(v);
    //! [testVMHostInstallReceptor]
}

#define IP_SOCKET_RECEPTOR_UUID 4321

void testVMHostActivateReceptor() {
    //! [testVMHostActivateReceptor]
    VMHost *v = _v_new();

    // create and install a stub IP receptor
    Tnode *ipr = _makeTestReceptorPackage(IP_SOCKET_RECEPTOR_UUID,0,0);  // no symbols or structures yet
    Xaddr ipxp = _v_load_receptor_package(v,ipr);
    Xaddr ipx = _v_install_r(v,ipxp,0,"server socket");
    Tnode *installed_ip = _r_get_instance(v->r,ipx);
    Receptor *ipsocketr = (Receptor *)_t_surface(_t_child(installed_ip,1));

    // create and install an http server bound to the IP socket
    Tnode *p = _makeTestHTTPReceptorPackage();
    Xaddr xp = _v_load_receptor_package(v,p);

    Tnode *b = _t_new_root(BINDINGS);
    Tnode *pair = _t_newr(b,BINDING_PAIR);
    _t_new(pair,MANIFEST_LABEL,"ip_socket",10);
    _t_new(pair,RECEPTOR_XADDR,&ipx,sizeof(Xaddr));

    Xaddr x = _v_install_r(v,xp,b,"http server");
    Tnode *installed_http = _r_get_instance(v->r,x);
    Receptor *httpr = (Receptor *)_t_surface(_t_child(installed_http,1));

    // add a listener that matches on any request and returns the "offline" response
    Tnode *act = _t_newp(0,ACTION,-1);
    Tnode *expect = _t_new_root(EXPECTATION);
    Tnode *req = _t_newi(expect,SEMTREX_SYMBOL_LITERAL,TSYM_HTTP_REQUEST);
    _r_add_listener(httpr,DEFAULT_ASPECT,TSYM_HTTP_REQUEST,expect,act);

    // create a signal and add it to the sockets sending processing queue
    Tnode *signal = _makeTestHTTPRequestTree(); // GET /groups/5/users.json?sort_by=last_name?page=2 HTTP/1.0

    //@todo
    // confirm that the signal has not been sent (receptor is not active)

    // activate the two receptors
    _v_activate(v,ipx);
    _v_activate(v,x);

    // confirm that the activate list has new children
    spec_is_equal(_t_children(v->active_receptors),2);

    // and that they are the same as the installed receptors
    Tnode *ar;
    ar = _t_child(v->active_receptors,1);
    spec_is_ptr_equal(ar,installed_ip);
    ar = _t_child(v->active_receptors,2);
    spec_is_ptr_equal(ar,installed_http);

    // now confirm that the signal was sent and processed and that a response is back at the socket
    //@todo

    Tnode *s = _r_send(httpr,ipsocketr,DEFAULT_ASPECT,signal);
    Tnode *result = _t_child(_t_child(s,1),1);

    spec_is_str_equal(_td(httpr,result)," (HTTP_RESPONSE (HTTP_RESPONSE_CONTENT_TYPE:Text/Plain) (TEST_STR_SYMBOL:System offline!))");

    //        puts(_td(httpr,httpr->root));
    _t_free(b);
    _v_free(v);
    //! [testVMHostActivateReceptor]
}

void testVMHost() {
    _setup_HTTPDefs();
    testVMHostCreate();
    testVMHostLoadReceptorPackage();
    testVMHostInstallReceptor();
    testVMHostActivateReceptor();
    _cleanup_HTTPDefs();
}
