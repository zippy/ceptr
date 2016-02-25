/**
 * @file vmhost_spec.h
 * @copyright Copyright (C) 2013-2016, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 * @ingroup tests
 */

#include "../src/ceptr.h"
#include "../src/vmhost.h"
#include "http_example.h"
#include "../src/shell.h"
#include "../src/protocol.h"

void testVMHostCreate() {
    //! [testVMHostCreate]
    VMHost *v = _v_new();
    SemTable *sem = v->r->sem;
    // test that the base contexts and defs were created
    spec_is_equal(sem->contexts,_NUM_DEFAULT_CONTEXTS);
    spec_is_equal(_t_children(_t_child(sem->stores[0].definitions,SEM_TYPE_SYMBOL)),NUM_SYS_SYMBOLS-1);
    spec_is_equal(_t_children(_t_child(sem->stores[0].definitions,SEM_TYPE_STRUCTURE)),NUM_SYS_STRUCTURES-1);

    spec_is_str_equal(t2s(v->r->root),"(RECEPTOR_INSTANCE (INSTANCE_OF:SYS_RECEPTOR) (CONTEXT_NUM:0) (PARENT_CONTEXT_NUM:-1) (RECEPTOR_STATE (FLUX (DEFAULT_ASPECT (EXPECTATIONS) (SIGNALS))) (PENDING_SIGNALS) (PENDING_RESPONSES) (RECEPTOR_ELAPSED_TIME:0)))");

    // test the installed receptors scape
    spec_is_sem_equal(v->installed_receptors->key_source,RECEPTOR_IDENTIFIER);
    spec_is_sem_equal(v->installed_receptors->data_source,RECEPTOR_SURFACE);
    _v_free(v);
    //! [testVMHostCreate]
}

/* #define HTTP_SERVER_RECEPTOR_UUID 4321 */
/* /\** */
/*  * generate an http server receptor package */
/*  * */
/*  * @snippet spec/vmhost_spec.h makeTestHTTPServerReceptorPackage */
/*  *\/ */
/* //! [makeTestReceptorPackage] */
/* T *_makeTestHTTPServerReceptorPackage() { */
/*     T *p = _t_new_root(RECEPTOR_PACKAGE); */
/*     T *m = _t_newr(p,MANIFEST); // specification for which manifest variables can be configured */

/*     _t_newi(p,RECEPTOR_IDENTIFIER,HTTP_SERVER_RECEPTOR_UUID); */
/*     T *defs = _t_newr(p,DEFINITIONS); */
/*     _t_add(defs,_t_clone(test_HTTP_structures)); */
/*     _t_add(defs,_t_clone(test_HTTP_symbols)); */

/*     T *procs = _t_newr(defs,PROCESSES); */
/*     // initialization process installs the listener for host registration */
/*     //@todo for now we fake this, but adding it manually as c code in the install example */

/*     _t_newr(defs,SCAPES); // for now we don't have any scapes */

/*     // server must be instantiated in the context of an octet stream input and output carriers */
/*     T *aspects = _t_newr(p,ASPECTS); */
/*     T *a = _t_newr(aspects,ASPECT_DEF); */
/*     _t_newi(a,EXTERNAL_ASPECT,DEFAULT_ASPECT); */
/*     _t_newi(a,CARRIER,OCTET_STREAM); */
/*     _t_newi(a,CARRIER,OCTET_STREAM); */

/*     //@todo internally server speaks to document providers */
/*     a = _t_newr(aspects,ASPECT_DEF); */
/*     _t_newi(a,INTERNAL_ASPECT,DOCUMENT_PROVIDER); */
/*     _t_newi(a,CARRIER,DOCUMENT); */
/*     _t_newi(a,CARRIER,DOCUMENT_PATH); */

/*     //@todo internally server speaks to host registerers */
/*     a = _t_newr(aspects,ASPECT_DEF); */
/*     _t_newi(a,INTERNAL_ASPECT,HOST_REGISTRY); */
/*     _t_newi(a,CARRIER,HOST_REGISTRATION_REQUEST); */
/*     _t_newi(a,CARRIER,HOST_REGISTRATION_RESULT); */

/*     return p; */
/* } */
/* //! [makeTestReceptorPackage] */

/* #define HELLO_WORLD_UUID 12345 */
/* /\** */
/*  * generate a a document server receptor package (which can attach to a an http server) */
/*  * */
/*  * @snippet spec/vmhost_spec.h makeTestHTTPAppReceptorPackage */
/*  *\/ */
/* //! [makeTestHTTPAppReceptorPackage] */
/* T *_makeTestHTTPAppReceptorPackage() { */
/*     T *p = _t_new_root(RECEPTOR_PACKAGE); */
/*     T *m = _t_newr(p,MANIFEST); */
/*     T *mp = _t_newr(m,MANIFEST_PAIR); */
/*     _t_new(mp,MANIFEST_LABEL,"host",5);  // binds to a host value */
/*     _t_newi(mp,MANIFEST_SPEC,HTTP_REQUEST_HOST); */
/*     //    T *me = _t_newr(mp,PATTERN); */

/*     _t_newi(p,RECEPTOR_IDENTIFIER,HELLO_WORLD_UUID); */

/*     T *defs = _t_newr(p,DEFINITIONS); */
/*     _t_add(defs,_t_clone(test_HTTP_structures)); */
/*     _t_add(defs,_t_clone(test_HTTP_symbols)); */
/*     T *procs = _t_newr(defs,PROCESSES); */
/*     // a process that simply reduces to an HTTP_RESPONSE indicating an off-line status */
/*     T *resp = _t_new_root(RESPOND); */
/*     _t_news(resp,CARRIER,HTTP_RESPONSE);*/
/*     T *http_resp = _t_newr(resp,HTTP_RESPONSE); */
/*     _t_new(http_resp,HTTP_RESPONSE_CONTENT_TYPE,"Text/Plain",11); */
/*     _t_new(http_resp,TEST_STR_SYMBOL,"Hello World!",13); */
/*     T *input = _t_new_root(INPUT_SIGNATURES); */
/*     T *output = _t_new_root(OUTPUT_SIGNATURE); */
/*     _d_define_process(procs,resp,"hellow","respond with hello",signature); */

/*     _t_newr(defs,SCAPES); // for now we don't have any scapes */

/*     // an hello world app speaks the document provider protocol which operates on */
/*     // a DOCUMENT_PATH input carrier and a DOCUMENT output carrier */
/*     T *aspects = _t_newr(p,ASPECTS); */
/*     T *a = _t_newr(aspects,ASPECT_DEF); */
/*     _t_newi(a,EXTERNAL_ASPECT,DEFAULT_ASPECT); */
/*     _t_newi(a,CARRIER,DOCUMENT_PATH); */
/*     _t_newi(a,CARRIER,DOCUMENT); */

/*     return p; */
/* } */
/* //! [makeTestHTTPAppReceptorPackage] */

/* void testVMHostLoadReceptorPackage() { */
/*     //! [testVMHostLoadReceptorPackage] */
/*     VMHost *v = _v_new(); */
/*     T *p = _makeTestHTTPAppReceptorPackage(); */

/*     Xaddr x = _v_load_receptor_package(v,p); */
/*     T *p1 = _r_get_instance(v->c,x); */

/*     spec_is_ptr_equal(p,p1); */
/*     _v_free(v); */
/*     //! [testVMHostLoadReceptorPackage] */
/* } */

/* void testVMHostInstallReceptor() { */
/*     //! [testVMHostInstallReceptor] */
/*     VMHost *v = _v_new(); */

/*     T *p = _makeTestHTTPServerReceptorPackage(); */
/*     Xaddr xp = _v_load_receptor_package(v,p); */

/*     Xaddr x = _v_install_r(v,xp,0,"hello world"); */

/*     // installing the receptor should instantiate a receptor from the package with the given bindings and symbol label */
/*     T *r = _r_get_instance(v->r,x); */
/*     spec_is_symbol_equal(v->r,x.symbol,INSTALLED_RECEPTOR); */
/*     spec_is_symbol_equal(v->r,_t_symbol(_t_child(r,1)),_r_get_symbol_by_label(v->r,"hello world")); */

/*     // and the definition labels of the instantiated receptor should all be set up properly */
/*     Receptor *httpr = (Receptor *)_t_surface(_t_child(r,1)); */
/*     spec_is_equal(_r_get_symbol_by_label(httpr,"HTTP_REQUEST"),HTTP_REQUEST); */
/*     spec_is_equal(_r_get_symbol_by_label(httpr,"hellow"),-1); // hard-coded process symbol */

/*     // trying to re-install the receptor should fail */
/*     x = _v_install_r(v,xp,0,"hellow world"); */
/*     spec_is_true(is_null_xaddr(x)); */

/*     // because the receptor's id is in the installed_receptors scape */
/*     T *pack = _r_get_instance(v->c,xp); */
/*     T *id = _t_child(pack,2); */
/*     TreeHash h = _t_hash(v->r->sem,id); */
/*     spec_is_xaddr_equal(v->r,_s_get(v->installed_receptors,h),xp); */

/*     _v_free(v); */
/*     //! [testVMHostInstallReceptor] */
/* } */

/* void testVMHostActivateReceptor() { */
/*     //! [testVMHostActivateReceptor] */
/*     VMHost *v = _v_new(); */

/*     // create and install a stub HTTP server receptor */
/*     T *httpd_rp = _makeTestHTTPServerReceptorPackage(); */
/*     Xaddr httpd_px = _v_load_receptor_package(v,httpd_rp); */
/*     Xaddr httpd_x = _v_install_r(v,httpd_px,0,"http server"); */
/*     T *installed_httpd = _r_get_instance(v->r,httpd_x); */
/*     Receptor *httpd_r = (Receptor *)_t_surface(_t_child(installed_httpd,1)); */

/*     // create and install an app bound to a HOST */
/*     T *p = _makeTestHTTPAppReceptorPackage(); */
/*     Xaddr xp = _v_load_receptor_package(v,p); */

/*     T *b = _t_new_root(BINDINGS); */
/*     T *pair = _t_newr(b,BINDING_PAIR); */
/*     char *host = "helloworld.com"; */

/*     _t_new(pair,MANIFEST_LABEL,"host",5); */
/*     _t_new(pair,HTTP_REQUEST_HOST,host,strlen(host)); */

/*     Xaddr x = _v_install_r(v,xp,b,"hello world app"); */
/*     T *installed_hellow = _r_get_instance(v->r,x); */
/*     Receptor *hello_r = (Receptor *)_t_surface(_t_child(installed_hellow,1)); */

/*     // add a listener that matches on any request with "Host: helloworld.org" */
/*     // /HTTP_REQUEST/.*,HTTP_REQUEST_HOST=helloworld.org */
/*     T *act = _t_newp(0,ACTION,-1); */
/*     T *expect = _t_new_root(PATTERN); */
/*     T *req = _t_newi(expect,SEMTREX_SYMBOL_LITERAL,HTTP_REQUEST); */
/*     T *ss = _t_newi(req,SEMTREX_SEQUENCE,0); */
/*     T *sss = _t_newi(ss,SEMTREX_ZERO_OR_MORE,0); */
/*     _t_newi(sss,SEMTREX_SYMBOL_ANY,0); */

/*     int size = sizeof(Svalue)+strlen(host)+1; */
/*     Svalue *sv = malloc(size); */
/*     sv->symbol = HTTP_REQUEST_HOST; */
/*     sv->flags = 0; */
/*     sv->count = 1; */
/*     sv->length = strlen(host); */
/*     strcpy((char *)&sv->value,host); */
/*     _t_new(ss,SEMTREX_VALUE_LITERAL,sv,size); */

/*     char buf[2000]; */
/*     // just check that we got our semtrex right */
/*     spec_is_str_equal(_dump_semtrex(G_sem,req,buf),"/(HTTP_REQUEST/.*,HTTP_REQUEST_HOST=helloworld.com)"); */
/*     free(sv); */

/*     _r_add_expectation(hello_r,DEFAULT_ASPECT,HTTP_REQUEST,expect,act,0,0); */

/*     // build up an HTTP_REQUEST tree that corresponds with a simple get for host helloworld.com */
/*     T *s = _t_new_root(HTTP_REQUEST); */
/*     T *s_version = _t_newr(s,HTTP_REQUEST_VERSION); */
/*     _t_newi(s_version,VERSION_MAJOR,1); */
/*     _t_newi(s_version,VERSION_MINOR,1); */
/*     T *s_method = _t_new(s,HTTP_REQUEST_METHOD,"GET",4); */
/*     T *s_path = _t_newr(s,HTTP_REQUEST_PATH); */
/*     T *s_host = _t_new(s,HTTP_REQUEST_HOST,"helloworld.com",15); */

/*     // put it on the flux to simulate that it has just been parsed out of an octet stream from a TCP/IP receptor */
/*     /// @todo */
/*     //int path[] = {1,2,TREE_PATH_TERMINATOR}; */
/*     //T *fs = _t_get(httpd_r->flux,path); */



/*     //simulate that an HTTP_Request signal from the http_server receptor has been sent */


/*     Xaddr xs = _v_send(v,x,httpd_x,DEFAULT_ASPECT,s); */

/*     // confirm that the signal has not been delivered and is still in pending list (because receptors aren't active) */
/*     spec_is_equal(_t_children(v->pending_signals),1); */

/*     // activate the two receptors */
/*     _v_activate(v,httpd_x); */
/*     _v_activate(v,x); */

Receptor *_makeAliveProtocolReceptor(VMHost *v);

void testVMHostActivateReceptor()  {
    //! [testVMHostActivateReceptor]
    VMHost *v = _v_new();
    SemTable *sem = v->r->sem;

    Receptor *server =  _r_new(sem,TEST_RECEPTOR);
    _o_express_role(server,ALIVE,SERVER,DEFAULT_ASPECT,NULL);

    Receptor *client =  _r_new(sem,TEST_RECEPTOR);

    T *noop = _t_new_root(NOOP);
    _t_newi(noop,TEST_INT_SYMBOL,314);
    Process proc = _r_define_process(client,noop,"do nothing","long desc...",NULL);

    T *binding = _t_new_root(PROTOCOL_BINDINGS);
    T *res = _t_newr(binding,RESOLUTION);
    _t_news(res,GOAL,HANDLER);
    _t_news(res,ACTUAL_PROCESS,proc);
    _o_express_role(client,ALIVE,CLIENT,DEFAULT_ASPECT,binding);
    _t_free(binding);

    Xaddr sx = _v_new_receptor(v,v->r,TEST_RECEPTOR,server);
    Xaddr cx = _v_new_receptor(v,v->r,TEST_RECEPTOR,client);
    _v_activate(v,sx);
    _v_activate(v,cx);

    // confirm that the activate list has new children
    spec_is_equal(v->active_receptor_count,2);

    // and that they are the same as the installed receptors
    Receptor *ar;
    ar = v->active_receptors[0].r;
    spec_is_ptr_equal(ar,server);
    ar = v->active_receptors[1].r;
    spec_is_ptr_equal(ar,client);

    _v_send(v,client->addr,server->addr,DEFAULT_ASPECT,ALIVE,_t_newi(0,PING,0));

    spec_is_str_equal(_td(client,v->r->pending_signals),"(PENDING_SIGNALS (SIGNAL (ENVELOPE (FROM_ADDRESS (CONTEXT_NUM:3)) (TO_ADDRESS (CONTEXT_NUM:2)) (ASPECT_IDENT:DEFAULT_ASPECT) (CARRIER:ALIVE) (SIGNAL_UUID)) (BODY:{(PING)})))");

    // simulate round-robin processing of signals
    //debug_enable(D_SIGNALS);
    _v_deliver_signals(v,v->r);
    _p_reduceq(server->q);
    _v_deliver_signals(v,server);
    //   _p_reduceq(client->q);

    // now confirm that the signal was sent,
    // first that the pending signals list is empty
    spec_is_equal(_t_children(v->r->pending_signals),0);

    // @todo delivery of response doesn't work here because _v_send is broken and there's no process
    // q or anything for the delivery to unblock.  This is a broken spec that we need to fix!!
    //    debug_disable(D_SIGNALS);

    _v_free(v);
    //! [testVMHostActivateReceptor]
}


void testVMHostShell() {

    // set up the vmhost
    G_vm = _v_new();
    SemTable *gsem = G_sem;
    G_sem = G_vm->sem;

    _v_instantiate_builtins(G_vm);

    // allocate c input out streams to mimic stdin and stdout
    FILE *input,*output;
    char commands[] = "receptors\ntime\n";
    input = fmemopen(commands, strlen(commands), "r");
    char *output_data = NULL;
    size_t size;
    output = open_memstream(&output_data,&size);

    Stream *output_stream, *input_stream;
    Receptor *i_r,*o_r;

    makeShell(G_vm,input,output,&i_r,&o_r,&input_stream,&output_stream);

    //   debug_enable(D_STREAM+D_SIGNALS+D_TREE+D_PROTOCOL);
    _v_start_vmhost(G_vm);
    sleep(1);
    debug_disable(D_STREAM+D_SIGNALS+D_TREE+D_PROTOCOL);

    // confirm that the completed processes get cleaned up in the course of the vmhosts processing
    spec_is_ptr_equal(i_r->q->completed,NULL);
    spec_is_ptr_equal(o_r->q->completed,NULL);

    // kill the string after elapsed time because it's system dependent
    T *e = _t_child(_t_child(i_r->root,ReceptorInstanceStateIdx),ReceptorElapsedTimeIdx);
    spec_is_true(*(int *)_t_surface(e)>0);

    spec_is_true(output_data != 0); // protect against seg-faults when nothing was written to the stream...
    if (output_data != 0) {
        output_data[121] =0;  // clip the tick so it work regardless of the time
        spec_is_str_equal(output_data,"COMPOSITORY:0 DEV_COMPOSITORY:1 TEST_RECEPTOR:2 CLOCK_RECEPTOR:3 shell:4 STREAM_EDGE:5 STREAM_EDGE:6 \n(TICK (TODAY (YEAR:");}
    __r_kill(G_vm->r);

    _v_join_thread(&G_vm->clock_thread);
    _v_join_thread(&G_vm->vm_thread);

    // free the memory used by the SYS_RECEPTOR
    _v_free(G_vm);
    G_vm = NULL;
    G_sem = gsem;

    _st_free(input_stream);
    _st_free(output_stream);
    free(output_data);
}

void testVMHostSerialize() {
    G_vm = _v_new();
    _v_instantiate_builtins(G_vm);

    spec_is_str_equal(t2s(G_vm->r->root),"(SYS_RECEPTOR (DEFINITIONS (STRUCTURES) (SYMBOLS) (PROCESSES) (PROTOCOLS) (SCAPES)) (FLUX (DEFAULT_ASPECT (EXPECTATIONS) (SIGNALS))) (RECEPTOR_STATE) (PENDING_SIGNALS) (PENDING_RESPONSES))");

    Receptor *clock = G_vm->active_receptors[0].r;
    //   _testReceptorClockAddExpectation(clock);

    void *surface;
    size_t length;
    _r_serialize(G_vm->r,&surface,&length);
    Receptor *r = _r_unserialize(G_vm->r->sem,surface);
    spec_is_str_equal(t2s(r->root),"(SYS_RECEPTOR (DEFINITIONS (STRUCTURES) (SYMBOLS) (PROCESSES) (PROTOCOLS) (SCAPES)) (FLUX (DEFAULT_ASPECT (EXPECTATIONS) (SIGNALS))) (RECEPTOR_STATE) (PENDING_SIGNALS) (PENDING_RESPONSES))");

    __r_kill(clock);
    _v_join_thread(&G_vm->clock_thread);

    // __r_dump_instances(r);
    _r_free(r);
    free(surface);
    _v_free(G_vm);
    G_vm=NULL;
}
void testVMHost() {
    testVMHostCreate();
    //testVMHostLoadReceptorPackage();
    //testVMHostInstallReceptor();
    //testVMHostActivateReceptor();
    testVMHostShell();
    //   testVMHostSerialize();
}
