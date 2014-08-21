#include "../src/ceptr.h"
#include "../src/receptor.h"

void testReceptorCreate() {
    Receptor *r;
    r = _r_new();
    spec_is_symbol_equal(r,_t_symbol(r->root),RECEPTOR);

    Tnode *t;

    // test that the symbols and structures tress are set up correctly
    t = _t_child(r->root,1);
    spec_is_symbol_equal(r,_t_symbol(r->structures),STRUCTURES);
    spec_is_ptr_equal(t,r->structures);
    t = _t_child(r->root,2);
    spec_is_symbol_equal(r,_t_symbol(r->symbols),SYMBOLS);
    spec_is_ptr_equal(t,r->symbols);

    // test that listeners and signals are set up correctly on the default aspect
    t = __r_get_listeners(r,DEFAULT_ASPECT);
    spec_is_symbol_equal(r,_t_symbol(t),LISTENERS);
    t = __r_get_signals(r,DEFAULT_ASPECT);
    spec_is_symbol_equal(r,_t_symbol(t),SIGNALS);

    // test that the flux is set up correctly
    t = _t_child(r->root,3);
    spec_is_symbol_equal(r,_t_symbol(r->flux),FLUX);
    spec_is_ptr_equal(t,r->flux);
    t = _t_child(r->flux,1);
    spec_is_symbol_equal(r,_t_symbol(t),ASPECT);
    spec_is_equal(*(int *)_t_surface(t),DEFAULT_ASPECT);


    _r_free(r);
}

void testReceptorAddListener() {
    Receptor *r;
    r = _r_new();

    // test that you can add a listener to a receptor's aspect
    Tnode *s = _t_new_root(EXPECTATION);
    _t_newi(s,SEMTREX_SYMBOL_LITERAL,0);
    Tnode *a = _t_new_root(ACTION);
    _r_add_listener(r,DEFAULT_ASPECT,TEST_SYMBOL,s,a);

    Tnode *l = _t_child(__r_get_listeners(r,DEFAULT_ASPECT),1);      // listener should have been added as first child of listeners
    spec_is_symbol_equal(r,_t_symbol(l),LISTENER);
    spec_is_equal(*(int *)_t_surface(l),TEST_SYMBOL); // carrier should be TEST_SYMBOL
    spec_is_ptr_equal(_t_child(l,1),s);       // our expectation semtrex should be first child of the listener
    spec_is_ptr_equal(_t_child(l,2),a);       // our action code tree should be the second child of the listener

    _r_free(r);
}

void testReceptorSignal() {
    Receptor *r = _r_new();
    Tnode *signal = _t_newi(0,TEST_SYMBOL,314);
    _r_send(r,r,DEFAULT_ASPECT,signal);

    // the first node on the default aspect signals should be the signal
    Tnode *s = _t_child(__r_get_signals(r,DEFAULT_ASPECT),1);
    spec_is_symbol_equal(r,_t_symbol(s),SIGNAL);
    Tnode *t = (Tnode *)_t_surface(s);  // whose surface should be the contents
    spec_is_equal(*(int *)_t_surface(t),314);
    spec_is_ptr_equal(signal,t);  // at some point this should probably fail, because we should have cloned the signal, not added it directly

    _r_free(r);
}

void testRunTree() {
    Tnode *act = _t_new_root(ACTION);
    Tnode *resp = _t_newi(act,RESPOND,0);
    Tnode *t = _t_newi(resp,TEST_SYMBOL,0);
    Tnode *p1 = _t_newi(0,TEST_SYMBOL,5413);
    Tnode *p2 = _t_newi(0,TEST_SYMBOL2,3145);

    Tnode *r = _r_make_run_tree(act,2,p1,p2);

    spec_is_symbol_equal(0,_t_symbol(r),RUN_TREE);

    t = _t_child(r,1);  // first child should be clone of code
    spec_is_symbol_equal(0,_t_symbol(t),RESPOND);
    spec_is_true(t!=resp); // should be a clone

    Tnode *p = _t_child(r,2); //second child should be params
    spec_is_symbol_equal(0,_t_symbol(p),PARAMS);

    t = _t_child(p,1);
    spec_is_symbol_equal(0,_t_symbol(t),TEST_SYMBOL);
    spec_is_true(t!=p1);  //should be a clone

    t = _t_child(p,2);  // third child should be params
    spec_is_symbol_equal(0,_t_symbol(t),TEST_SYMBOL2);
    spec_is_true(t!=p2);  //should be a clone

    _t_free(act);
    _t_free(r);
    _t_free(p1);
    _t_free(p2);

}

void testRunTreeReduce() {
    Tnode *t = _t_new_root(RUN_TREE);
    // test INTERPOLATE_FROM_MATCH which takes two params, the stx-match and the tree it matched on
    Tnode *n = _t_newr(t,INTERPOLATE_FROM_MATCH);
    Tnode *t2 = _t_newi(n,TEST_SYMBOL2,0);
    _t_newi(t2,INTERPOLATE_SYMBOL,TEST_SYMBOL);

    // setup params
    Tnode *p = _t_newi(t,PARAMS,0);
    Tnode *p1 = _t_newi(p,SEMTREX_MATCH_RESULTS,0);
    Tnode *sm = _t_newi(p1,SEMTREX_MATCH,TEST_SYMBOL);
    int path[] = {TREE_PATH_TERMINATOR};
    _t_new(sm,TREE_PATH,path,2*sizeof(int));
    _t_newi(sm,SEMTREX_MATCH_SIBLINGS_COUNT,1);
    Tnode *p2 = _t_newi(p,TEST_SYMBOL,314);
    _r_reduce(t);
    char buf[2000];
    __t_dump(0,_t_child(t,1),0,buf);
    spec_is_str_equal(buf," (TEST_SYMBOL2 (TEST_SYMBOL:314))");
    _t_free(t);
}

//TODO: when interpolating from a match, how do we handle non-leaf interpollations, i.e. where do you hook children onto?

void testReceptorAction() {

    Receptor *r = _r_new();

    // The signal is a name with a child that is the first name
    Tnode *signal = _t_new_root(TEST_NAME_SYMBOL);
    _t_new(signal,TEST_FIRST_NAME_SYMBOL,"eric",5);

    // The listener just matches on signal TEST_NAME_TYPE type and responds with
    // converted tree to: /TEST_SYMBOL2/TEST_FIRST_NAME
    Tnode *expect = _t_new_root(EXPECTATION);
    Tnode *sa = _t_newi(expect,SEMTREX_SYMBOL_ANY,0);
    Tnode *sg = _t_newi(sa,SEMTREX_GROUP,TEST_FIRST_NAME_SYMBOL);
    _t_newi(sg,SEMTREX_SYMBOL_LITERAL,TEST_FIRST_NAME_SYMBOL);
    Tnode *act = _t_new_root(ACTION);
    Tnode *resp = _t_newi(act,RESPOND,0);
    Tnode *n = _t_newi(resp,INTERPOLATE_FROM_MATCH,0);
    Tnode *t = _t_newi(n,TEST_SYMBOL2,0);
    _t_newi(t,INTERPOLATE_SYMBOL,TEST_FIRST_NAME_SYMBOL);

    // confirm that the signal will match on our expectation
    spec_is_true(_t_match(sa,signal));

    _r_add_listener(r,DEFAULT_ASPECT,TEST_NAME_SYMBOL,expect,act);
    Tnode *s = _r_send(r,r,DEFAULT_ASPECT,signal);

    spec_is_symbol_equal(r,_t_symbol(s),SIGNAL);
    Tnode *result = _t_child(_t_child(s,1),1);

    // the result should be signal tree with  the matched TEST_SYMBOL value interpolated
    // in the right place
    spec_is_symbol_equal(r,_t_symbol(result),TEST_SYMBOL2);
    Tnode *r1 = _t_child(result,1);
    spec_is_symbol_equal(r,_t_symbol(r1),TEST_FIRST_NAME_SYMBOL);
    spec_is_str_equal((char *)_t_surface(r1),"eric");

    // TODO: a signal that has no matches should return a null result?
    signal = _t_newi(0,TEST_SYMBOL2,3141);
    result = _r_send(r,r,DEFAULT_ASPECT,signal);
    spec_is_ptr_equal(result,NULL);

    _r_free(r);
}

void testReceptorDef() {
    Receptor *r = _r_new();

    Symbol lat = _r_def_symbol(r,FLOAT,"latitude");
    Symbol lon = _r_def_symbol(r,FLOAT,"longitude");
    Tnode *def;

    spec_is_structure_equal(r,__r_get_symbol_structure(r,lat),FLOAT);

    spec_is_str_equal((char *)_t_surface(_t_child(def = _t_child(r->symbols,lat),2)),"latitude");
    spec_is_str_equal((char *)_t_surface(_t_child(_t_child(r->symbols,lon),2)),"longitude");

    int *path = labelGet(&r->table,"latitude");
    spec_is_ptr_equal(_t_get(r->root,path),def);
    spec_is_equal(_r_get_symbol_by_label(r,"latitude"),lat);

    Structure latlong = _r_def_structure(r,"latlong",2,lat,lon);

    def = _t_child(r->structures,latlong);
    spec_is_str_equal("latlong",(char *)_t_surface(def));

    path = labelGet(&r->table,"latlong");
    spec_is_ptr_equal(_t_get(r->root,path),def);

    spec_is_structure_equal(r,_r_get_structure_by_label(r,"latlong"),latlong);
    spec_is_long_equal(__r_get_symbol_size(r,lat,0),sizeof(float));

    Symbol house_loc = _r_def_symbol(r,latlong,"house location");
    spec_is_equal(__r_get_symbol_structure(r,house_loc),latlong);
    spec_is_long_equal(__r_get_symbol_size(r,house_loc,0),sizeof(float)*2);

    Symbol name = _r_def_symbol(r,CSTRING,"name");
    spec_is_long_equal(__r_get_symbol_size(r,name,"zippy"),(long)6);

    Structure namedhouse = _r_def_structure(r,"namedhouse",2,house_loc,name);

    Symbol home = _r_def_symbol(r,namedhouse,"home");
    char surface[] ={1,2,3,4,5,6,7,8,'b','o','b','b','y',0};
    spec_is_long_equal(__r_get_symbol_size(r,home,surface),sizeof(float)*2+6);

    _r_free(r);
}

void testReceptorInstances() {
    Receptor *r = _r_new();
    Symbol lat = _r_def_symbol(r,FLOAT,"latitude");
    Symbol lon = _r_def_symbol(r,FLOAT,"longitude");
    Structure latlong = _r_def_structure(r,"latlong",2,lat,lon);
    Symbol house_loc = _r_def_symbol(r,latlong,"house location");
    float ll[] = {132.5,92.3};
    Xaddr x = _r_new_instance(r,house_loc,ll);

    float *ill;
    Instance i = _r_get_instance(r,x);
    ill = (float *)i.surface;

    spec_is_float_equal(ill[0],ll[0]);
    spec_is_float_equal(ill[1],ll[1]);
    _r_free(r);
}

void testReceptorSerialize() {
    Receptor *r = _r_new();
    Symbol lat = _r_def_symbol(r,FLOAT,"latitude");
    Symbol lon = _r_def_symbol(r,FLOAT,"longitude");
    Structure latlong = _r_def_structure(r,"latlong",2,lat,lon);
    Symbol house_loc = _r_def_symbol(r,latlong,"house location");
    float ll[] = {132.5,92.3};
    Xaddr x = _r_new_instance(r,house_loc,ll);

    void *surface;
    size_t length;
    char buf[2000];
    char buf1[2000];

    _r_serialize(r,&surface,&length);

    spec_is_long_equal(length,(long)242);

    Receptor *r1 = _r_unserialize(surface,length);

    __t_dump(r,r->root,0,buf);
    __t_dump(r1,r1->root,0,buf1);

    //    puts(_td(r,r1->root));
    spec_is_str_equal(buf1,buf);

    int *path = labelGet(&r1->table,"latitude");
    int p[] = {2,1,TREE_PATH_TERMINATOR};
    spec_is_path_equal(path,p);
    spec_is_equal(_r_get_symbol_by_label(r1,"latitude"),lat);
    spec_is_equal(_r_get_structure_by_label(r1,"latlong"),latlong);

    free(surface);
    _r_free(r);
    _r_free(r1);

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
    testReceptorCreate();
    testReceptorAddListener();
    testReceptorSignal();
    testRunTree();
    testRunTreeReduce();
    testReceptorAction();
    testReceptorDef();
    testReceptorInstances();
    testReceptorSerialize();
    testSemtrexDump();

}
