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

    Tnode *s_tree = _t_child(v->r->root,4);
    spec_is_symbol_equal(v->r,_t_symbol(s_tree),PENDING_SIGNALS);
    spec_is_ptr_equal(v->pending_signals,s_tree);
    spec_is_equal(_t_children(s_tree),0);

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

#define HELLO_WORLD_UUID 12345
/**
 * generate an HTTP receptor package
 *
 * @snippet spec/vmhost_spec.h makeTestHTTPAppReceptorPackage
 */
//! [makeTestHTTPAppReceptorPackage]
Tnode *_makeTestHTTPAppReceptorPackage() {
    Tnode *p = _t_new_root(RECEPTOR_PACKAGE);
    Tnode *m = _t_newr(p,MANIFEST);
    Tnode *mp = _t_newr(m,MANIFEST_PAIR);
    _t_new(mp,MANIFEST_LABEL,"http_server",12);  // must bind with an http server
    _t_newi(mp,MANIFEST_SPEC,RECEPTOR_XADDR);

    _t_newi(p,RECEPTOR_IDENTIFIER,HELLO_WORLD_UUID);

    Tnode *defs = _t_newr(p,DEFINITIONS);
    _t_add(defs,_t_clone(test_HTTP_structures));
    _t_add(defs,_t_clone(test_HTTP_symbols));
    Tnode *procs = _t_newr(defs,PROCESSES);
    // a process that simply reduces to an HTTP_RESPONSE indicating an off-line status
    Tnode *resp = _t_new_root(RESPOND);
    Tnode *http_resp = _t_newr(resp,HTTP_RESPONSE);
    _t_new(http_resp,HTTP_RESPONSE_CONTENT_TYPE,"Text/Plain",11);
    _t_new(http_resp,TEST_STR_SYMBOL,"Hello World!",13);
    Tnode *input = _t_new_root(INPUT);
    Tnode *output = _t_new_root(OUTPUT_SIGNATURE);
    _d_code_process(procs,resp,"hellow","respond with hello",input,output);

    _t_newr(defs,SCAPES); // for now we don't have any scapes

    return p;
}
//! [makeTestHTTPAppReceptorPackage]

void testVMHostLoadReceptorPackage() {
    //! [testVMHostLoadReceptorPackage]
    VMHost *v = _v_new();
    Tnode *p = _makeTestHTTPAppReceptorPackage();

    Xaddr x = _v_load_receptor_package(v,p);
    Tnode *p1 = _r_get_instance(v->c,x);

    spec_is_ptr_equal(p,p1);
    _v_free(v);
    //! [testVMHostLoadReceptorPackage]
}

void testVMHostInstallReceptor() {
    //! [testVMHostInstallReceptor]
    VMHost *v = _v_new();

    Tnode *p = _makeTestHTTPAppReceptorPackage();
    Xaddr xp = _v_load_receptor_package(v,p);

    Xaddr x = _v_install_r(v,xp,0,"hello world");

    // installing the receptor should instantiate a receptor from the package with the given bindings and symbol label
    Tnode *r = _r_get_instance(v->r,x);
    spec_is_symbol_equal(v->r,x.symbol,INSTALLED_RECEPTOR);
    spec_is_symbol_equal(v->r,_t_symbol(_t_child(r,1)),_r_get_symbol_by_label(v->r,"hello world"));

    // and the definition labels of the instantiated receptor should all be set up properly
    Receptor *httpr = (Receptor *)_t_surface(_t_child(r,1));
    spec_is_equal(_r_get_symbol_by_label(httpr,"HTTP_REQUEST"),HTTP_REQUEST);
    spec_is_equal(_r_get_symbol_by_label(httpr,"hellow"),-1); // hard-coded process symbol

    // trying to re-install the receptor should fail
    x = _v_install_r(v,xp,0,"hellow world");
    spec_is_true(is_null_xaddr(x));

    // because the receptor's id is in the installed_receptors scape
    Tnode *pack = _r_get_instance(v->c,xp);
    Tnode *id = _t_child(pack,2);
    TreeHash h = _t_hash(v->r->defs.symbols,v->r->defs.structures,id);
    spec_is_xaddr_equal(v->r,_s_get(v->installed_receptors,h),xp);

    _v_free(v);
    //! [testVMHostInstallReceptor]
}

#define HTTP_SERVER_RECEPTOR_UUID 4321

void testVMHostActivateReceptor() {
    //! [testVMHostActivateReceptor]
    VMHost *v = _v_new();

    // create and install a stub HTTP server receptor
    Tnode *httpd_rp = _makeTestReceptorPackage(HTTP_SERVER_RECEPTOR_UUID,0,0);  // no symbols or structures yet
    Xaddr httpd_px = _v_load_receptor_package(v,httpd_rp);
    Xaddr httpd_x = _v_install_r(v,httpd_px,0,"hhtp server");
    Tnode *installed_httpd = _r_get_instance(v->r,httpd_x);
    Receptor *httpd_r = (Receptor *)_t_surface(_t_child(installed_httpd,1));

    // create and install an app bound to a URL
    Tnode *p = _makeTestHTTPAppReceptorPackage();
    Xaddr xp = _v_load_receptor_package(v,p);

    Tnode *b = _t_new_root(BINDINGS);
    Tnode *pair = _t_newr(b,BINDING_PAIR);
    _t_new(pair,MANIFEST_LABEL,"http_server",12);
    _t_new(pair,RECEPTOR_XADDR,&httpd_x,sizeof(Xaddr));

    Xaddr x = _v_install_r(v,xp,b,"hello world app");
    Tnode *installed_hellow = _r_get_instance(v->r,x);
    Receptor *hello_r = (Receptor *)_t_surface(_t_child(installed_hellow,1));

    // add a listener that matches on any request with "Host: helloworld.org"
    // /HTTP_REQUEST/.*,HTTP_REQUEST_HOST=helloworld.org
    Tnode *act = _t_newp(0,ACTION,-1);
    Tnode *expect = _t_new_root(EXPECTATION);
    Tnode *req = _t_newi(expect,SEMTREX_SYMBOL_LITERAL,HTTP_REQUEST);
    Tnode *ss = _t_newi(req,SEMTREX_SEQUENCE,0);
    Tnode *sss = _t_newi(ss,SEMTREX_ZERO_OR_MORE,0);
    _t_newi(sss,SEMTREX_SYMBOL_ANY,0);

    char *host = "helloworld.com";
    int size = sizeof(Svalue)+strlen(host)+1;
    Svalue *sv = malloc(size);
    sv->symbol = HTTP_REQUEST_HOST;
    sv->length = strlen(host);
    strcpy((char *)&sv->value,host);
    _t_new(ss,SEMTREX_VALUE_LITERAL,sv,size);

    char buf[2000];
    // just check that we got our semtrex right
    spec_is_str_equal(_dump_semtrex(test_HTTP_defs,req,buf),"/(HTTP_REQUEST/.*,HTTP_REQUEST_HOST=helloworld.com)");
    free(sv);

    _r_add_listener(hello_r,DEFAULT_ASPECT,HTTP_REQUEST,expect,act);

    // simulate that an HTTP_Request signal from the http_server receptor has been sent
    Tnode *s = _t_new_root(HTTP_REQUEST);
    Tnode *s_version = _t_newr(s,HTTP_REQUEST_VERSION);
    _t_newi(s_version,VERSION_MAJOR,1);
    _t_newi(s_version,VERSION_MINOR,1);
    Tnode *s_method = _t_newi(s,HTTP_REQUEST_METHOD,TEST_HTTP_METHOD_GET_VALUE);
    Tnode *s_path = _t_newr(s,HTTP_REQUEST_PATH);
    Tnode *s_host = _t_new(s,HTTP_REQUEST_HOST,"helloworld.com",15);

    Xaddr xs = _v_send(v,x,httpd_x,DEFAULT_ASPECT,s);

    // confirm that the signal has not been delivered and is still in pending list (because receptors aren't active)
    spec_is_equal(_t_children(v->pending_signals),1);

    // activate the two receptors
    _v_activate(v,httpd_x);
    _v_activate(v,x);

    // confirm that the activate list has new children
    spec_is_equal(_t_children(v->active_receptors),2);

    // and that they are the same as the installed receptors
    Tnode *ar;
    ar = _t_child(v->active_receptors,1);
    spec_is_ptr_equal(ar,installed_httpd);
    ar = _t_child(v->active_receptors,2);
    spec_is_ptr_equal(ar,installed_hellow);

    // simulate round-robin processing of signals
    _v_process_signals(v);

    // now confirm that the signal was sent,
    // first that the pending signals list is empty
    spec_is_equal(_t_children(v->pending_signals),0);

    // and second, that a response is back at the http_server
    //@todo

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
