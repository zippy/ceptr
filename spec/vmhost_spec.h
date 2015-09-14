/**
 * @file vmhost_spec.h
 * @copyright Copyright (C) 2013-2015, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 * @ingroup tests
 */

#include "../src/ceptr.h"
#include "../src/vmhost.h"
#include "http_example.h"

void testVMHostCreate() {
    //! [testVMHostCreate]
    VMHost *v = _v_new();

    // test the structure of the VM_HOST receptor
    spec_is_str_equal(t2s(v->r->root),"(VM_HOST_RECEPTOR (DEFINITIONS (STRUCTURES) (SYMBOLS) (PROCESSES) (PROTOCOLS) (SCAPES)) (ASPECTS) (FLUX (ASPECT:1 (LISTENERS) (SIGNALS))) (RECEPTOR_STATE) (ACTIVE_RECEPTORS) (PENDING_SIGNALS) (COMPOSITORY:{(COMPOSITORY (DEFINITIONS (STRUCTURES) (SYMBOLS) (PROCESSES) (PROTOCOLS) (SCAPES)) (ASPECTS) (FLUX (ASPECT:1 (LISTENERS) (SIGNALS))) (RECEPTOR_STATE))}))");

    // test the installed receptors scape
    spec_is_sem_equal(v->installed_receptors->key_source,RECEPTOR_IDENTIFIER);
    spec_is_sem_equal(v->installed_receptors->data_source,RECEPTOR);
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
/*     //    T *me = _t_newr(mp,EXPECTATION); */

/*     _t_newi(p,RECEPTOR_IDENTIFIER,HELLO_WORLD_UUID); */

/*     T *defs = _t_newr(p,DEFINITIONS); */
/*     _t_add(defs,_t_clone(test_HTTP_structures)); */
/*     _t_add(defs,_t_clone(test_HTTP_symbols)); */
/*     T *procs = _t_newr(defs,PROCESSES); */
/*     // a process that simply reduces to an HTTP_RESPONSE indicating an off-line status */
/*     T *resp = _t_new_root(RESPOND); */
/*     T *http_resp = _t_newr(resp,HTTP_RESPONSE); */
/*     _t_new(http_resp,HTTP_RESPONSE_CONTENT_TYPE,"Text/Plain",11); */
/*     _t_new(http_resp,TEST_STR_SYMBOL,"Hello World!",13); */
/*     T *input = _t_new_root(INPUT); */
/*     T *output = _t_new_root(OUTPUT_SIGNATURE); */
/*     _d_code_process(procs,resp,"hellow","respond with hello",input,output); */

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
/*     TreeHash h = _t_hash(v->r->defs.symbols,v->r->defs.structures,id); */
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
/*     T *expect = _t_new_root(EXPECTATION); */
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
/*     spec_is_str_equal(_dump_semtrex(test_HTTP_defs,req,buf),"/(HTTP_REQUEST/.*,HTTP_REQUEST_HOST=helloworld.com)"); */
/*     free(sv); */

/*     _r_add_listener(hello_r,DEFAULT_ASPECT,HTTP_REQUEST,expect,act); */

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


void testVMHostActivateReceptor() {
    //! [testVMHostActivateReceptor]
    VMHost *v = _v_new();

    Symbol ping;
    Receptor *server = _makePingProtocolReceptor(&ping);
    Symbol alive_server_seq = _r_get_symbol_by_label(server,"alive_server");
    _r_express_protocol(server,1,alive_server_seq,DEFAULT_ASPECT,NULL);

    Receptor *client = _makePingProtocolReceptor(&ping);
    Symbol alive_client_seq = _r_get_symbol_by_label(client,"alive_client");


    T *noop = _t_new_root(NOOP);
    _t_newi(noop,TEST_INT_SYMBOL,314);
    T *input = _t_new_root(INPUT);
    T *output = _t_new_root(OUTPUT_SIGNATURE);
    Process proc = _r_code_process(client,noop,"do nothing","long desc...",input,output);
    T *handler = _t_newp(0,ACTION,proc);

    _r_express_protocol(client,1,alive_client_seq,DEFAULT_ASPECT,handler);

    Symbol ss = _r_declare_symbol(v->r,RECEPTOR,"alive server");
    Symbol cs = _r_declare_symbol(v->r,RECEPTOR,"alive client");

    Xaddr sx = _v_new_receptor(v,ss,server);
    Xaddr cx = _v_new_receptor(v,cs,client);
    __v_activate(v,server);
    __v_activate(v,client);

    // confirm that the activate list has new children
    spec_is_equal(_t_children(v->active_receptors),2);

    // and that they are the same as the installed receptors
    T *ar;
    ar = _t_child(v->active_receptors,1);
    spec_is_ptr_equal(ar,server->root);
    ar = _t_child(v->active_receptors,2);
    spec_is_ptr_equal(ar,client->root);

    _v_send(v,cx,sx,DEFAULT_ASPECT,_t_newi(0,ping,0));

    spec_is_str_equal(_td(client,v->pending_signals),"(PENDING_SIGNALS (SIGNAL (ENVELOPE (RECEPTOR_XADDR:INSTALLED_RECEPTOR.2) (RECEPTOR_XADDR:INSTALLED_RECEPTOR.1) (ASPECT:1)) (BODY:{(ping_message:0)})))");
    // simulate round-robin processing of signals
    _v_deliver_signals(v);
    if (server->q) {
        _p_reduceq(server->q);
        if (server->q->completed) {
            T *result = server->q->completed->context->run_tree;
            T *signals = _t_child(result,_t_children(result));
            // if the results added signals then send em!
            if (semeq(SIGNALS,_t_symbol(signals))) {
                _v_send_signals(v,signals);
            }
        }
    }
    _v_deliver_signals(v);
    if (client->q) {
        _p_reduceq(client->q);
        if (client->q->completed) {
            T *result = client->q->completed->context->run_tree;
            T *signals = _t_child(result,_t_children(result));
            // if the results added signals then send em!
            if (semeq(SIGNALS,_t_symbol(signals))) {
                _v_send_signals(v,signals);
            }
        }
    }



    // now confirm that the signal was sent,
    // first that the pending signals list is empty
    spec_is_equal(_t_children(v->pending_signals),0);

    // and that the client now has the arrived response ping signal, plus the reduced action run-tree
    T *t = __r_get_signals(client,DEFAULT_ASPECT);
    spec_is_str_equal(_td(client,t),"(SIGNALS (SIGNAL (ENVELOPE (RECEPTOR_XADDR:INSTALLED_RECEPTOR.1) (RECEPTOR_XADDR:INSTALLED_RECEPTOR.2) (ASPECT:1)) (BODY:{(alive_message:1)}) (RUN_TREE (TEST_INT_SYMBOL:314) (PARAMS))))");

    _v_free(v);
    //! [testVMHostActivateReceptor]
}

void testVMHostShell() {

    // set up the vmhost
    G_vm = _v_new();
    _v_instantiate_builtins(G_vm);

    // create the shell receptor
    Symbol shell = _r_declare_symbol(G_vm->r,RECEPTOR,"shell");
    Receptor *r = _r_new(shell);

    Symbol verb = _r_declare_symbol(r,CSTRING,"verb");
    Structure command = _r_define_structure(r,"command",1,verb); // need the optional parameters part here
    Symbol shell_command = _r_declare_symbol(r,command,"shell command");

    Xaddr shellx = _v_new_receptor(G_vm,shell,r);
    _v_activate(G_vm,shellx);

    // create stdin/out receptors

    // allocate c input out streams to mimic stdin and stdout
    FILE *input,*output;
    char commands[] = "time\nreceptors\ntime\n";
    input = fmemopen(commands, strlen (commands), "r");
    Stream *input_stream = _st_new_unix_stream(input);
    char *output_data = NULL;
    size_t size;
    output = open_memstream(&output_data,&size);
    Stream *output_stream = _st_new_unix_stream(output);

    Symbol std_in = _r_declare_symbol(G_vm->r,RECEPTOR,"std_in");
    Receptor *i_r = _r_makeStreamReaderReceptor(std_in,TEST_STREAM_SYMBOL,input_stream,shellx);
    Xaddr ix = _v_new_receptor(G_vm,std_in,i_r);
    _v_activate(G_vm,ix);

    Symbol std_out = _r_declare_symbol(G_vm->r,RECEPTOR,"std_out");
    Receptor *o_r = _r_makeStreamWriterReceptor(std_out,TEST_STREAM_SYMBOL,output_stream);
    Xaddr ox = _v_new_receptor(G_vm,std_out,o_r);
    _v_activate(G_vm,ox);

    // create expectations for commands
    // (expect (on std_in LINE) action (send self (shell_command parsed from LINE))
    T *expect = _t_new_root(EXPECTATION);
    T *s = _t_news(expect,SEMTREX_GROUP,LINE);
    _sl(s,LINE);
    T *p = _t_new_root(SEND);
    Xaddr to = {RECEPTOR_XADDR,0};  // self
    _t_new(p,RECEPTOR_XADDR,&to,sizeof(to));
    T *x = _t_newr(p,shell_command);
    int pt1[] = {2,1,TREE_PATH_TERMINATOR};
    _t_new(x,PARAM_REF,pt1,sizeof(int)*4);
    _t_newi(p,BOOLEAN,1); // indicate async send

    Process proc = _r_code_process(r,p,"send self command","long desc...",NULL,NULL);
    T *act = _t_newp(0,ACTION,proc);
    T* params = _t_new_root(PARAMS);
    _t_news(params,INTERPOLATE_SYMBOL,LINE);
    _r_add_listener(r,DEFAULT_ASPECT,LINE,expect,params,act);

    // (expect (on flux SHELL_COMMAND:time) action(send std_out (convert_to_lines (listen to clock)))
    expect = _t_new_root(EXPECTATION);
    s = _t_news(expect,SEMTREX_GROUP,shell_command);

    T *cm = _sl(s,shell_command);
    T *vl =  _t_newr(cm,SEMTREX_VALUE_LITERAL);
    T *vls = _t_newr(vl,SEMTREX_VALUE_SET);
    _t_new_str(vls,LINE,"time"); // this should be verb, the action doesn't convert the LINE to a VERB yet

    p = _t_new_root(SEND);
    _t_new(p,RECEPTOR_XADDR,&ox,sizeof(ox));
    x = _t_new_str(p,LINE,"placeholder for time");
    //    int pt1[] = {2,1,TREE_PATH_TERMINATOR};
    //    _t_new(x,PARAM_REF,pt1,sizeof(int)*4);
    proc = _r_code_process(r,p,"send time to std_out","long desc...",NULL,NULL);
    act = _t_newp(0,ACTION,proc);
    params = _t_new_root(PARAMS);
    _t_news(params,INTERPOLATE_SYMBOL,shell_command);
    _r_add_listener(r,DEFAULT_ASPECT,LINE,expect,params,act);

    // (expect (on flux SHELL_COMMAND:receptor) action (send std_out (convert_to_lines (send vmhost (get receptor list from vmhost)))))

    debug_enable(D_STREAM+D_SIGNALS);
    //    spec_is_str_equal(_td(o_r,o_r->flux),"or flux");
    _v_start_vmhost(G_vm);
    sleep(1);
    //    spec_is_str_equal(_td(o_r,o_r->flux),"or flux");
    //    spec_is_str_equal(_td(r,r->flux),"shell flux");
    debug_disable(D_STREAM);

    // confirm that the completed processes get cleaned up in the course of the vmhosts processing
    spec_is_ptr_equal(i_r->q->completed,NULL);
    spec_is_ptr_equal(o_r->q->completed,NULL);

    // kill the string after elapsed time because it's system dependent
    char *rs = t2s(_t_child(i_r->root,4));
    rs[39]=0;
    spec_is_str_equal(rs,"(RECEPTOR_STATE (RECEPTOR_ELAPSED_TIME:");

    spec_is_true(output_data != 0); // protect against seg-faults when nothing was written to the stream...
    if (output_data != 0) {spec_is_str_equal(output_data,"some time representation x 2");}
    __r_kill(G_vm->r);

    _v_join_thread(&G_vm->clock_thread);
    _v_join_thread(&G_vm->vm_thread);
    // free the memory used by the VM_HOST
    _v_free(G_vm);
    G_vm = NULL;

    _st_free(input_stream);
    _st_free(output_stream);
    free(output_data);
}

void testVMHost() {
    _setup_HTTPDefs();
    testVMHostCreate();
    //    testVMHostLoadReceptorPackage();
    //    testVMHostInstallReceptor();
    testVMHostActivateReceptor();
    testVMHostShell();
    _cleanup_HTTPDefs();
}
