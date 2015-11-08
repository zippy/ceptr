/**
 * @file receptor_spec.h
 * @copyright Copyright (C) 2013-2015, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 * @ingroup tests
 */

#include "../src/ceptr.h"
#include "../src/receptor.h"
#include "../src/def.h"
#include "../src/accumulator.h"
#include "../src/vmhost.h"
#include "http_example.h"
#include <unistd.h>

void testReceptorCreate() {
    //! [testReceptorCreate]
    Receptor *r;
    r = _r_new(TEST_RECEPTOR_SYMBOL);

    spec_is_symbol_equal(r,_t_symbol(r->root),TEST_RECEPTOR_SYMBOL);

    T *t,*d;

    // test that the symbols, structures & process trees are set up correctly
    d = _t_child(r->root,1);
    t = _t_child(d,1);
    spec_is_symbol_equal(r,_t_symbol(r->defs.structures),STRUCTURES);
    spec_is_ptr_equal(t,r->defs.structures);
    t = _t_child(d,2);
    spec_is_symbol_equal(r,_t_symbol(r->defs.symbols),SYMBOLS);
    spec_is_ptr_equal(t,r->defs.symbols);
    t = _t_child(d,3);
    spec_is_symbol_equal(r,_t_symbol(r->defs.processes),PROCESSES);
    spec_is_ptr_equal(t,r->defs.processes);
    t = _t_child(d,4);
    spec_is_symbol_equal(r,_t_symbol(r->defs.protocols),PROTOCOLS);
    spec_is_ptr_equal(t,r->defs.protocols);
    t = _t_child(d,5);
    spec_is_symbol_equal(r,_t_symbol(r->defs.scapes),SCAPES);
    spec_is_ptr_equal(t,r->defs.scapes);

    // test that listeners and signals are set up correctly on the default aspect
    t = __r_get_listeners(r,DEFAULT_ASPECT);
    spec_is_symbol_equal(r,_t_symbol(t),LISTENERS);
    t = __r_get_signals(r,DEFAULT_ASPECT);
    spec_is_symbol_equal(r,_t_symbol(t),SIGNALS);

    // test that the aspects specs are set up correctly
    t = _t_child(r->root,2);
    spec_is_symbol_equal(r,_t_symbol(t),ASPECTS);

    // test that the flux is set up correctly
    t = _t_child(r->root,3);
    spec_is_symbol_equal(r,_t_symbol(r->flux),FLUX);
    spec_is_ptr_equal(t,r->flux);
    t = _t_child(r->flux,1);
    spec_is_symbol_equal(r,_t_symbol(t),ASPECT);
    spec_is_equal(*(int *)_t_surface(t),DEFAULT_ASPECT);

    spec_is_str_equal(t2s(r->root),"(TEST_RECEPTOR_SYMBOL (DEFINITIONS (STRUCTURES) (SYMBOLS) (PROCESSES) (PROTOCOLS) (SCAPES)) (ASPECTS) (FLUX (ASPECT:1 (LISTENERS) (SIGNALS))) (RECEPTOR_STATE) (PENDING_SIGNALS) (PENDING_RESPONSES))");

    _r_free(r);
    //! [testReceptorCreate]
}

void testReceptorAddListener() {
    Receptor *r;
    r = _r_new(TEST_RECEPTOR_SYMBOL);

    Symbol dummy = {0,0,0};
    // test that you can add a listener to a receptor's aspect
    T *s = _t_new_root(EXPECTATION);
    _sl(s,dummy);
    T *a = _t_news(0,ACTION,NULL_PROCESS);
    T *p = _t_new_root(PARAMS);  // empty construct params list
    _r_add_listener(r,DEFAULT_ASPECT,TEST_INT_SYMBOL,s,p,a);

    T *l = _t_child(__r_get_listeners(r,DEFAULT_ASPECT),1);      // listener should have been added as first child of listeners
    spec_is_symbol_equal(r,_t_symbol(l),LISTENER);
    spec_is_sem_equal(*(Symbol *)_t_surface(l),TEST_INT_SYMBOL); // carrier should be TEST_INT_SYMBOL
    spec_is_ptr_equal(_t_child(l,1),s);       // our expectation semtrex should be first child of the listener
    spec_is_ptr_equal(_t_child(l,2),p);       // our params constructor should be the second child of the listener
    spec_is_ptr_equal(_t_child(l,3),a);       // our action code tree should be the third child of the listener

    _r_free(r);
}

void testReceptorSignal() {
    Receptor *r = _r_new(TEST_RECEPTOR_SYMBOL);
    T *signal_contents = _t_newi(0,TEST_INT_SYMBOL,314);
    ReceptorAddress f = 3; // DUMMY ADDR
    ReceptorAddress t = 4; // DUMMY ADDR

    T *s = __r_make_signal(f,t,DEFAULT_ASPECT,signal_contents);

    spec_is_symbol_equal(r,_t_symbol(s),SIGNAL);

    T *envelope = _t_child(s,SignalEnvelopeIdx);
    spec_is_str_equal(t2s(envelope),"(ENVELOPE (RECEPTOR_ADDRESS:3) (RECEPTOR_ADDRESS:4) (ASPECT:1) (CARRIER:TEST_INT_SYMBOL) (SIGNAL_UUID))");
    T *body = _t_child(s,SignalBodyIdx);
    spec_is_str_equal(t2s(body),"(BODY:{(TEST_INT_SYMBOL:314)})");
    T *contents = (T*)_t_surface(body);
    spec_is_ptr_equal(signal_contents,contents);

    _t_free(s);
    _r_free(r);
}

void testReceptorSignalDeliver() {
    Receptor *r = _r_new(TEST_RECEPTOR_SYMBOL);
    T *signal_contents = _t_newi(0,TEST_INT_SYMBOL,314);
    ReceptorAddress f = 3; // DUMMY ADDR
    ReceptorAddress t = 4; // DUMMY ADDR

    // a new signal should simply be placed on the flux when delivered
    T *s = __r_make_signal(f,t,DEFAULT_ASPECT,signal_contents);
    spec_is_equal(_r_deliver(r,s),noDeliveryErr);

    T *signals = __r_get_signals(r,DEFAULT_ASPECT);
    spec_is_str_equal(_td(r,signals),"(SIGNALS (SIGNAL (ENVELOPE (RECEPTOR_ADDRESS:3) (RECEPTOR_ADDRESS:4) (ASPECT:1) (CARRIER:TEST_INT_SYMBOL) (SIGNAL_UUID)) (BODY:{(TEST_INT_SYMBOL:314)})))");
    _r_free(r);
}

void testReceptorResponseDeliver() {
    Receptor *r = _r_new(TEST_RECEPTOR_SYMBOL);

    // set up receptor to have sent and signal and blocked waiting for the response
    T *t = _t_new_root(RUN_TREE);
    T *p = _t_new_root(NOOP);
    T *send = _t_newr(p,SEND);
    _t_newi(send,RECEPTOR_ADDRESS,4);
    _t_newi(send,TEST_INT_SYMBOL,98789);
    _t_news(send,RESPONSE_CARRIER,TEST_STR_SYMBOL);
    //    _t_newi(send,BOOLEAN,1); // mark async

    T *c = _t_rclone(p);
    _t_free(p);
    _t_add(t,c);
    _p_addrt2q(r->q,t);
    _p_reduceq(r->q);

    // the run tree should now be blocked with the sending signal uuid in the place
    // where the response value should be filled and the pending responses list should
    // have the UUID and the code path in it
    T *rt = r->q->blocked->context->run_tree;
    spec_is_str_equal(_td(r,rt),"(RUN_TREE (process:NOOP (SIGNAL_UUID)))");
    spec_is_str_equal(_td(r,r->pending_responses),"(PENDING_RESPONSES (PENDING_RESPONSE (SIGNAL_UUID) (CARRIER:TEST_STR_SYMBOL) (PROCESS_IDENT:1) (RESPONSE_CODE_PATH:/1/1)))");

    // create a response signals
    ReceptorAddress from = 4; // DUMMY ADDR
    ReceptorAddress to = 0; // DUMMY ADDR
    T *s = __r_make_signal(from,to,DEFAULT_ASPECT,_t_new_str(0,TEST_STR_SYMBOL,"foo"));

    // add the response uuid to the envelope
    T *response_id = _t_clone(_t_child(_t_child(rt,1),1));
    _t_add(_t_child(s,SignalEnvelopeIdx),response_id);
    //    debug_enable(D_SIGNALS);
    spec_is_equal(_r_deliver(r,s),noDeliveryErr);
    //    debug_disable(D_SIGNALS);

    // block list should now be empty
    spec_is_ptr_equal(r->q->blocked,NULL);

    // and the run tree should now be active and have the response contents value
    // in place for reduction.
    rt = r->q->active->context->run_tree;
    spec_is_str_equal(_td(r,rt),"(RUN_TREE (process:NOOP (TEST_STR_SYMBOL:foo)))");

    // and the pending_responses list should be cleared too
    spec_is_str_equal(_td(r,r->pending_responses),"(PENDING_RESPONSES)");

    _r_free(r);
}

void testReceptorAction() {
    //! [testReceptorAction]
    Receptor *r = _r_new(TEST_RECEPTOR_SYMBOL);

    // The signal is an HTTP request
    T *signal_contents = _makeTestHTTPRequestTree(); // GET /groups/5/users.json?sort_by=last_name?page=2 HTTP/1.0

    ReceptorAddress f = 3; // DUMMY ADDR
    ReceptorAddress t = 4; // DUMMY ADDR

    T *signal = __r_make_signal(f,t,DEFAULT_ASPECT,signal_contents);

    // our expectation should match on the first path segment
    // /HTTP_REQUEST/.,.,HTTP_REQUEST_PATH/HTTP_REQUEST_PATH_SEGMENTS/<HTTP_REQUEST_PATH_SEGMENT:HTTP_REQUEST_PATH_SEGMENT>
    T *expect = _t_new_root(EXPECTATION);
    char *stx = "/HTTP_REQUEST/(.,.,HTTP_REQUEST_PATH/HTTP_REQUEST_PATH_SEGMENTS/<HTTP_REQUEST_PATH_SEGMENT:HTTP_REQUEST_PATH_SEGMENT>)";
    T *req = parseSemtrex(&test_HTTP_defs,stx);
    _t_add(expect,req);
/*    T *req = _t_news(expect,SEMTREX_SYMBOL_LITERAL,HTTP_REQUEST);
    T *seq = _t_newr(req,SEMTREX_SEQUENCE);
    _t_newr(seq,SEMTREX_SYMBOL_ANY);  // skips the Version
    _t_newr(seq,SEMTREX_SYMBOL_ANY);  // skips over the Method
    T *path = _t_news(seq,SEMTREX_SYMBOL_LITERAL,HTTP_REQUEST_PATH);
    T *segs = _t_news(path,SEMTREX_SYMBOL_LITERAL,HTTP_REQUEST_PATH_SEGMENTS);
    T *g = _t_news(segs,SEMTREX_GROUP,HTTP_REQUEST_PATH_SEGMENT);
    _t_news(g,SEMTREX_SYMBOL_LITERAL,HTTP_REQUEST_PATH_SEGMENT);
*/

    // gotta load the defs into the receptor for printing things out to work right.
    r->defs.symbols = _t_clone(test_HTTP_defs.symbols);
    r->defs.structures = _t_clone(test_HTTP_defs.structures);

    T *result;
    int matched;
    // make sure our expectation semtrex actually matches the signal
    spec_is_true(_t_matchr(req,signal_contents,&result));
    T *m = _t_get_match(result,HTTP_REQUEST_PATH_SEGMENT);

    spec_is_str_equal(_td(r,m),"(SEMTREX_MATCH:1 (SEMTREX_MATCH_SYMBOL:HTTP_REQUEST_PATH_SEGMENT) (SEMTREX_MATCH_PATH:/3/1/1) (SEMTREX_MATCH_SIBLINGS_COUNT:1))");
    if (result) {
        _t_free(result);
    }

    T *params;
    Process p;
    _makeTestHTTPResponseProcess(r,&params,&p);
    T *act = _t_newp(0,ACTION,p);

    _r_add_listener(r,DEFAULT_ASPECT,HTTP_REQUEST,expect,params,act);

    Error err = _r_deliver(r,signal);
    spec_is_equal(err,noDeliveryErr);

    // signal and run_tree should be added and ready on the process queue
    spec_is_equal(r->q->contexts_count,1);
    spec_is_str_equal(_td(r,__r_get_signals(r,DEFAULT_ASPECT)),
                      "(SIGNALS (SIGNAL (ENVELOPE (RECEPTOR_ADDRESS:3) (RECEPTOR_ADDRESS:4) (ASPECT:1) (CARRIER:HTTP_REQUEST) (SIGNAL_UUID)) (BODY:{(HTTP_REQUEST (HTTP_REQUEST_VERSION (VERSION_MAJOR:1) (VERSION_MINOR:0)) (HTTP_REQUEST_METHOD:GET) (HTTP_REQUEST_PATH (HTTP_REQUEST_PATH_SEGMENTS (HTTP_REQUEST_PATH_SEGMENT:groups) (HTTP_REQUEST_PATH_SEGMENT:5)) (HTTP_REQUEST_PATH_FILE (FILE_NAME:users) (FILE_EXTENSION:json)) (HTTP_REQUEST_PATH_QUERY (HTTP_REQUEST_PATH_QUERY_PARAMS (HTTP_REQUEST_PATH_QUERY_PARAM (PARAM_KEY:sort_by) (PARAM_VALUE:last_name)) (HTTP_REQUEST_PATH_QUERY_PARAM (PARAM_KEY:page) (PARAM_VALUE:2))))))}) (RUN_TREE (process:RESPOND (PARAM_REF:/2/1)) (PARAMS (HTTP_RESPONSE (HTTP_RESPONSE_CONTENT_TYPE:CeptrSymbol/HTTP_REQUEST_PATH_SEGMENT) (HTTP_REQUEST_PATH_SEGMENT:groups))))))"
                      );

    // manually run the process queue
    _p_reduceq(r->q);

    // should add a pending signal to be sent with the matched PATH_SEGMENT returned as the response signal body
    spec_is_str_equal(_td(r,r->pending_signals),"(PENDING_SIGNALS (SIGNAL (ENVELOPE (RECEPTOR_ADDRESS:4) (RECEPTOR_ADDRESS:3) (ASPECT:1) (CARRIER:HTTP_RESPONSE) (SIGNAL_UUID) (SIGNAL_UUID)) (BODY:{(HTTP_RESPONSE (HTTP_RESPONSE_CONTENT_TYPE:CeptrSymbol/HTTP_REQUEST_PATH_SEGMENT) (HTTP_REQUEST_PATH_SEGMENT:groups))})))");

    result = _t_child(r->q->completed->context->run_tree,1);
    spec_is_str_equal(_td(r,result),"(SIGNAL_UUID)");

    _t_free(r->defs.symbols); // normally these would be freeed by the r_free, but we hand loaded them...
    _t_free(r->defs.structures);
    _r_free(r);
    //! [testReceptorAction]
}

void testReceptorDef() {
    Receptor *r = _r_new(TEST_RECEPTOR_SYMBOL);

    Symbol lat = _r_declare_symbol(r,FLOAT,"latitude");
    Symbol lon = _r_declare_symbol(r,FLOAT,"longitude");
    T *def;

    spec_is_structure_equal(r,__r_get_symbol_structure(r,lat),FLOAT);

    spec_is_str_equal((char *)_t_surface(_t_child(def = _t_child(r->defs.symbols,lat.id),1)),"latitude");
    spec_is_str_equal((char *)_t_surface(_t_child(_t_child(r->defs.symbols,lon.id),1)),"longitude");

    int *path = labelGet(&r->table,"latitude");
    spec_is_ptr_equal(_t_get(r->root,path),def);
    spec_is_sem_equal(_r_get_symbol_by_label(r,"latitude"),lat);

    Structure latlong = _r_define_structure(r,"latlong",2,lat,lon);

    def = _t_child(r->defs.structures,latlong.id);
    T *l = _t_child(def,1);
    spec_is_str_equal((char *)_t_surface(l),"latlong");

    path = labelGet(&r->table,"latlong");
    spec_is_ptr_equal(_t_get(r->root,path),def);

    spec_is_structure_equal(r,_r_get_structure_by_label(r,"latlong"),latlong);
    spec_is_long_equal(__r_get_symbol_size(r,lat,0),sizeof(float));

    Symbol house_loc = _r_declare_symbol(r,latlong,"house location");
    spec_is_sem_equal(__r_get_symbol_structure(r,house_loc),latlong);
    spec_is_long_equal(__r_get_symbol_size(r,house_loc,0),sizeof(float)*2);

    Symbol name = _r_declare_symbol(r,CSTRING,"name");
    spec_is_long_equal(__r_get_symbol_size(r,name,"zippy"),(long)6);

    Structure namedhouse = _r_define_structure(r,"namedhouse",2,house_loc,name);

    Symbol home = _r_declare_symbol(r,namedhouse,"home");
    char surface[] ={1,2,3,4,5,6,7,8,'b','o','b','b','y',0};
    spec_is_long_equal(__r_get_symbol_size(r,home,surface),sizeof(float)*2+6);

    T *code = _t_new_root(ACTION);
    Process p = _r_code_process(r,code,"power","takes the mathematical power of the two params",NULL);
    spec_is_equal(_t_children(r->defs.processes),p.id);

    _r_free(r);
}

/**
 * define a "house location" symbol built out of a latlong
 *
 * @snippet spec/receptor_spec.h defineHouseLocation
*/
//! [defineHouseLocation]
void defineHouseLocation(Receptor *r,Symbol *lat,Symbol *lon, Structure *latlong, Symbol *house_loc) {
    *lat = _r_declare_symbol(r,FLOAT,"latitude");
    *lon = _r_declare_symbol(r,FLOAT,"longitude");
    *latlong = _r_define_structure(r,"latlong",2,*lat,*lon);
    *house_loc = _r_declare_symbol(r,*latlong,"house location");
}
//! [defineHouseLocation]

void testReceptorDefMatch() {
    //! [testReceptorDefMatch]
    Receptor *r = _r_new(TEST_RECEPTOR_SYMBOL);
    Symbol lat,lon,house_loc;
    Structure latlong;
    defineHouseLocation(r,&lat,&lon,&latlong,&house_loc);

    T *t = _t_new_root(house_loc);
    float x = 99.0;
    T *t_lat = _t_new(t,lat,&x,sizeof(x));
    T *t_lon = _t_new(t,lon,&x,sizeof(x));

    T *stx = _r_build_def_semtrex(r,house_loc);
    char buf[2000];
    spec_is_str_equal(_dump_semtrex(&r->defs,stx,buf),"/house location/(latitude,longitude)");
    __t_dump(&r->defs,stx,0,buf);
    spec_is_str_equal(buf,"(SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:house location) (SEMTREX_SEQUENCE (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:latitude)) (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:longitude))))");

    // a correctly structured tree should match its definition
    spec_is_true(_r_def_match(r,house_loc,t));

    // a correctly structured tree with different semantics shouldn't match its definition
    t_lon->contents.symbol = lat;
    spec_is_true(!_r_def_match(r,house_loc,t));

    // an incorrectly structured tree (i.e. missing a child) shouldn't match its definition
    _t_detach_by_idx(t,2);
    spec_is_true(!_r_def_match(r,house_loc,t));

    _t_free(stx);
    _t_free(t);
    _t_free(t_lon);
    _r_free(r);
    //! [testReceptorDefMatch]
}

/*
  Protocol Startpoint <- Data Source (of type carrier type)
     The carrier type is implicit in the first step's expectation semtrex.
  Protocol Endpoint -> Action Handler (which can accept data of my carrier type)
     The last step of a protocol is an "output" handler ACTION that must be provided at
     expression time
*/
Receptor *_makePingProtocolReceptor(Symbol *pingP) {
    Receptor *r;
    r = _r_new(TEST_RECEPTOR_SYMBOL);

    Symbol ping_protocol = _r_declare_symbol(r,PROTOCOL,"alive");
    Symbol ping = _r_declare_symbol(r,BIT,"ping_message");
    Symbol alive = _r_declare_symbol(r,BIT,"alive_message");
    *pingP = ping;

    // define a ping protocol with server and client sequences
    T *ps = r->defs.protocols;
    T *p = _t_newr(ps,ping_protocol);

    // define the steps that can be used in sequences
    Symbol listen_for_ping = _r_declare_symbol(r,PROTOCOL_STEP,"listen_for_ping");
    Symbol get_alive_response = _r_declare_symbol(r,PROTOCOL_STEP,"get_alive_response");

    T *steps = _t_newr(p,STEPS);
    T *step = _t_newr(steps,listen_for_ping);
    T *e = _t_newr(step,EXPECTATION);
    _sl(e,ping);
    T *params = _t_newr(step,PARAMS);

    T *ping_resp = _t_new_root(RESPOND);
    _t_newi(ping_resp,alive,1);
    Process proc = _r_code_process(r,ping_resp,"send alive response","long desc...",NULL);
    _t_newp(step,ACTION,proc);

    step = _t_newr(steps,get_alive_response);
    e = _t_newr(step,EXPECTATION);
    _sl(e,alive);
    params = _t_newr(step,PARAMS);

    // define the sequences (built of steps)
    Symbol alive_server = _r_declare_symbol(r,SEQUENCE,"alive_server");
    Symbol alive_client = _r_declare_symbol(r,SEQUENCE,"alive_client");

    T *sequences = _t_newr(p,SEQUENCES);

    // the alive_server sequence just has one step, which is to listen for the ping
    T *seq = _t_newr(sequences,alive_server);
    _t_news(seq,STEP_SYMBOL,listen_for_ping);

    // the alive_client sequence has two steps: send the ping, and listen for the alive response
    seq = _t_newr(sequences,alive_client);
    _t_news(seq,STEP_SYMBOL,get_alive_response);
    wjson(&r->defs,p,"protocol",-1);

    return r;
}

void testReceptorProtocol() {
    //! [testReceptorProtocol]
    Symbol ping;

    Receptor *r = _makePingProtocolReceptor(&ping);

    T *ps = r->defs.protocols;
    char *d = _td(r,ps);

    spec_is_str_equal(d,"(PROTOCOLS (alive (STEPS (listen_for_ping (EXPECTATION (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:ping_message))) (PARAMS) (ACTION:send alive response)) (get_alive_response (EXPECTATION (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:alive_message))) (PARAMS))) (SEQUENCES (alive_server (STEP_SYMBOL:listen_for_ping)) (alive_client (STEP_SYMBOL:get_alive_response)))))");

    Symbol alive_server = _r_get_symbol_by_label(r,"alive_server");
    _r_express_protocol(r,1,alive_server,DEFAULT_ASPECT,NULL);

    d = _td(r,r->flux);

    spec_is_str_equal(d,"(FLUX (ASPECT:1 (LISTENERS (LISTENER:NULL_SYMBOL (EXPECTATION (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:ping_message))) (PARAMS) (ACTION:send alive response))) (SIGNALS)))");

    // delivering a fake signal should return a ping
    ReceptorAddress f = 3; // DUMMY ADDR
    ReceptorAddress t = 4; // DUMMY ADDR
    T *signal = __r_make_signal(f,t,DEFAULT_ASPECT,_t_newi(0,ping,0));

    d = _td(r,signal);
    spec_is_str_equal(d,"(SIGNAL (ENVELOPE (RECEPTOR_ADDRESS:3) (RECEPTOR_ADDRESS:4) (ASPECT:1) (CARRIER:ping_message) (SIGNAL_UUID)) (BODY:{(ping_message:0)}))");

    T *result;
    Error err = _r_deliver(r,signal);

    // after delivery we should see the process in the active list with the semtrex match in the params
    d = _td(r,r->q->active->context->run_tree);
    spec_is_str_equal(d,"(RUN_TREE (process:RESPOND (alive_message:1)) (PARAMS))");

    // manually run the process queue
    _p_reduceq(r->q);

    result = _t_child(r->q->completed->context->run_tree,1);
    d = _td(r,result);
    spec_is_str_equal(d,"(SIGNAL_UUID)");

    d = _td(r,r->pending_signals);
    spec_is_str_equal(d,"(PENDING_SIGNALS (SIGNAL (ENVELOPE (RECEPTOR_ADDRESS:4) (RECEPTOR_ADDRESS:3) (ASPECT:1) (CARRIER:alive_message) (SIGNAL_UUID) (SIGNAL_UUID)) (BODY:{(alive_message:1)})))");
    //    _t_free(result);
    _r_free(r);
    //! [testReceptorProtocol]
}

void testReceptorInstanceNew() {
    //! [testReceptorInstancesNew]
    Receptor *r = _r_new(TEST_RECEPTOR_SYMBOL);
    Symbol lat,lon,house_loc;
    Structure latlong;
    defineHouseLocation(r,&lat,&lon,&latlong,&house_loc);

    // create a house location tree
    T *t = _t_new_root(house_loc);
    float ll[] = {132.5,92.3};
    T *t_lat = _t_new(t,lat,&ll[0],sizeof(float));
    T *t_lon = _t_new(t,lon,&ll[1],sizeof(float));

    Xaddr x = _r_new_instance(r,t);
    spec_is_equal(x.addr,1);
    spec_is_sem_equal(x.symbol,house_loc);

    float *ill;
    T *i = _r_get_instance(r,x);

    wjson(&r->defs,i,"houseloc",-1);
    spec_is_ptr_equal(t,i);

    _r_free(r);
    //! [testReceptorInstancesNew]
}

void testReceptorSerialize() {
    //! [testReceptorSerialize]

    Receptor *r = _r_new(TEST_RECEPTOR_SYMBOL);
    Symbol lat,lon,house_loc;
    Structure latlong;
    defineHouseLocation(r,&lat,&lon,&latlong,&house_loc);

    // create a house location tree
    T *t = _t_new_root(house_loc);
    float ll[] = {132.5,92.3};
    T *t_lat = _t_new(t,lat,&ll[0],sizeof(float));
    T *t_lon = _t_new(t,lon,&ll[1],sizeof(float));

    Xaddr x = _r_new_instance(r,t);

    // also add a sub-receptor as an instance so we can test proper
    // serialization of nested receptors
    T *ir = _t_new_root(INSTALLED_RECEPTOR);
    Receptor *r2 = _r_new(TEST_RECEPTOR_SYMBOL);
    _t_new_receptor(ir,TEST_RECEPTOR_SYMBOL,r2);
    Xaddr xr = _r_new_instance(r,ir);
    T *t2 = _t_newi(0,TEST_INT_SYMBOL,314);
    Xaddr x2 = _r_new_instance(r2,t2);

    void *surface;
    size_t length;
    char buf[2000];
    char buf1[2000];

    _r_serialize(r,&surface,&length);

    // serialized receptor is two stacked serialized mtrees, first one for the tree
    // and then one for the instances
    S *s1 = (S *)surface;
    S *s2 = (S *)(surface + s1->total_size);
    spec_is_long_equal(length,s1->total_size+s2->total_size);
    //    spec_is_long_equal(length,250);
    //    spec_is_long_equal(*(size_t *)surface,250);

    Receptor *ru = _r_unserialize(surface);

    //    __r_dump_instances(r);
    //    __r_dump_instances(ru);

    // check that the structures look the same by comparing a string dump of the two
    // receptors
    __t_dump(&r->defs,r->root,0,buf);
    __t_dump(&ru->defs,ru->root,0,buf1);
    spec_is_str_equal(buf1,buf);

    // check flux
    spec_is_sem_equal(_t_symbol(ru->flux),FLUX);
    spec_is_sem_equal(_t_symbol(ru->pending_signals),PENDING_SIGNALS);
    spec_is_sem_equal(_t_symbol(ru->pending_responses),PENDING_RESPONSES);

    // check that the unserialized receptor has the labels loaded into the label table
    int *path = labelGet(&ru->table,"latitude");
    int p[] = {1,2,1,TREE_PATH_TERMINATOR};
    spec_is_path_equal(path,p);
    spec_is_sem_equal(_r_get_symbol_by_label(ru,"latitude"),lat);
    spec_is_sem_equal(_r_get_structure_by_label(ru,"latlong"),latlong);

    // check that the unserialized receptor has all the instances loaded into the instance store too
    T *t1 = _r_get_instance(ru,x);
    buf[0] = buf1[0] = 0;
    __t_dump(&r->defs,t,0,buf);
    __t_dump(&ru->defs,t1,0,buf1);
    spec_is_str_equal(buf1,buf);

    t1 = _r_get_instance(ru,xr);
    buf[0] = buf1[0] = 0;
    __t_dump(&r->defs,ir,0,buf);
    __t_dump(&ru->defs,t1,0,buf1);
    spec_is_str_equal(buf1,buf);

    Receptor *r3 = __r_get_receptor(t1);
    t1 = _r_get_instance(r3,x2);
    spec_is_equal(*(int*)_t_surface(t1),314);

    //    __r_dump_instances(r3);  show the 314 int

    free(surface);
    _r_free(r);
    _r_free(ru);
    //! [testReceptorSerialize]
}


Symbol mantissa;
Symbol exponent;
Symbol exps[16];
Structure flt;
Structure integer;
Symbol latitude;
Symbol longitude;
Structure lat_long;
Symbol home_location;

void defineNums(Receptor *r) {
    int i = 0;
    char buf[10];
    for(i=0;i<16;i++){
	sprintf(buf,"exp%d",i);
	exps[i] = _r_declare_symbol(r,BIT,buf);
    }
    integer = _r_define_structure(r,"integer",16,exps[0],exps[1],exps[2],exps[3],exps[4],exps[5],exps[6],exps[7],exps[8],exps[9],exps[10],exps[11],exps[12],exps[13],exps[14],exps[15]);
    mantissa = _r_declare_symbol(r,integer,"mantissa");
    exponent = _r_declare_symbol(r,integer,"exponent");
    flt = _r_define_structure(r,"float",2,mantissa,exponent);
    latitude = _r_declare_symbol(r,flt,"latitude");
    longitude = _r_declare_symbol(r,flt,"longitude");
    lat_long = _r_define_structure(r,"latlong",2,latitude,longitude);
    home_location = _r_declare_symbol(r,lat_long,"home_location");
}

void makeInt(T *t,int v) {
    int i;
    for(i=0;i<16;i++){
	_t_newi(t,exps[i],(v>>i)&1);
    }
}
void testReceptorNums() {
    Receptor *r = _r_new(TEST_RECEPTOR_SYMBOL);
    defineNums(r);

    // create a home location tree
    T *t = _t_new_root(home_location);
    T *lat = _t_newr(t,latitude);
    T *lon = _t_newr(t,longitude);
    T *m = _t_newr(lat,mantissa);
    T *e = _t_newr(lat,exponent);
    makeInt(m,22);
    makeInt(e,1);
    m = _t_newr(lon,mantissa);
    e = _t_newr(lon,exponent);
    makeInt(m,1);
    makeInt(e,2);
    spec_is_str_equal(_td(r,t),"(home_location (latitude (mantissa (exp0:0) (exp1:1) (exp2:1) (exp3:0) (exp4:1) (exp5:0) (exp6:0) (exp7:0) (exp8:0) (exp9:0) (exp10:0) (exp11:0) (exp12:0) (exp13:0) (exp14:0) (exp15:0)) (exponent (exp0:1) (exp1:0) (exp2:0) (exp3:0) (exp4:0) (exp5:0) (exp6:0) (exp7:0) (exp8:0) (exp9:0) (exp10:0) (exp11:0) (exp12:0) (exp13:0) (exp14:0) (exp15:0))) (longitude (mantissa (exp0:1) (exp1:0) (exp2:0) (exp3:0) (exp4:0) (exp5:0) (exp6:0) (exp7:0) (exp8:0) (exp9:0) (exp10:0) (exp11:0) (exp12:0) (exp13:0) (exp14:0) (exp15:0)) (exponent (exp0:0) (exp1:1) (exp2:0) (exp3:0) (exp4:0) (exp5:0) (exp6:0) (exp7:0) (exp8:0) (exp9:0) (exp10:0) (exp11:0) (exp12:0) (exp13:0) (exp14:0) (exp15:0))))");
    wjson(&r->defs,t,"homeloc",-1);

    _t_free(t);
    _r_free(r);
}

void testReceptorEdgeStream() {
    FILE *rs,*ws;
    char buffer[] = "line1\nline2\n";

    rs = fmemopen(buffer, strlen (buffer), "r");
    Stream *reader_stream = _st_new_unix_stream(rs,1);

    VMHost *v = _v_new();

    char *output_data;
    size_t size;
    ws = open_memstream(&output_data,&size);
    Stream *writer_stream = _st_new_unix_stream(ws,0);

    Receptor *w = _r_makeStreamWriterReceptor(TEST_RECEPTOR_SYMBOL,TEST_STREAM_SYMBOL,writer_stream);

    Xaddr writer = _v_new_receptor(v,v->r,TEST_RECEPTOR_SYMBOL,w);

    Receptor *r = _r_makeStreamReaderReceptor(TEST_RECEPTOR_SYMBOL,TEST_STREAM_SYMBOL,reader_stream,writer.addr);
    Xaddr testReceptor =  _v_new_receptor(v,v->r,TEST_RECEPTOR_SYMBOL,r);

    spec_is_str_equal(_td(w,__r_get_listeners(w,DEFAULT_ASPECT)),"(LISTENERS (LISTENER:LINE (EXPECTATION (SEMTREX_SYMBOL_ANY)) (PARAMS (INTERPOLATE_SYMBOL:NULL_SYMBOL)) (ACTION:echo input to stream)))");

    // manually run the reader's process queue
    //debug_enable(D_STREAM);
    Q *q = r->q;
    Stream *st = reader_stream;
    spec_is_equal(_p_reduceq(q),noReductionErr);
    while(!(st->flags&StreamHasData) && st->flags&StreamAlive ) {sleepms(1);};
    spec_is_equal(_p_reduceq(q),noReductionErr);
    while(!(st->flags&StreamHasData) && st->flags&StreamAlive ) {sleepms(1);};
    spec_is_equal(_p_reduceq(q),noReductionErr);
    while(!(st->flags&StreamHasData) && st->flags&StreamAlive ) {sleepms(1);};
    spec_is_equal(_p_reduceq(q),noReductionErr);
    //debug_disable(D_STREAM);
    spec_is_false(st->flags&StreamAlive);

    spec_is_false(r->q->completed == NULL);
    if (r->q->completed) {
        spec_is_equal(r->q->completed->context->err,deadStreamReadReductionErr);
    }
    /* T *result = r->q->blocked->context->run_tree; */

    /* /// @todo BOOLEAN is what's left from the replicate.  Should it be something else? */
    /* spec_is_str_equal(_td(r,result),"(RUN_TREE (BOOLEAN:0))"); */

    spec_is_str_equal(_td(r,r->pending_signals),"(PENDING_SIGNALS (SIGNAL (ENVELOPE (RECEPTOR_ADDRESS:0) (RECEPTOR_ADDRESS:2) (ASPECT:1) (CARRIER:LINE) (SIGNAL_UUID)) (BODY:{(LINE:line1)})) (SIGNAL (ENVELOPE (RECEPTOR_ADDRESS:0) (RECEPTOR_ADDRESS:2) (ASPECT:1) (CARRIER:LINE) (SIGNAL_UUID)) (BODY:{(LINE:line2)})))");

    // manually run the signal sending code
    _v_deliver_signals(v,r);

    // and see that they've shown up in the writer receptor's flux signals list
    spec_is_str_equal(_td(w,__r_get_signals(w,DEFAULT_ASPECT)),"(SIGNALS (SIGNAL (ENVELOPE (RECEPTOR_ADDRESS:3) (RECEPTOR_ADDRESS:2) (ASPECT:1) (CARRIER:LINE) (SIGNAL_UUID)) (BODY:{(LINE:line1)}) (RUN_TREE (process:STREAM_WRITE (TEST_STREAM_SYMBOL) (PARAM_REF:/2/1)) (PARAMS (LINE:line1)))) (SIGNAL (ENVELOPE (RECEPTOR_ADDRESS:3) (RECEPTOR_ADDRESS:2) (ASPECT:1) (CARRIER:LINE) (SIGNAL_UUID)) (BODY:{(LINE:line2)}) (RUN_TREE (process:STREAM_WRITE (TEST_STREAM_SYMBOL) (PARAM_REF:/2/1)) (PARAMS (LINE:line2)))))");

    // and that they've been removed from process queue pending signals list
    spec_is_str_equal(_td(r,r->pending_signals),"(PENDING_SIGNALS)");

    spec_is_str_equal(_td(w,w->q->active->context->run_tree),"(RUN_TREE (process:STREAM_WRITE (TEST_STREAM_SYMBOL) (PARAM_REF:/2/1)) (PARAMS (LINE:line1)))");

    // manually run the process queue
    //    debug_enable(D_REDUCE);
    _p_reduceq(w->q);
    //    debug_disable(D_REDUCE);

    spec_is_str_equal(_td(w,w->q->completed->context->run_tree),"(RUN_TREE (REDUCTION_ERROR_SYMBOL:NULL_SYMBOL) (PARAMS (LINE:line2)))");

    spec_is_str_equal(output_data,"line1\nline2\n");

    _st_free(reader_stream);
    _st_free(writer_stream);
    free(output_data);
    _v_free(v);
}

void _testReceptorClockAddListener(Receptor *r) {
    T *expect = _t_new_root(EXPECTATION);

    /// @todo figure out why looking for the SECOND down a walk match fails
    // char *stx = "/<TICK:(%SECOND)>";
    // T *s = parseSemtrex(&r->d,stx);

    T *s =  _sl(expect,TICK);

    T *x = _t_newr(0,NOOP);
    int pt1[] = {2,1,TREE_PATH_TERMINATOR};
    _t_new(x,PARAM_REF,pt1,sizeof(int)*4);

    Process proc = _r_code_process(r,x,"noop return param","long desc...",NULL);
    T *act = _t_newp(0,ACTION,proc);

    T* params = _t_new_root(PARAMS);
    _t_news(params,INTERPOLATE_SYMBOL,NULL_SYMBOL);  // NULL_SYMBOL = the full match

    _r_add_listener(r,DEFAULT_ASPECT,TICK,expect,params,act);
}

void testReceptorClock() {
    Receptor *r = _r_makeClockReceptor();
    spec_is_str_equal(_td(r,r->root),"(CLOCK_RECEPTOR (DEFINITIONS (STRUCTURES) (SYMBOLS) (PROCESSES (PROCESS_CODING (PROCESS_NAME:plant a listener to send the time) (PROCESS_INTENTION:long desc...) (process:LISTEN (PARAM_REF:/2/1) (PARAMS (RECEPTOR_ADDRESS:0) (INTERPOLATE_SYMBOL:NULL_SYMBOL) (RESPONSE_CARRIER:NULL_SYMBOL)) (ACTION:SEND)) (PROCESS_SIGNATURE (OUTPUT_SIGNATURE (SIGNATURE_LABEL:result) (SIGNATURE_SYMBOL:NULL_SYMBOL)) (INPUT_SIGNATURE (SIGNATURE_LABEL:stream) (SIGNATURE_STRUCTURE:SEMTREX))))) (PROTOCOLS) (SCAPES)) (ASPECTS) (FLUX (ASPECT:1 (LISTENERS (LISTENER:CLOCK_TELL_TIME (EXPECTATION (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:CLOCK_TELL_TIME) (SEMTREX_GROUP:EXPECTATION (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:EXPECTATION))))) (PARAMS (INTERPOLATE_SYMBOL:EXPECTATION)) (ACTION:plant a listener to send the time))) (SIGNALS))) (RECEPTOR_STATE) (PENDING_SIGNALS) (PENDING_RESPONSES))");

   /* The clock receptor acts as if it receives a signal with contents TICK (which is of TIMESTAMP structure) for every time that updates a magic scape with that time according to which listeners have been planted.  This means you can plant listeners based on a semtrex for any kind of time you want.  If you want the current time just plant a listener for TICK.  If you want to listen for every second plant a listener on the Symbol literal SECOND, and the clock receptor will trigger the listener every time the SECOND changes.  You can also listen for particular intervals and times by adding specificity to the semtrex, so to trigger a 3:30am action a-la-cron listen for: "/<TICK:(%HOUR=3,MINUTE=30)>"
       @todo we should also make the clock receptor also seem to send signals of other semantic formats, i.e. so it's easy to listen for things like "on Wednesdays", or other semantic date/time identifiers.
     */

    // send the clock receptor a "tell me the time" request
    ReceptorAddress self = __r_get_self_address(r);
    T *tell = _t_new_root(CLOCK_TELL_TIME);
    T *e = _t_newr(tell,EXPECTATION);
    _sl(e,TICK);

    T *signal = __r_make_signal(self,self,DEFAULT_ASPECT,tell);
    // debug_enable(D_SIGNALS);
    _r_deliver(r,signal);
    _p_reduceq(r->q);

    // the listener should have now been planted!
    spec_is_str_equal(_td(r,_t_child(__r_get_listeners(r,DEFAULT_ASPECT),2)),"(LISTENER:NULL_SYMBOL (EXPECTATION (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:TICK))) (PARAMS (RECEPTOR_ADDRESS:0) (INTERPOLATE_SYMBOL:NULL_SYMBOL) (RESPONSE_CARRIER:NULL_SYMBOL)) (ACTION:SEND))");

    // "run" the receptor and verify that listener gets activated
    pthread_t thread;
    int rc = 0;
    rc = pthread_create(&thread,0,___clock_thread,r);
    if (rc){
        raise_error("ERROR; return code from pthread_create() is %d\n", rc);
    }

    // request to tell time should be on the signals list
    T *signals = __r_get_signals(r,DEFAULT_ASPECT);
    spec_is_str_equal(_td(r,signals),"(SIGNALS (SIGNAL (ENVELOPE (RECEPTOR_ADDRESS:0) (RECEPTOR_ADDRESS:0) (ASPECT:1) (CARRIER:CLOCK_TELL_TIME) (SIGNAL_UUID)) (BODY:{(CLOCK_TELL_TIME (EXPECTATION (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:TICK))))}) (RUN_TREE (REDUCTION_ERROR_SYMBOL:NULL_SYMBOL) (PARAMS (EXPECTATION (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:TICK)))))))");

    sleep(1);
    _p_reduceq(r->q);

    // save the tick that was sent for comparison later to see if it's the one recieved
    T *sent_tick = (T*)_t_surface(_t_child(_t_child(signals,2),2));

    // do manual signal delivery (code pulled from _v_deliver_signals but we don't
    // need the routing info in this case because we can just deliver to ourselves

    signals = r->pending_signals;
    while(_t_children(signals)>0) {
        T *s = _t_detach_by_idx(signals,1);

        Error err = _r_deliver(r,s);
        if (err) {
            raise_error("delivery error: %d",err);
        }
    }

    char buf1[100];
    __td(r,sent_tick,buf1);

    // the listener should have sent back the tick (which is right now "fish")
    signals = __r_get_signals(r,DEFAULT_ASPECT);
    T *received_tick = (T*)_t_surface(_t_child(_t_child(signals,3),2));

    spec_is_str_equal(_td(r,received_tick),buf1);

    __r_kill(r);

    void *status;
    rc = pthread_join(thread, &status);
    debug_disable(D_SIGNALS);
    if (rc) {
        raise_error("ERROR; return code from pthread_join() is %d\n", rc);
    }

    _r_free(r);
}

void testReceptor() {
    _setup_HTTPDefs();
    testReceptorCreate();
    testReceptorAddListener();
    testReceptorSignal();
    testReceptorSignalDeliver();
    testReceptorResponseDeliver();
    testReceptorAction();
    testReceptorDef();
    testReceptorDefMatch();
    testReceptorProtocol();
    testReceptorInstanceNew();
    testReceptorSerialize();
    testReceptorNums();
    testReceptorEdgeStream();
    testReceptorClock();
    _cleanup_HTTPDefs();
}
