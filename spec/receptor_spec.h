/**
 * @file receptor_spec.h
 * @copyright Copyright (C) 2013-2015, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 * @ingroup tests
 */

#include "../src/ceptr.h"
#include "../src/receptor.h"
#include "../src/def.h"
#include "http_example.h"

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

    spec_is_str_equal(t2s(r->root),"(TEST_RECEPTOR_SYMBOL (DEFINITIONS (STRUCTURES) (SYMBOLS) (PROCESSES) (PROTOCOLS) (SCAPES)) (ASPECTS) (FLUX (ASPECT:1 (LISTENERS) (SIGNALS))))");

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
    _r_add_listener(r,DEFAULT_ASPECT,TEST_INT_SYMBOL,s,a);

    T *l = _t_child(__r_get_listeners(r,DEFAULT_ASPECT),1);      // listener should have been added as first child of listeners
    spec_is_symbol_equal(r,_t_symbol(l),LISTENER);
    spec_is_sem_equal(*(Symbol *)_t_surface(l),TEST_INT_SYMBOL); // carrier should be TEST_INT_SYMBOL
    spec_is_ptr_equal(_t_child(l,1),s);       // our expectation semtrex should be first child of the listener
    spec_is_ptr_equal(_t_child(l,2),a);       // our action code tree should be the second child of the listener

    _r_free(r);
}

void testReceptorSignal() {
    Receptor *r = _r_new(TEST_RECEPTOR_SYMBOL);
    T *signal_contents = _t_newi(0,TEST_INT_SYMBOL,314);
    Xaddr f = {RECEPTOR_XADDR,3};  // DUMMY XADDR
    Xaddr t = {RECEPTOR_XADDR,4};  // DUMMY XADDR

    T *s = __r_make_signal(f,t,DEFAULT_ASPECT,signal_contents);

    spec_is_symbol_equal(r,_t_symbol(s),SIGNAL);

    T *envelope = _t_child(s,1);
    spec_is_symbol_equal(r,_t_symbol(envelope),ENVELOPE);
    T *body = _t_child(s,2);
    spec_is_symbol_equal(r,_t_symbol(body),BODY);
    T *contents = (T*)_t_surface(body);
    spec_is_ptr_equal(signal_contents,contents);

    //@todo symbol check??? FROM_RECEPTOR_XADDR???
    Xaddr from = *(Xaddr *)_t_surface(_t_child(envelope,1));
    spec_is_xaddr_equal(r,from,f);
    Xaddr to = *(Xaddr *)_t_surface(_t_child(envelope,2));
    spec_is_xaddr_equal(r,to,t);
    Aspect a = *(Aspect *)_t_surface(_t_child(envelope,3));
    spec_is_equal(a,DEFAULT_ASPECT);

    char buf[2000];
    __t_dump(0,s,0,buf);
    spec_is_str_equal(buf,"(SIGNAL (ENVELOPE (RECEPTOR_XADDR:RECEPTOR_XADDR.3) (RECEPTOR_XADDR:RECEPTOR_XADDR.4) (ASPECT:1)) (BODY:{(TEST_INT_SYMBOL:314)}))");

    _t_free(s);
    _r_free(r);
}

void testReceptorAction() {
    //! [testReceptorAction]
    Receptor *r = _r_new(TEST_RECEPTOR_SYMBOL);

    // The signal is an HTTP request
    T *signal_contents = _makeTestHTTPRequestTree(); // GET /groups/5/users.json?sort_by=last_name?page=2 HTTP/1.0

    Xaddr f = {RECEPTOR_XADDR,3};  // DUMMY XADDR
    Xaddr t = {RECEPTOR_XADDR,4};  // DUMMY XADDR

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

    Process p = _makeTestHTTPResponseProcess(r);

    T *act = _t_newp(0,ACTION,p);

    _r_add_listener(r,DEFAULT_ASPECT,HTTP_REQUEST,expect,act);

    result = _r_deliver(r,signal);
    //    spec_is_symbol_equal(r,_t_symbol(result),HTTP_RESPONSE);

    // the result should be signal tree with the matched PATH_SEGMENT returned as the body
    spec_is_str_equal(_td(r,result),"(SIGNALS (SIGNAL (ENVELOPE (RECEPTOR_XADDR:RECEPTOR_XADDR.4) (RECEPTOR_XADDR:RECEPTOR_XADDR.3) (ASPECT:1)) (BODY:{(HTTP_RESPONSE (HTTP_RESPONSE_CONTENT_TYPE:CeptrSymbol/HTTP_REQUEST_PATH_SEGMENT) (HTTP_REQUEST_PATH_SEGMENT:groups))})))");
    _t_free(result);
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
    T *input = _t_new_root(INPUT_SIGNATURE);
    T *output = _t_new_root(OUTPUT_SIGNATURE);
    Process p = _r_code_process(r,code,"power","takes the mathematical power of the two params",input,output);
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

Receptor *_makePingProtocolReceptor(Symbol *pingP) {
    Receptor *r;
    r = _r_new(TEST_RECEPTOR_SYMBOL);

    Symbol ping = _r_declare_symbol(r,BOOLEAN,"ping");
    *pingP = ping;

    // define a ping protocol with two roles and two interactions
    T *ps = r->defs.protocols;
    T *p = _t_newr(ps,PROTOCOL);
    T *roles = _t_newr(p,ROLES);
    _t_new(roles,ROLE,"server",7);
    _t_new(roles,ROLE,"client",7);
    T *interactions = _t_newr(p,INTERACTIONS);

    // initial ping request interaction
    T *i,*s,*e;
    i = _t_newr(interactions,INTERACTION);
    _t_new(i,STEP,"ping",5);
    _t_new(i,FROM_ROLE,"client",7);
    _t_new(i,TO_ROLE,"server",7);
    _t_news(i,CARRIER,ping); // input carrier
    _t_news(i,CARRIER,ping); // output carrier
    e = _t_newr(i,EXPECTATION);
    T *req = _sl(e,ping);

    T *ping_resp = _t_new_root(RESPOND);
    _t_newi(ping_resp,ping,1);
    T *input = _t_new_root(INPUT);
    T *output = _t_new_root(OUTPUT_SIGNATURE);
    Process proc = _r_code_process(r,ping_resp,"send ping response","long desc...",input,output);

    _t_newp(i,ACTION,proc);

    s = _t_newr(i,RESPONSE_STEPS);
    _t_new(s,STEP,"ping_response",14);

    // ping response interaction
    i = _t_newr(interactions,INTERACTION);
    _t_new(i,STEP,"ping_response",14);
    _t_new(i,FROM_ROLE,"server",7);
    _t_new(i,TO_ROLE,"client",7);
    _t_news(i,CARRIER,ping); // input carrier
    _t_news(i,CARRIER,ping); // output carrier
    e = _t_newr(i,EXPECTATION);
    _sl(e,ping);
    //    s = _t_newr(i,RESPONSE_STEPS);

    T *aspects = _t_child(r->root,2);
    T *a = _t_newr(aspects,ASPECT_DEF);
    _t_newi(a,ASPECT_TYPE,EXTERNAL_ASPECT);
    _t_news(a,CARRIER,ping);
    _t_news(a,CARRIER,ping);
    wjson(&r->defs,p,"protocol",-1);
    return r;
}

void testReceptorProtocol() {
    //! [testReceptorProtocol]
    Symbol ping;
    Receptor *r = _makePingProtocolReceptor(&ping);

    _r_install_protocol(r,1,"server",DEFAULT_ASPECT);

    char *d = _td(r,r->root);

    spec_is_str_equal(d,"(TEST_RECEPTOR_SYMBOL (DEFINITIONS (STRUCTURES) (SYMBOLS (SYMBOL_DECLARATION (SYMBOL_LABEL:ping) (SYMBOL_STRUCTURE:BOOLEAN))) (PROCESSES (PROCESS_CODING (PROCESS_NAME:send ping response) (PROCESS_INTENTION:long desc...) (process:RESPOND (ping:1)) (INPUT) (OUTPUT_SIGNATURE))) (PROTOCOLS (PROTOCOL (ROLES (ROLE:server) (ROLE:client)) (INTERACTIONS (INTERACTION (STEP:ping) (FROM_ROLE:client) (TO_ROLE:server) (CARRIER:ping) (CARRIER:ping) (EXPECTATION (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:ping))) (ACTION:send ping response) (RESPONSE_STEPS (STEP:ping_response))) (INTERACTION (STEP:ping_response) (FROM_ROLE:server) (TO_ROLE:client) (CARRIER:ping) (CARRIER:ping) (EXPECTATION (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:ping))))))) (SCAPES)) (ASPECTS (ASPECT_DEF (ASPECT_TYPE:0) (CARRIER:ping) (CARRIER:ping))) (FLUX (ASPECT:1 (LISTENERS (LISTENER:ping (EXPECTATION (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:ping))) (ACTION:send ping response))) (SIGNALS))))");

    // delivering a fake signal should return a ping
    Xaddr f = {RECEPTOR_XADDR,3};  // DUMMY XADDR
    Xaddr t = {RECEPTOR_XADDR,4};  // DUMMY XADDR
    T *signal = __r_make_signal(f,t,DEFAULT_ASPECT,_t_newi(0,ping,0));

    d = _td(r,signal);
    spec_is_str_equal(d,"(SIGNAL (ENVELOPE (RECEPTOR_XADDR:RECEPTOR_XADDR.3) (RECEPTOR_XADDR:RECEPTOR_XADDR.4) (ASPECT:1)) (BODY:{(ping:0)}))");

    T *result = _r_deliver(r,signal);
    d = _td(r,result);
    spec_is_str_equal(d,"(SIGNALS (SIGNAL (ENVELOPE (RECEPTOR_XADDR:RECEPTOR_XADDR.4) (RECEPTOR_XADDR:RECEPTOR_XADDR.3) (ASPECT:1)) (BODY:{(ping:1)})))");
    _t_free(result);
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

    void *surface;
    size_t length;
    char buf[2000];
    char buf1[2000];

    _r_serialize(r,&surface,&length);

    spec_is_long_equal(length,250);
    spec_is_long_equal(*(size_t *)surface,250);

    Receptor *r1 = _r_unserialize(surface);

    // check that the structures look the same by comparing a string dump of the two
    // receptors
    __t_dump(&r->defs,r->root,0,buf);
    __t_dump(&r1->defs,r1->root,0,buf1);
    spec_is_str_equal(buf1,buf);

    // check that the unserialized receptor has the labels loaded into the label table
    int *path = labelGet(&r1->table,"latitude");
    int p[] = {2,1,TREE_PATH_TERMINATOR};
    spec_is_path_equal(path,p);
    spec_is_sem_equal(_r_get_symbol_by_label(r1,"latitude"),lat);
    spec_is_sem_equal(_r_get_structure_by_label(r1,"latlong"),latlong);

    // check that the unserialized receptor has all the instances loaded into the instance store too
    T *t1 = _r_get_instance(r1,x);
    buf[0] = buf1[0] = 0;
    __t_dump(&r->defs,t,0,buf);
    __t_dump(&r1->defs,t1,0,buf1);
    spec_is_str_equal(buf1,buf);

    free(surface);
    _r_free(r);
    _r_free(r1);
    //! [testReceptorSerialize]
}

void testReceptor() {
    _setup_HTTPDefs();
    testReceptorCreate();
    testReceptorAddListener();
    testReceptorSignal();
    testReceptorAction();
    testReceptorDef();
    testReceptorDefMatch();
    testReceptorProtocol();
    testReceptorInstanceNew();
    //    testReceptorSerialize();
    _cleanup_HTTPDefs();
}
