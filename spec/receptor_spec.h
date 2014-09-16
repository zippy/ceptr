/**
 * @file receptor_spec.h
 * @copyright Copyright (C) 2013-2014, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
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

    Tnode *t;

    // test that the symbols, structures & process trees are set up correctly
    t = _t_child(r->root,1);
    spec_is_symbol_equal(r,_t_symbol(r->defs.structures),STRUCTURES);
    spec_is_ptr_equal(t,r->defs.structures);
    t = _t_child(r->root,2);
    spec_is_symbol_equal(r,_t_symbol(r->defs.symbols),SYMBOLS);
    spec_is_ptr_equal(t,r->defs.symbols);
    t = _t_child(r->root,3);
    spec_is_symbol_equal(r,_t_symbol(r->defs.processes),PROCESSES);
    spec_is_ptr_equal(t,r->defs.processes);

    // test that listeners and signals are set up correctly on the default aspect
    t = __r_get_listeners(r,DEFAULT_ASPECT);
    spec_is_symbol_equal(r,_t_symbol(t),LISTENERS);
    t = __r_get_signals(r,DEFAULT_ASPECT);
    spec_is_symbol_equal(r,_t_symbol(t),SIGNALS);

    // test that the flux is set up correctly
    t = _t_child(r->root,4);
    spec_is_symbol_equal(r,_t_symbol(r->flux),FLUX);
    spec_is_ptr_equal(t,r->flux);
    t = _t_child(r->flux,1);
    spec_is_symbol_equal(r,_t_symbol(t),ASPECT);
    spec_is_equal(*(int *)_t_surface(t),DEFAULT_ASPECT);

    _r_free(r);
    //! [testReceptorCreate]
}

void testReceptorAddListener() {
    Receptor *r;
    r = _r_new(TEST_RECEPTOR_SYMBOL);

    // test that you can add a listener to a receptor's aspect
    Tnode *s = _t_new_root(EXPECTATION);
    _t_newi(s,SEMTREX_SYMBOL_LITERAL,0);
    Tnode *a = _t_newi(0,ACTION,NULL_PROCESS);
    _r_add_listener(r,DEFAULT_ASPECT,TEST_INT_SYMBOL,s,a);

    Tnode *l = _t_child(__r_get_listeners(r,DEFAULT_ASPECT),1);      // listener should have been added as first child of listeners
    spec_is_symbol_equal(r,_t_symbol(l),LISTENER);
    spec_is_equal(*(int *)_t_surface(l),TEST_INT_SYMBOL); // carrier should be TEST_INT_SYMBOL
    spec_is_ptr_equal(_t_child(l,1),s);       // our expectation semtrex should be first child of the listener
    spec_is_ptr_equal(_t_child(l,2),a);       // our action code tree should be the second child of the listener

    _r_free(r);
}

void testReceptorSignal() {
    Receptor *r = _r_new(TEST_RECEPTOR_SYMBOL);
    Tnode *signal = _t_newi(0,TEST_INT_SYMBOL,314);
    _r_send(r,r,DEFAULT_ASPECT,signal);

    // the first node on the default aspect signals should be the signal
    Tnode *s = _t_child(__r_get_signals(r,DEFAULT_ASPECT),1);
    spec_is_symbol_equal(r,_t_symbol(s),SIGNAL);
    Tnode *t = (Tnode *)_t_surface(s);  // whose surface should be the contents
    spec_is_equal(*(int *)_t_surface(t),314);
    spec_is_ptr_equal(signal,t);  // at some point this should probably fail, because we should have cloned the signal, not added it directly

    _r_free(r);
}

void testReceptorAction() {
    //! [testReceptorAction]
    Receptor *r = _r_new(TEST_RECEPTOR_SYMBOL);

    // The signal is an HTTP request
    Tnode *signal = _makeTestHTTPRequestTree(); // GET /groups/5/users.json?sort_by=last_name?page=2 HTTP/1.0

    // our expectation should match on the first path segment
    Tnode *expect = _t_new_root(EXPECTATION);
    Tnode *req = _t_newi(expect,SEMTREX_SYMBOL_LITERAL,TSYM_HTTP_REQUEST);
    Tnode *seq = _t_newr(req,SEMTREX_SEQUENCE);
    _t_newr(seq,SEMTREX_SYMBOL_ANY);  // skips the Version
    _t_newr(seq,SEMTREX_SYMBOL_ANY);  // skips over the Method
    Tnode *path = _t_newi(seq,SEMTREX_SYMBOL_LITERAL,TSYM_HTTP_REQUEST_PATH);
    Tnode *segs = _t_newi(path,SEMTREX_SYMBOL_LITERAL,TSYM_HTTP_REQUEST_PATH_SEGMENTS);
    Tnode *g = _t_newi(segs,SEMTREX_GROUP,TSYM_HTTP_REQUEST_PATH_SEGMENT);
    _t_newi(g,SEMTREX_SYMBOL_LITERAL,TSYM_HTTP_REQUEST_PATH_SEGMENT);

    Tnode *result;
    int matched;
    // make sure our expectation semtrex actually matches the signal
    spec_is_true(matched = _t_matchr(req,signal,&result));
    Tnode *m = _t_get_match(result,TSYM_HTTP_REQUEST_PATH_SEGMENT);
    char buf[2000];
    __t_dump(test_HTTP_symbols,m,0,buf);
    spec_is_str_equal(buf," (SEMTREX_MATCH:HTTP_REQUEST_PATH_SEGMENT (SEMTREX_MATCHED_PATH:/3/1/1) (SEMTREX_MATCH_SIBLINGS_COUNT:1))");
    if (result) {
	_t_free(result);
    }

    // the action simply responds back with the method that was originally sent
    // this test should be made more real... but for now it responds back with a ping
    // like message that is what the first path segment was

    Tnode *resp = _t_new_root(RESPOND);
    Tnode *n = _t_newr(resp,INTERPOLATE_FROM_MATCH);
    Tnode *http_resp = _t_newr(n,TSYM_HTTP_RESPONSE);
    _t_new(http_resp,TSYM_HTTP_RESPONSE_CONTENT_TYPE,"CeptrSymbol/HTTP_REQUEST_PATH_SEGMENT",38);
    _t_newi(http_resp,INTERPOLATE_SYMBOL,TSYM_HTTP_REQUEST_PATH_SEGMENT);
    _t_newi(n,PARAM_REF,1);
    _t_newi(n,PARAM_REF,2);

    Tnode *input = _t_new_root(INPUT_SIGNATURE);
    Tnode *output = _t_new_root(OUTPUT_SIGNATURE);
    Process p = _r_code_process(r,resp,"code path ping","respond with the first segment of the code path",input,output);

    Tnode *act = _t_newp(0,ACTION,p);

    _r_add_listener(r,DEFAULT_ASPECT,TSYM_HTTP_REQUEST,expect,act);

    Tnode *s = _r_send(r,r,DEFAULT_ASPECT,signal);
    spec_is_symbol_equal(r,_t_symbol(s),SIGNAL);

    // the result should be signal tree with the matched PATH_SEGMENT returned as the body
    result = _t_child(_t_child(s,1),1);
    __t_dump(test_HTTP_symbols,result,0,buf);
    spec_is_str_equal(buf," (HTTP_RESPONSE (HTTP_RESPONSE_CONTENT_TYPE:CeptrSymbol/HTTP_REQUEST_PATH_SEGMENT) (HTTP_REQUEST_PATH_SEGMENT:groups))");

    _r_free(r);
    //! [testReceptorAction]
}

void testReceptorDef() {
    Receptor *r = _r_new(TEST_RECEPTOR_SYMBOL);

    Symbol lat = _r_declare_symbol(r,FLOAT,"latitude");
    Symbol lon = _r_declare_symbol(r,FLOAT,"longitude");
    Tnode *def;

    spec_is_structure_equal(r,__r_get_symbol_structure(r,lat),FLOAT);

    spec_is_str_equal((char *)_t_surface(_t_child(def = _t_child(r->defs.symbols,lat),2)),"latitude");
    spec_is_str_equal((char *)_t_surface(_t_child(_t_child(r->defs.symbols,lon),2)),"longitude");

    int *path = labelGet(&r->table,"latitude");
    spec_is_ptr_equal(_t_get(r->root,path),def);
    spec_is_equal(_r_get_symbol_by_label(r,"latitude"),lat);

    Structure latlong = _r_define_structure(r,"latlong",2,lat,lon);

    def = _t_child(r->defs.structures,latlong);
    Tnode *l = _t_child(def,1);
    spec_is_str_equal((char *)_t_surface(l),"latlong");

    path = labelGet(&r->table,"latlong");
    spec_is_ptr_equal(_t_get(r->root,path),def);

    spec_is_structure_equal(r,_r_get_structure_by_label(r,"latlong"),latlong);
    spec_is_long_equal(__r_get_symbol_size(r,lat,0),sizeof(float));

    Symbol house_loc = _r_declare_symbol(r,latlong,"house location");
    spec_is_equal(__r_get_symbol_structure(r,house_loc),latlong);
    spec_is_long_equal(__r_get_symbol_size(r,house_loc,0),sizeof(float)*2);

    Symbol name = _r_declare_symbol(r,CSTRING,"name");
    spec_is_long_equal(__r_get_symbol_size(r,name,"zippy"),(long)6);

    Structure namedhouse = _r_define_structure(r,"namedhouse",2,house_loc,name);

    Symbol home = _r_declare_symbol(r,namedhouse,"home");
    char surface[] ={1,2,3,4,5,6,7,8,'b','o','b','b','y',0};
    spec_is_long_equal(__r_get_symbol_size(r,home,surface),sizeof(float)*2+6);

    Tnode *code = _t_new_root(ACTION);
    Tnode *input = _t_new_root(INPUT_SIGNATURE);
    Tnode *output = _t_new_root(OUTPUT_SIGNATURE);
    Process p = _r_code_process(r,code,"power","takes the mathematical power of the two params",input,output);
    spec_is_equal(_t_children(r->defs.processes),-p);

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
    Receptor *r = _r_new(TEST_RECEPTOR_SYMBOL);
    Symbol lat,lon,house_loc;
    Structure latlong;
    defineHouseLocation(r,&lat,&lon,&latlong,&house_loc);

    Tnode *t = _t_new_root(house_loc);
    float x = 99.0;
    Tnode *t_lat = _t_new(t,lat,&x,sizeof(x));
    Tnode *t_lon = _t_new(t,lon,&x,sizeof(x));

    Tnode *stx = _r_build_def_semtrex(r,house_loc,0);
    char buf[2000];
    spec_is_str_equal(_dump_semtrex(stx,buf),"/(3/1,2)");
    __t_dump(r->defs.symbols,stx,0,buf);
    spec_is_str_equal(buf," (SEMTREX_SYMBOL_LITERAL:house location (SEMTREX_SEQUENCE (SEMTREX_SYMBOL_LITERAL:latitude) (SEMTREX_SYMBOL_LITERAL:longitude)))");

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
}

void testReceptorInstanceNew() {
    //! [testReceptorInstancesNew]
    Receptor *r = _r_new(TEST_RECEPTOR_SYMBOL);
    Symbol lat,lon,house_loc;
    Structure latlong;
    defineHouseLocation(r,&lat,&lon,&latlong,&house_loc);

    // create a house location tree
    Tnode *t = _t_new_root(house_loc);
    float ll[] = {132.5,92.3};
    Tnode *t_lat = _t_new(t,lat,&ll[0],sizeof(float));
    Tnode *t_lon = _t_new(t,lon,&ll[1],sizeof(float));

    Xaddr x = _r_new_instance(r,t);
    spec_is_equal(x.addr,1);
    spec_is_equal(x.symbol,house_loc);

    float *ill;
    Tnode *i = _r_get_instance(r,x);

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
    Tnode *t = _t_new_root(house_loc);
    float ll[] = {132.5,92.3};
    Tnode *t_lat = _t_new(t,lat,&ll[0],sizeof(float));
    Tnode *t_lon = _t_new(t,lon,&ll[1],sizeof(float));

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
    __t_dump(r->defs.symbols,r->root,0,buf);
    __t_dump(r1->defs.symbols,r1->root,0,buf1);
    spec_is_str_equal(buf1,buf);

    // check that the unserialized receptor has the labels loaded into the label table
    int *path = labelGet(&r1->table,"latitude");
    int p[] = {2,1,TREE_PATH_TERMINATOR};
    spec_is_path_equal(path,p);
    spec_is_equal(_r_get_symbol_by_label(r1,"latitude"),lat);
    spec_is_equal(_r_get_structure_by_label(r1,"latlong"),latlong);

    // check that the unserialized receptor has all the instances loaded into the instance store too
    Tnode *t1 = _r_get_instance(r1,x);
    buf[0] = buf1[0] = 0;
    __t_dump(r->defs.symbols,t,0,buf);
    __t_dump(r1->defs.symbols,t1,0,buf1);
    spec_is_str_equal(buf1,buf);

    free(surface);
    _r_free(r);
    _r_free(r1);
    //! [testReceptorSerialize]
}

void testSemtrexDump() {
    Tnode *s = _makeTestSemtrex1();
    char buf[2000];
    spec_is_str_equal(_dump_semtrex(s,buf),"/(TEST_STR_SYMBOL/(1/(11/111)),2,3)");

    _t_free(s);

    // /TEST_STR_SYMBOL/{.*,{.}},4  <- a more complicated group semtrex
    s = _t_newi(0,SEMTREX_SYMBOL_LITERAL,TEST_STR_SYMBOL);
    Tnode *ss = _t_newi(s,SEMTREX_SEQUENCE,0);
    Tnode *sg = _t_newi(ss,SEMTREX_GROUP,TEST_GROUP_SYMBOL1);
    Tnode *ss2 = _t_newi(sg,SEMTREX_SEQUENCE,0);
    Tnode *st = _t_newi(ss2,SEMTREX_ZERO_OR_MORE,0);
    _t_newi(st,SEMTREX_SYMBOL_ANY,0);
    Tnode *sg2 = _t_newi(ss2,SEMTREX_GROUP,TEST_GROUP_SYMBOL2);
    _t_newi(sg2,SEMTREX_SYMBOL_ANY,0);
    Tnode *s3 = _t_newi(ss,SEMTREX_SYMBOL_LITERAL,4);
    spec_is_str_equal(_dump_semtrex(s,buf),"/(TEST_STR_SYMBOL/{.*,{.}},4)");
    _t_free(s);

}

void testReceptor() {
    _setup_HTTPDefs();
    testReceptorCreate();
    testReceptorAddListener();
    testReceptorSignal();
    testReceptorAction();
    testReceptorDef();
    testReceptorDefMatch();
    testReceptorInstanceNew();
    //    testReceptorSerialize();
    testSemtrexDump();
    _cleanup_HTTPDefs();
}
