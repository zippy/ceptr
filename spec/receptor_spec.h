/**
 * @file receptor_spec.h
 * @copyright Copyright (C) 2013-2016, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 * @ingroup tests
 */

#include "../src/ceptr.h"
#include "../src/receptor.h"
#include "../src/def.h"
#include "../src/accumulator.h"
#include "../src/vmhost.h"
#include "../src/protocol.h"
#include "http_example.h"
#include <unistd.h>

void testReceptorCreate() {
    //! [testReceptorCreate]
    Receptor *r;
    r = _r_new(G_sem,TEST_RECEPTOR);

    spec_is_symbol_equal(r,_t_symbol(r->root),RECEPTOR_INSTANCE);

    T *t;

    // test that expectations and signals are set up correctly on the default aspect
    t = __r_get_expectations(r,DEFAULT_ASPECT);
    spec_is_symbol_equal(r,_t_symbol(t),EXPECTATIONS);
    t = __r_get_signals(r,DEFAULT_ASPECT);
    spec_is_symbol_equal(r,_t_symbol(t),SIGNALS);

    // test that the flux is set up correctly
    int p[] = {ReceptorInstanceStateIdx,ReceptorFluxIdx,TREE_PATH_TERMINATOR};
    t = _t_get(r->root,p);
    spec_is_symbol_equal(r,_t_symbol(r->flux),FLUX);
    spec_is_ptr_equal(t,r->flux);
    t = _t_child(r->flux,1);
    spec_is_symbol_equal(r,_t_symbol(t),DEFAULT_ASPECT);

    spec_is_str_equal(t2s(r->root),"(RECEPTOR_INSTANCE (INSTANCE_OF:TEST_RECEPTOR) (CONTEXT_NUM:3) (PARENT_CONTEXT_NUM:0) (RECEPTOR_STATE (FLUX (DEFAULT_ASPECT (EXPECTATIONS) (SIGNALS))) (PENDING_SIGNALS) (PENDING_RESPONSES) (RECEPTOR_ELAPSED_TIME:0)))");

    _r_free(r);
    //! [testReceptorCreate]
}

void testReceptorAddRemoveExpectation() {
    Receptor *r;
    r = _r_new(G_sem,TEST_RECEPTOR);

    Symbol dummy = {r->context,SEM_TYPE_SYMBOL,1};
    // test that you can add a expectation to a receptor's aspect
    T *s = _t_new_root(PATTERN);
    _sl(s,dummy);
    T *a = _t_news(0,ACTION,NULL_PROCESS);
    _r_add_expectation(r,DEFAULT_ASPECT,TEST_INT_SYMBOL,s,a,0,0,NULL);

    T *es = __r_get_expectations(r,DEFAULT_ASPECT);
    T *e = _t_child(es,1);      // expectation should have been added as first child of expectations
    spec_is_str_equal(_td(r,e),"(EXPECTATION (CARRIER:TEST_INT_SYMBOL) (PATTERN (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:TEST_INT_SYMBOL))) (ACTION:NULL_PROCESS) (PARAMS) (END_CONDITIONS (UNLIMITED)))");

    _r_remove_expectation(r,e);

    spec_is_str_equal(_td(r,es),"(EXPECTATIONS)");
    _r_free(r);
}

void testReceptorSignal() {
    Receptor *r = _r_new(G_sem,TEST_RECEPTOR);
    T *sc,*signal_contents = _t_newi(0,TEST_INT_SYMBOL,314);
    ReceptorAddress f = {3}; // DUMMY ADDR
    ReceptorAddress t = {4}; // DUMMY ADDR

    T *s = __r_make_signal(f,t,DEFAULT_ASPECT,TESTING,sc=_t_clone(signal_contents),0,0);

    spec_is_symbol_equal(r,_t_symbol(s),SIGNAL);

    T *envelope = _t_child(s,SignalEnvelopeIdx);
    spec_is_str_equal(t2s(envelope),"(ENVELOPE (FROM_ADDRESS (RECEPTOR_ADDR:3)) (TO_ADDRESS (RECEPTOR_ADDR:4)) (ASPECT_IDENT:DEFAULT_ASPECT) (CARRIER:TESTING) (SIGNAL_UUID))");
    T *body = _t_child(s,SignalBodyIdx);
    spec_is_str_equal(t2s(body),"(BODY:{(TEST_INT_SYMBOL:314)})");
    T *contents = (T*)_t_surface(body);
    spec_is_ptr_equal(sc,contents);
    _t_free(s);

    UUIDt u = __uuid_gen();
    s = __r_make_signal(f,t,DEFAULT_ASPECT,TESTING,_t_clone(signal_contents),&u,0);
    spec_is_str_equal(t2s(s),"(SIGNAL (ENVELOPE (FROM_ADDRESS (RECEPTOR_ADDR:3)) (TO_ADDRESS (RECEPTOR_ADDR:4)) (ASPECT_IDENT:DEFAULT_ASPECT) (CARRIER:TESTING) (SIGNAL_UUID) (IN_RESPONSE_TO_UUID)) (BODY:{(TEST_INT_SYMBOL:314)}))");
    int p[] = {SignalEnvelopeIdx,EnvelopeExtraIdx,TREE_PATH_TERMINATOR};
    T *ru = _t_get(s,p);
    spec_is_true(__uuid_equal(&u,_t_surface(ru)));

    _t_free(s);
    T *ec = defaultRequestUntil();
    s = __r_make_signal(f,t,DEFAULT_ASPECT,TESTING,signal_contents,0,ec);
    spec_is_ptr_equal(ec,_t_get(s,p));
    _t_free(s);

    _r_free(r);
}

void testReceptorSignalDeliver() {
    Receptor *r = _r_new(G_sem,TEST_RECEPTOR);
    T *signal_contents = _t_newi(0,TEST_INT_SYMBOL,314);
    ReceptorAddress f = {3}; // DUMMY ADDR
    ReceptorAddress t = {4}; // DUMMY ADDR

    // a new signal should simply be placed on the flux when delivered
    T *s = __r_make_signal(f,t,DEFAULT_ASPECT,TESTING,signal_contents,0,0);
    spec_is_equal(_r_deliver(r,s),noDeliveryErr);

    T *signals = __r_get_signals(r,DEFAULT_ASPECT);
    spec_is_str_equal(_td(r,signals),"(SIGNALS (SIGNAL (ENVELOPE (FROM_ADDRESS (RECEPTOR_ADDR:3)) (TO_ADDRESS (RECEPTOR_ADDR:4)) (ASPECT_IDENT:DEFAULT_ASPECT) (CARRIER:TESTING) (SIGNAL_UUID)) (BODY:{(TEST_INT_SYMBOL:314)})))");
    _r_free(r);
}

extern int G_next_process_id;
void testReceptorResponseDeliver() {
    Receptor *r = _r_new(G_sem,TEST_RECEPTOR);

    G_next_process_id = 0; // reset the process ids so the test will always work

    ReceptorAddress tt = {4}; // DUMMY ADDR

    // set up receptor to have sent and signal and blocked waiting for the response
    T *t = _t_new_root(RUN_TREE);
    T *p = _t_new_root(NOOP);
    T *req = _t_newr(p,REQUEST);
    __r_make_addr(req,TO_ADDRESS,tt);
    _t_news(req,ASPECT_IDENT,DEFAULT_ASPECT);
    _t_news(req,CARRIER,TESTING);
    _t_newi(req,TEST_INT_SYMBOL,98789);
    _t_news(req,RESPONSE_CARRIER,TESTING);

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
    T *pr = _t_child(r->pending_responses,1);
    spec_is_str_equal(_td(r,_t_child(pr,1)),"(SIGNAL_UUID)");
    spec_is_str_equal(_td(r,_t_child(pr,2)),"(CARRIER:TESTING)");
    spec_is_str_equal(_td(r,_t_child(pr,3)),"(WAKEUP_REFERENCE (PROCESS_IDENT:1) (CODE_PATH:/1/1))");
    T *ec = _t_child(pr,4);
    spec_is_sem_equal(_t_symbol(ec),END_CONDITIONS);

    // create a response signals
    ReceptorAddress from = {4}; // DUMMY ADDR
    ReceptorAddress to = {0}; // DUMMY ADDR

    // get the original signal uuid from the run tree
    UUIDt response_id = *(UUIDt *)_t_surface(_t_child(_t_child(rt,1),1));
    T *s = __r_make_signal(from,to,DEFAULT_ASPECT,TESTING,_t_new_str(0,TEST_STR_SYMBOL,"foo"),&response_id,0);

    //    debug_enable(D_SIGNALS);
    spec_is_equal(_r_deliver(r,s),noDeliveryErr);
    //    debug_disable(D_SIGNALS);

    // block list should now be empty
    spec_is_ptr_equal(r->q->blocked,NULL);

    // and the run tree should now be active and have the response contents value
    // in place for reduction.
    rt = r->q->active->context->run_tree;
    spec_is_str_equal(_td(r,rt),"(RUN_TREE (process:NOOP (TEST_STR_SYMBOL:foo)))");

    // and the pending_responses list should be cleared too because we only asked for one response.
    spec_is_str_equal(_td(r,r->pending_responses),"(PENDING_RESPONSES)");

    _r_free(r);
}

void testReceptorEndCondition() {
    T *until = _t_new_root(END_CONDITIONS);
    _t_newr(until,UNLIMITED);
    bool cleanup,allow;
    evaluateEndCondition(until,&cleanup,&allow);
    spec_is_false(cleanup);spec_is_true(allow);
    _t_free(until);

    until = _t_new_root(END_CONDITIONS);
    _t_newi(until,COUNT,2);
    evaluateEndCondition(until,&cleanup,&allow);
    spec_is_false(cleanup);spec_is_true(allow);
    spec_is_str_equal(t2s(until),"(END_CONDITIONS (COUNT:1))");
    evaluateEndCondition(until,&cleanup,&allow);
    spec_is_true(cleanup);spec_is_true(allow);
    spec_is_str_equal(t2s(until),"(END_CONDITIONS (COUNT:0))");
    _t_free(until);
    //    debug_enable(D_SIGNALS);
    until = _t_new_root(END_CONDITIONS);
    T *ts = __r_make_timestamp(TIMEOUT_AT,-2);
    _t_add(until,ts);
    evaluateEndCondition(until,&cleanup,&allow);
    spec_is_true(cleanup);spec_is_false(allow);
    _t_free(until);

    until = _t_new_root(END_CONDITIONS);
    ts = __r_make_timestamp(TIMEOUT_AT,+2);
    _t_add(until,ts);
    evaluateEndCondition(until,&cleanup,&allow);
    spec_is_false(cleanup);spec_is_true(allow);
    _t_free(until);
    debug_disable(D_SIGNALS);

}

void testReceptorExpectation() {
    //! [testReceptorExpectation]
    Receptor *r = _r_new(G_sem,TEST_RECEPTOR);

    // The signal is an HTTP request
    T *signal_contents = _makeTestHTTPRequestTree(); // GET /groups/5/users.json?sort_by=last_name?page=2 HTTP/1.0

    ReceptorAddress f = {3}; // DUMMY ADDR
    ReceptorAddress t = {4}; // DUMMY ADDR

    T *signal = __r_make_signal(f,t,DEFAULT_ASPECT,HTTP_REQUEST,signal_contents,0,0);

    // our expectation pattern should match on the first path segment
    // /HTTP_REQUEST/.,.,HTTP_REQUEST_PATH/HTTP_REQUEST_PATH_SEGMENTS/<HTTP_REQUEST_PATH_SEGMENT:HTTP_REQUEST_PATH_SEGMENT>
    T *pattern = _t_new_root(PATTERN);
    char *stx = "/HTTP_REQUEST/(.,.,HTTP_REQUEST_PATH/HTTP_REQUEST_PATH_SEGMENTS/<HTTP_REQUEST_PATH_SEGMENT:HTTP_REQUEST_PATH_SEGMENT>)";
    T *req = parseSemtrex(G_sem,stx);
    _t_add(pattern,req);
/*    T *req = _t_news(pattern,SEMTREX_SYMBOL_LITERAL,HTTP_REQUEST);
    T *seq = _t_newr(req,SEMTREX_SEQUENCE);
    _t_newr(seq,SEMTREX_SYMBOL_ANY);  // skips the Version
    _t_newr(seq,SEMTREX_SYMBOL_ANY);  // skips over the Method
    T *path = _t_news(seq,SEMTREX_SYMBOL_LITERAL,HTTP_REQUEST_PATH);
    T *segs = _t_news(path,SEMTREX_SYMBOL_LITERAL,HTTP_REQUEST_PATH_SEGMENTS);
    T *g = _t_news(segs,SEMTREX_GROUP,HTTP_REQUEST_PATH_SEGMENT);
    _t_news(g,SEMTREX_SYMBOL_LITERAL,HTTP_REQUEST_PATH_SEGMENT);
*/

    // gotta load the defs into the receptor for printing things out to work right.
    //    r->defs.symbols = _t_clone(test_HTTP_defs.symbols);
    // r->defs.structures = _t_clone(test_HTTP_defs.structures);

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

    // have this expectation clean itself up after one match
    T *until = _t_new_root(END_CONDITIONS);
    _t_newi(until,COUNT,1);

    T *using = NULL;

    _r_add_expectation(r,DEFAULT_ASPECT,HTTP_REQUEST,pattern,act,params,until,using);

    Error err = _r_deliver(r,signal);
    spec_is_equal(err,noDeliveryErr);

    // signal and run_tree should be added and ready on the process queue
    spec_is_equal(r->q->contexts_count,1);
    spec_is_str_equal(_td(r,__r_get_signals(r,DEFAULT_ASPECT)),
                      "(SIGNALS (SIGNAL (ENVELOPE (FROM_ADDRESS (RECEPTOR_ADDR:3)) (TO_ADDRESS (RECEPTOR_ADDR:4)) (ASPECT_IDENT:DEFAULT_ASPECT) (CARRIER:HTTP_REQUEST) (SIGNAL_UUID)) (BODY:{(HTTP_REQUEST (HTTP_REQUEST_VERSION (VERSION_MAJOR:1) (VERSION_MINOR:0)) (HTTP_REQUEST_METHOD:GET) (HTTP_REQUEST_PATH (HTTP_REQUEST_PATH_SEGMENTS (HTTP_REQUEST_PATH_SEGMENT:groups) (HTTP_REQUEST_PATH_SEGMENT:5)) (HTTP_REQUEST_PATH_FILE (FILE_NAME:users) (FILE_EXTENSION:json)) (HTTP_REQUEST_PATH_QUERY (HTTP_REQUEST_PATH_QUERY_PARAMS (HTTP_REQUEST_PATH_QUERY_PARAM (PARAM_KEY:sort_by) (PARAM_VALUE:last_name)) (HTTP_REQUEST_PATH_QUERY_PARAM (PARAM_KEY:page) (PARAM_VALUE:2))))))}) (RUN_TREE (process:RESPOND (CARRIER:HTTP_RESPONSE) (PARAM_REF:/2/1)) (PARAMS (HTTP_RESPONSE (HTTP_RESPONSE_STATUS (STATUS_VALUE:200) (STATUS_TEXT:OK)) (HTTP_HEADERS (CONTENT_TYPE (MEDIA_TYPE_IDENT:TEXT_MEDIA_TYPE) (MEDIA_SUBTYPE_IDENT:CEPTR_TEXT_MEDIA_SUBTYPE))) (HTTP_RESPONSE_BODY (HTTP_REQUEST_PATH_SEGMENT:groups)))))))"
                      );

    // manually run the process queue
    _p_reduceq(r->q);

    // should add a pending signal to be sent with the matched PATH_SEGMENT returned as the response signal body
    spec_is_str_equal(_td(r,r->pending_signals),"(PENDING_SIGNALS (SIGNAL (ENVELOPE (FROM_ADDRESS (RECEPTOR_ADDR:4)) (TO_ADDRESS (RECEPTOR_ADDR:3)) (ASPECT_IDENT:DEFAULT_ASPECT) (CARRIER:HTTP_RESPONSE) (SIGNAL_UUID) (IN_RESPONSE_TO_UUID)) (BODY:{(HTTP_RESPONSE (HTTP_RESPONSE_STATUS (STATUS_VALUE:200) (STATUS_TEXT:OK)) (HTTP_HEADERS (CONTENT_TYPE (MEDIA_TYPE_IDENT:TEXT_MEDIA_TYPE) (MEDIA_SUBTYPE_IDENT:CEPTR_TEXT_MEDIA_SUBTYPE))) (HTTP_RESPONSE_BODY (HTTP_REQUEST_PATH_SEGMENT:groups)))})))");

    result = _t_child(r->q->completed->context->run_tree,1);
    spec_is_str_equal(_td(r,result),"(SIGNAL_UUID)");

    T *es = __r_get_expectations(r,DEFAULT_ASPECT);
    spec_is_str_equal(_td(r,es),"(EXPECTATIONS)");

    _r_free(r);
    //! [testReceptorExpectation]
}

void testReceptorDef() {
    Receptor *r = _r_new(G_sem,TEST_RECEPTOR);

    Symbol lat = _r_define_symbol(r,FLOAT,"Latitude");
    Symbol lon = _r_define_symbol(r,FLOAT,"Longitude");

    spec_is_structure_equal(r,__r_get_symbol_structure(r,lat),FLOAT);

    T *def;
    spec_is_str_equal((char *)_t_surface(_t_child(def = _sem_get_def(r->sem,lat),DefLabelIdx)),"Latitude");
    spec_is_str_equal((char *)_t_surface(_t_child(_sem_get_def(r->sem,lon),DefLabelIdx)),"Longitude");

     spec_is_sem_equal(_r_get_sem_by_label(r,"Latitude"),lat);
    spec_is_sem_equal(_sem_get_by_label(G_sem,"Latitude",r->context),lat);

    Structure latlong = _r_define_structure(r,"Latlong",2,lat,lon);

    def = _sem_get_def(r->sem,latlong);
    T *l = _t_child(def,DefLabelIdx);
    spec_is_str_equal((char *)_t_surface(l),"Latlong");

    spec_is_structure_equal(r,_r_get_sem_by_label(r,"Latlong"),latlong);
    spec_is_long_equal(__r_get_symbol_size(r,lat,0),sizeof(float));

    Symbol house_loc = _r_define_symbol(r,latlong,"house location");
    spec_is_sem_equal(__r_get_symbol_structure(r,house_loc),latlong);
    spec_is_long_equal(__r_get_symbol_size(r,house_loc,0),sizeof(float)*2);

    Symbol name = _r_define_symbol(r,CSTRING,"name");
    spec_is_long_equal(__r_get_symbol_size(r,name,"zippy"),(long)6);

    Structure namedhouse = _r_define_structure(r,"namedhouse",2,house_loc,name);

    Symbol home = _r_define_symbol(r,namedhouse,"home");
    char surface[] ={1,2,3,4,5,6,7,8,'b','o','b','b','y',0};
    spec_is_long_equal(__r_get_symbol_size(r,home,surface),sizeof(float)*2+6);

    T *code = _t_new_root(NOOP);
    Process p = _r_define_process(r,code,"power","takes the mathematical power of the two params",NULL);
    def = _sem_get_def(r->sem,p);

    //    spec_is_str_equal(_td(r,def),"");

    _r_free(r);
}

/**
 * define a "house location" symbol built out of a latlong
 *
 * @snippet spec/receptor_spec.h defineHouseLocation
*/
//! [defineHouseLocation]
void defineHouseLocation(Receptor *r,Symbol *lat,Symbol *lon, Structure *latlong, Symbol *house_loc) {
    *lat = _r_define_symbol(r,FLOAT,"latitude");
    *lon = _r_define_symbol(r,FLOAT,"longitude");
    *latlong = _r_define_structure(r,"latlong",2,*lat,*lon);
    *house_loc = _r_define_symbol(r,*latlong,"house location");
}
//! [defineHouseLocation]

void testReceptorDefMatch() {
    //! [testReceptorDefMatch]
    Receptor *r = _r_new(G_sem,TEST_RECEPTOR);
    Symbol lat,lon,house_loc;
    Structure latlong;
    defineHouseLocation(r,&lat,&lon,&latlong,&house_loc);

    T *t = _t_new_root(house_loc);
    float x = 99.0;
    T *t_lat = _t_new(t,lat,&x,sizeof(x));
    T *t_lon = _t_new(t,lon,&x,sizeof(x));

    T *stx = _r_build_def_semtrex(r,house_loc);
    char buf[2000];
    spec_is_str_equal(_dump_semtrex(r->sem,stx,buf),"/house location/(latitude,longitude)");
    __t_dump(r->sem,stx,0,buf);
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

void testReceptorInstances() {
    //! [testReceptorInstances]
    Receptor *r = _r_new(G_sem,TEST_RECEPTOR);
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

    wjson(r->sem,i,"houseloc",-1);
    spec_is_ptr_equal(i,t);

    _r_delete_instance(r,x);
    i = _r_get_instance(r,x);
    spec_is_ptr_equal(i,NULL);

    _r_free(r);
    //! [testReceptorInstances]
}

void testReceptorSerialize() {
    //! [testReceptorSerialize]

    Receptor *r = _r_new(G_sem,TEST_RECEPTOR);
    Symbol lat,lon,house_loc;
    Structure latlong;
    lat = _r_get_sem_by_label(r,"latitude");
    lon = _r_get_sem_by_label(r,"longitude");
    house_loc = _r_get_sem_by_label(r,"house location");
    latlong = _r_get_sem_by_label(r,"latlong");

    // create a house location tree
    T *t = _t_new_root(house_loc);
    float ll[] = {132.5,92.3};
    T *t_lat = _t_new(t,lat,&ll[0],sizeof(float));
    T *t_lon = _t_new(t,lon,&ll[1],sizeof(float));

    Xaddr x = _r_new_instance(r,t);

    // also add a sub-receptor as an instance so we can test proper
    // serialization of nested receptors
    Receptor *r2 = _r_new(G_sem,TEST_RECEPTOR);
    T *ir = _t_new_receptor(0,TEST_RECEPTOR,r2);
    Xaddr xr = _r_new_instance(r,ir);
    T *t2 = _t_newi(0,TEST_INT_SYMBOL,314);
    Xaddr x2 = _r_new_instance(r2,t2);

    void *surface;
    size_t length;
    char buf[2000];
    char buf1[2000];

    T *signal_contents = _t_newi(0,TEST_INT_SYMBOL,314);
    ReceptorAddress from = {3}; // DUMMY ADDR
    ReceptorAddress to = {4}; // DUMMY ADDR

    // add a signal too
    T *s = __r_make_signal(from,to,DEFAULT_ASPECT,TESTING,signal_contents,0,0);
    _r_deliver(r,s);

    _r_serialize(r,&surface,&length);

    // serialized receptor is two stacked serialized mtrees, first one for the state tree
    // and then the second for the receptor's instances
    S *s1 = (S *)surface;
    S *s2 = (S *)(surface + s1->total_size);
    spec_is_long_equal(length,s1->total_size+s2->total_size);
    //    spec_is_long_equal(length,250);
    //    spec_is_long_equal(*(size_t *)surface,250);

    Receptor *ru = _r_unserialize(G_sem,surface);
    //    __r_dump_instances(r);
    //    __r_dump_instances(ru);

    // check that the structures look the same by comparing a string dump of the two
    // receptors
    __t_dump(r->sem,r->root,0,buf);
    __t_dump(ru->sem,ru->root,0,buf1);
    spec_is_str_equal(buf1,buf);

    // check flux
    spec_is_sem_equal(_t_symbol(ru->flux),FLUX);
    spec_is_sem_equal(_t_symbol(ru->pending_signals),PENDING_SIGNALS);
    spec_is_sem_equal(_t_symbol(ru->pending_responses),PENDING_RESPONSES);

    // check that the unserialized receptor is matched up to the correct definitions in the semtable
    spec_is_sem_equal(_r_get_sem_by_label(ru,"latitude"),lat);
    spec_is_sem_equal(_r_get_sem_by_label(ru,"latlong"),latlong);

    // check that the unserialized receptor has all the instances loaded into the instance store too
    T *t1 = _r_get_instance(ru,x);
    buf[0] = buf1[0] = 0;
    __t_dump(r->sem,t,0,buf);
    __t_dump(ru->sem,t1,0,buf1);
    spec_is_str_equal(buf1,buf);

    t1 = _r_get_instance(ru,xr);
    spec_is_true(t1 != NULL);
    if (t1) {
        buf[0] = buf1[0] = 0;
        __t_dump(r->sem,ir,0,buf);
        __t_dump(ru->sem,t1,0,buf1);
        spec_is_str_equal(buf1,buf);
        Receptor *r3 = __r_get_receptor(t1);
        t1 = _r_get_instance(r3,x2);
        spec_is_equal(*(int*)_t_surface(t1),314);
        _r_free(ru);
    }
    //__r_dump_instances(r3); // show the 314 int

    free(surface);
    _r_free(r);
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
	exps[i] = _r_define_symbol(r,BIT,buf);
    }
    integer = _r_define_structure(r,"integer",16,exps[0],exps[1],exps[2],exps[3],exps[4],exps[5],exps[6],exps[7],exps[8],exps[9],exps[10],exps[11],exps[12],exps[13],exps[14],exps[15]);
    mantissa = _r_define_symbol(r,integer,"mantissa");
    exponent = _r_define_symbol(r,integer,"exponent");
    flt = _r_define_structure(r,"float",2,mantissa,exponent);
    latitude = _r_define_symbol(r,flt,"latitude");
    longitude = _r_define_symbol(r,flt,"longitude");
    lat_long = _r_define_structure(r,"latlong",2,latitude,longitude);
    home_location = _r_define_symbol(r,lat_long,"home_location");
}

void makeInt(T *t,int v) {
    int i;
    for(i=0;i<16;i++){
	_t_newi(t,exps[i],(v>>i)&1);
    }
}
void testReceptorNums() {
    Receptor *r = _r_new(G_sem,TEST_RECEPTOR);
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
    wjson(r->sem,t,"homeloc",-1);

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

    Receptor *r = _r_makeStreamEdgeReceptor(v->sem);
    Xaddr edge = _v_new_receptor(v,v->r,STREAM_EDGE,r);
    _r_addWriter(r,TEST_STREAM_SYMBOL,writer_stream,DEFAULT_ASPECT);
    _r_addReader(r,TEST_STREAM_SYMBOL,reader_stream,r->addr,DEFAULT_ASPECT,LINE,LINE);

    spec_is_str_equal(_td(r,__r_get_expectations(r,DEFAULT_ASPECT)),"(EXPECTATIONS (EXPECTATION (CARRIER:NULL_SYMBOL) (PATTERN (SEMTREX_SYMBOL_ANY)) (ACTION:echo2stream) (PARAMS (TEST_STREAM_SYMBOL) (SLOT (USAGE:NULL_SYMBOL))) (END_CONDITIONS (UNLIMITED))))");

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

    spec_is_str_equal(_td(r,r->pending_signals),"(PENDING_SIGNALS (SIGNAL (ENVELOPE (FROM_ADDRESS (RECEPTOR_ADDR:3)) (TO_ADDRESS (RECEPTOR_ADDR:3)) (ASPECT_IDENT:DEFAULT_ASPECT) (CARRIER:LINE) (SIGNAL_UUID)) (BODY:{(LINE:line1)})) (SIGNAL (ENVELOPE (FROM_ADDRESS (RECEPTOR_ADDR:3)) (TO_ADDRESS (RECEPTOR_ADDR:3)) (ASPECT_IDENT:DEFAULT_ASPECT) (CARRIER:LINE) (SIGNAL_UUID)) (BODY:{(LINE:line2)})))");

    // manually run the signal sending code
    _v_deliver_signals(v,r);

    // and see that they've shown up in the edge receptor's flux signals list
    spec_is_str_equal(_td(r,__r_get_signals(r,DEFAULT_ASPECT)),"(SIGNALS (SIGNAL (ENVELOPE (FROM_ADDRESS (RECEPTOR_ADDR:3)) (TO_ADDRESS (RECEPTOR_ADDR:3)) (ASPECT_IDENT:DEFAULT_ASPECT) (CARRIER:LINE) (SIGNAL_UUID)) (BODY:{(LINE:line1)}) (RUN_TREE (process:STREAM_WRITE (PARAM_REF:/2/1) (PARAM_REF:/2/2)) (PARAMS (TEST_STREAM_SYMBOL) (LINE:line1)))) (SIGNAL (ENVELOPE (FROM_ADDRESS (RECEPTOR_ADDR:3)) (TO_ADDRESS (RECEPTOR_ADDR:3)) (ASPECT_IDENT:DEFAULT_ASPECT) (CARRIER:LINE) (SIGNAL_UUID)) (BODY:{(LINE:line2)}) (RUN_TREE (process:STREAM_WRITE (PARAM_REF:/2/1) (PARAM_REF:/2/2)) (PARAMS (TEST_STREAM_SYMBOL) (LINE:line2)))))");

    // and that they've been removed from process queue pending signals list
    spec_is_str_equal(_td(r,r->pending_signals),"(PENDING_SIGNALS)");

    spec_is_str_equal(_td(r,r->q->active->context->run_tree),"(RUN_TREE (process:STREAM_WRITE (PARAM_REF:/2/1) (PARAM_REF:/2/2)) (PARAMS (TEST_STREAM_SYMBOL) (LINE:line1)))");

    // manually run the process queue
    //    debug_enable(D_REDUCE);
    _p_reduceq(r->q);
    //    debug_disable(D_REDUCE);

    spec_is_str_equal(_td(r,r->q->completed->context->run_tree),"(RUN_TREE (REDUCTION_ERROR_SYMBOL:NULL_SYMBOL) (PARAMS (TEST_STREAM_SYMBOL) (LINE:line2)))");

    spec_is_str_equal(output_data,"line1\nline2\n");

    _st_free(reader_stream);
    _st_free(writer_stream);
    free(output_data);
    _v_free(v);
}

void testReceptorClock() {
    Receptor *r = _r_makeClockReceptor(G_sem);
    spec_is_str_equal(_td(r,r->root),"(RECEPTOR_INSTANCE (INSTANCE_OF:CLOCK_RECEPTOR) (CONTEXT_NUM:4) (PARENT_CONTEXT_NUM:0) (RECEPTOR_STATE (FLUX (DEFAULT_ASPECT (EXPECTATIONS (EXPECTATION (CARRIER:tell_time) (PATTERN (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:CLOCK_TELL_TIME))) (ACTION:respond with current time) (PARAMS) (END_CONDITIONS (UNLIMITED)))) (SIGNALS))) (PENDING_SIGNALS) (PENDING_RESPONSES) (RECEPTOR_ELAPSED_TIME:0)))");

   /*
      The clock receptor should do two things: respond to CLOCK_TELL_TIME signals with the current time, and also allow you to plant a listener based on a semtrex for any kind of time you want.  If you want the current time just plant a listener for TICK.  If you want to listen for every second plant a listener on the Symbol literal SECOND, and the clock receptor will trigger the listener every time the SECOND changes.  You can also listen for particular intervals and times by adding specificity to the semtrex, so to trigger a 3:30am action a-la-cron listen for: "/<TICK:(%HOUR=3,MINUTE=30)>"
       @todo we should also make the clock receptor also respond to other semantic formats, i.e. so it's easy to listen for things like "on Wednesdays", or other semantic date/time identifiers.
     */
    Protocol time = _sem_get_by_label(G_sem,"time",r->context);
    T *def = _sem_get_def(G_sem,time);
    spec_is_str_equal(_td(r,def),"(PROTOCOL_DEFINITION (PROTOCOL_LABEL:time) (PROTOCOL_SEMANTICS (ROLE:TIME_TELLER) (ROLE:TIME_HEARER) (GOAL:REQUEST_HANDLER)) (tell_time (INITIATE (ROLE:TIME_HEARER) (DESTINATION (ROLE:TIME_TELLER)) (ACTION:time_request)) (EXPECT (ROLE:TIME_TELLER) (SOURCE (ROLE:TIME_HEARER)) (PATTERN (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:CLOCK_TELL_TIME))) (ACTION:respond with current time))))");

    //    debug_enable(D_SIGNALS);

    // send the clock receptor a "tell me the time" request by initiating the tell_time interaction in the protocol

    T *bindings = _t_new_root(PROTOCOL_BINDINGS);
    T *res = _t_newr(bindings,RESOLUTION);
    T *w = _t_newr(res,WHICH_RECEPTOR);
    _t_news(w,ROLE,TIME_TELLER);
    __r_make_addr(w,ACTUAL_RECEPTOR,r->addr);
    res = _t_newr(bindings,RESOLUTION);
    w = _t_newr(res,WHICH_RECEPTOR);
    _t_news(w,ROLE,TIME_HEARER);
    __r_make_addr(w,ACTUAL_RECEPTOR,r->addr);
    res = _t_newr(bindings,RESOLUTION);
    w = _t_newr(res,WHICH_PROCESS);
    _t_news(w,GOAL,REQUEST_HANDLER);

    // @todo bleah, this should be a better proc, at least with a SIGNAL_REF
    // or something.
    T *noop = _t_new_root(NOOP);
    _t_newi(noop,TEST_INT_SYMBOL,314);
    Process proc = _r_define_process(r,noop,"do nothing","long desc...",NULL);
    _t_news(w,ACTUAL_PROCESS,proc);

    _o_initiate(r,time,tell_time,bindings);

    spec_is_str_equal(_td(r,r->q->active->context->run_tree),"(RUN_TREE (process:do nothing (process:REQUEST (TO_ADDRESS (RECEPTOR_ADDR:4)) (ASPECT_IDENT:DEFAULT_ASPECT) (CARRIER:tell_time) (CLOCK_TELL_TIME) (RESPONSE_CARRIER:tell_time))) (PARAMS))");

    _test_reduce_signals(r);

    // we need a better indicator of success, but this at least shows that pending signals and
    // pending responses created should have been cleaned up.
    spec_is_str_equal(_td(r,r->pending_signals),"(PENDING_SIGNALS)");
    spec_is_str_equal(_td(r,r->pending_responses),"(PENDING_RESPONSES)");

    debug_disable(D_SIGNALS);

    Xaddr x = {TICK,1};
    T* tick = _r_get_instance(r,x);

    // clone the initial tick for later comparison
    tick = _t_clone(tick);

    // "run" the receptor and verify that the tick instance keeps getting updated
    // we wait a second first so that the first update will be a second later
    sleepms(1005);
    pthread_t thread;
    int rc = 0;
    rc = pthread_create(&thread,0,___clock_thread,r);
    if (rc){
        raise_error("ERROR; return code from pthread_create() is %d\n", rc);
    }

    sleepms(1);
    T* ntick = _r_get_instance(r,x);
    int p[] = {2,3,TREE_PATH_TERMINATOR};
    spec_is_equal(*(int *)_t_surface(_t_get(ntick,p)),
                  1+*(int *)_t_surface(_t_get(tick,p))  // should be 1 second later!
                  );

    __r_kill(r);

    void *status;
    rc = pthread_join(thread, &status);

    if (rc) {
        raise_error("ERROR; return code from pthread_join() is %d\n", rc);
    }

    debug_disable(D_SIGNALS);
    _t_free(tick);
    _r_free(r);
}

void testReceptor() {
    testReceptorCreate();
    testReceptorAddRemoveExpectation();
    testReceptorSignal();
    testReceptorSignalDeliver();
    testReceptorResponseDeliver();
    testReceptorEndCondition();
    testReceptorExpectation();
    testReceptorDef();
    testReceptorDefMatch();
    testReceptorInstances();
    testReceptorSerialize();
    testReceptorNums();
    testReceptorEdgeStream();
    testReceptorClock();
}
