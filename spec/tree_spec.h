/**
 * @file tree_spec.h
 * @copyright Copyright (C) 2013-2015, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 * @ingroup tests
 */

#include "../src/ceptr.h"
#include "../src/receptor.h"
#include "http_example.h"


void testCreateTreeNodes() {
    /* test the creation of trees and the various function that give access to created data elements
       and basic tree structure navigation
     */
    T *t, *t1, *t2, *t3, *t4, *t5;

    t = _t_new(0,TEST_STR_SYMBOL,"hello",6);
    spec_is_long_equal(_t_size(t),(long)6);
    spec_is_equal(_t_children(t),0);
    spec_is_str_equal((char *)_t_surface(t),"hello");
    spec_is_ptr_equal(_t_parent(t),NULL);
    spec_is_ptr_equal(_t_root(t),t);
    spec_is_ptr_equal(_t_child(t,1),NULL);
    spec_is_true(semeq(_t_symbol(t),TEST_STR_SYMBOL));

    t1 = _t_new(t,TEST_STR_SYMBOL,"t1",3);
    spec_is_ptr_equal(_t_parent(t1),t);
    spec_is_equal(_t_children(t),1);
    spec_is_ptr_equal(_t_child(t,1),t1);
    spec_is_ptr_equal(_t_root(t1),t);

    t2 = _t_new(t,TEST_STR_SYMBOL,"t2",3);
    spec_is_ptr_equal(_t_parent(t2),t);
    spec_is_equal(_t_children(t),2);
    spec_is_ptr_equal(_t_child(t,2),t2);
    spec_is_ptr_equal(_t_root(t2),t);

    Symbol s99 = {0,0,99};

    t3 = _t_newi(t,s99,101);
    spec_is_ptr_equal(_t_parent(t3),t);
    spec_is_equal(_t_children(t),3);
    spec_is_equal(*(int *)_t_surface(_t_child(t,3)),101);

    spec_is_ptr_equal(_t_next_sibling(t1),t2);
    spec_is_ptr_equal(_t_next_sibling(t),NULL);
    spec_is_ptr_equal(_t_next_sibling(t2),t3);
    spec_is_ptr_equal(_t_next_sibling(t3),NULL);

    t4 = _t_new_root(TEST_TREE_SYMBOL);
    _t_add(t,t4);
    spec_is_equal(_t_children(t),4);
    spec_is_ptr_equal(_t_child(t,4),t4);

    t5 = _t_newr(t4,TEST_TREE_SYMBOL2);
    spec_is_ptr_equal(_t_parent(t5),t4);
    spec_is_long_equal(_t_size(t5),(long)0);
    spec_is_symbol_equal(0,_t_symbol(t5),TEST_TREE_SYMBOL2);

    _t_detach_by_ptr(t,t3);
    _t_free(t3);  // detatch doesn't free the memory of the removed node
    spec_is_equal(_t_children(t),3);
    spec_is_ptr_equal(_t_child(t,3),t4);
    spec_is_ptr_equal(_t_child(t,2),t2);

    _t_free(t);

    float f = 3.1415;
    T *tf = _t_new(0,TEST_FLOAT_SYMBOL,&f,sizeof(float));

    spec_is_str_equal(t2s(tf),"(TEST_FLOAT_SYMBOL:3.141500)");
    _t_free(tf);
}

void testTreeNewReceptor() {
    //! [testTreeNewReceptor]

    // @fixme this is really a bogus test, because why would a TEST_INT have a receptor as a child?
    // we should make this a rational test

    T *t = _t_newi(0,TEST_INT_SYMBOL,0);
    Receptor *r = _r_new(TEST_RECEPTOR_SYMBOL);
    T *tr = _t_new_receptor(t,TEST_RECEPTOR_SYMBOL,r);

    spec_is_ptr_equal(_t_surface(tr),r);
    spec_is_true(!(tr->context.flags & TFLAG_ALLOCATED));

    spec_is_str_equal(t2s(t),"(TEST_INT_SYMBOL:0 (TEST_RECEPTOR_SYMBOL:{(TEST_RECEPTOR_SYMBOL (DEFINITIONS (STRUCTURES) (SYMBOLS) (PROCESSES) (PROTOCOLS) (SCAPES) (ASPECTS)) (FLUX (DEFAULT_ASPECT (EXPECTATIONS) (SIGNALS))) (RECEPTOR_STATE) (PENDING_SIGNALS) (PENDING_RESPONSES))}))");

    _t_free(t); // note, no need to free the receptor explicitly, as _t_free knows about it
    //! [testTreeNewReceptor]
}

void testTreeScape() {
    //! [testTreeScape]
    Scape *s = _s_new(TEST_INT_SYMBOL,TEST_STR_SYMBOL);
    T *ts = _t_new_scape(0,TEST_ALPHABETIZE_SCAPE_SYMBOL,s);

    spec_is_true(ts->context.flags &= TFLAG_SURFACE_IS_SCAPE);
    spec_is_ptr_equal(_t_surface(ts),s);
    spec_is_str_equal(t2s(ts),"(TEST_ALPHABETIZE_SCAPE_SYMBOL:key TEST_INT_SYMBOL,data TEST_STR_SYMBOL)");

    _t_free(ts); // note, no need to free the scape explicitly, as _t_free knows about it
    //! [testTreeScape]
}

void testTreeStream() {
    //! [testTreeStream]
    FILE *stream;
    char buffer[] = "line1\nline2\n";
    stream = fmemopen(buffer, strlen (buffer), "r+");

    Stream *s = _st_new_unix_stream(stream,0);
    T *ts = _t_new_stream(0,TEST_STREAM_SYMBOL,s);

    spec_is_true(ts->context.flags & TFLAG_SURFACE_IS_STREAM);
    spec_is_true(ts->context.flags & TFLAG_REFERENCE);
    spec_is_ptr_equal(_t_surface(ts),s);
    char *x = t2s(ts);
    x[19]=0; // chop off the actual address cus that changes all the time
    spec_is_str_equal(x,"(TEST_STREAM_SYMBOL");

    _t_free(ts);
    // note, for now we must handle all stream deallocation manually because _t_new_stream
    // always makes the symbols as references to the Stream object
    _st_free(s);

    //! [testTreeStream]
}

void testTreeOrthogonal() {
    T *t = _t_newi(0,TEST_INT_SYMBOL,1234);
    T *t2 = _t_newi(0,TEST_INT_SYMBOL2,99);
    T *o = _t_newt(t,TEST_TREE_SYMBOL,t2);

    spec_is_str_equal(t2s(t),"(TEST_INT_SYMBOL:1234 (TEST_TREE_SYMBOL:{(TEST_INT_SYMBOL2:99)}))");

    T *tc = _t_clone(t);
    spec_is_str_equal(t2s(tc),"(TEST_INT_SYMBOL:1234 (TEST_TREE_SYMBOL:{(TEST_INT_SYMBOL2:99)}))");

    _t_free(t);
    _t_free(tc);
}

void testTreeRealloc() {
    T *ts[12];
    T *t = _t_new(0,TEST_STR_SYMBOL,"t",2);
    char tname[3];
    int i;
    tname[0] = 't';
    tname[2] = 0;
    for (i=0;i<12;i++){
	tname[1] = 'a'+i;
	ts[i] = _t_new(t,TEST_STR_SYMBOL,tname,3);
    }
    spec_is_str_equal((char *)_t_surface(ts[11]),"tl");
    _t_free(t);
}

void testTreePathGet() {
    //! [testTreePathGet]
    T *t = _makeTestHTTPRequestTree(); // GET /groups/5/users.json?sort_by=last_name?page=2 HTTP/1.0

    int p0[] = {TREE_PATH_TERMINATOR};
    int p1[] = {1,TREE_PATH_TERMINATOR};
    int p2[] = {2,TREE_PATH_TERMINATOR};
    int p3[] = {3,TREE_PATH_TERMINATOR};
    int p33[] = {3,3,TREE_PATH_TERMINATOR};
    int p331[] = {3,3,1,TREE_PATH_TERMINATOR};
    int p3312[] = {3,3,1,2,TREE_PATH_TERMINATOR};
    int p33122[] = {3,3,1,2,2,TREE_PATH_TERMINATOR};
    int p311[] = {3,1,1,TREE_PATH_TERMINATOR};

    spec_is_ptr_equal(_t_get(t,p0),t);
    spec_is_symbol_equal(0,_t_symbol(_t_get(t,p1)),HTTP_REQUEST_VERSION);
    spec_is_symbol_equal(0,_t_symbol(_t_get(t,p2)),HTTP_REQUEST_METHOD);
    spec_is_symbol_equal(0,_t_symbol(_t_get(t,p3)),HTTP_REQUEST_PATH);
    spec_is_symbol_equal(0,_t_symbol(_t_get(t,p33)),HTTP_REQUEST_PATH_QUERY);
    spec_is_symbol_equal(0,_t_symbol(_t_get(t,p331)),HTTP_REQUEST_PATH_QUERY_PARAMS);
    spec_is_symbol_equal(0,_t_symbol(_t_get(t,p3312)),HTTP_REQUEST_PATH_QUERY_PARAM);
    spec_is_symbol_equal(0,_t_symbol(_t_get(t,p33122)),PARAM_VALUE);
    spec_is_symbol_equal(0,_t_symbol(_t_get(t,p311)),HTTP_REQUEST_PATH_SEGMENT);

    spec_is_str_equal(_t2s(&test_HTTP_defs,_t_getv(t,3,3,1,2,TREE_PATH_TERMINATOR)),"(HTTP_REQUEST_PATH_QUERY_PARAM (PARAM_KEY:page) (PARAM_VALUE:2))");

    //  _t_get returns null if tree doesn't have a node at the given path
    p311[2] = 3;
    spec_is_ptr_equal(_t_get(t,p311),NULL);

    _t_free(t);
    //! [testTreePathGet]
}

void testTreePathGetSurface() {
    //! [testTreePathGetSurface]
    T *t = _makeTestHTTPRequestTree(); // GET /groups/5/users.json?sort_by=last_name?page=2 HTTP/1.0

    int p1[] = {3,1,1,TREE_PATH_TERMINATOR};
    int p2[] = {3,1,2,TREE_PATH_TERMINATOR};
    int p3[] = {3,3,1,2,2,TREE_PATH_TERMINATOR};

    spec_is_str_equal((char *)_t_get_surface(t,p1),"groups");
    spec_is_str_equal((char *)_t_get_surface(t,p2),"5");
    spec_is_str_equal((char *)_t_get_surface(t,p3),"2");

    // make a test tree with the HTTP request tree as an orthogonal tree
    T *tt = _t_newt(0,TEST_TREE_SYMBOL,t);
    int po[] = {0,TREE_PATH_TERMINATOR};
    int po1[] = {0,3,1,1,TREE_PATH_TERMINATOR};

    // using 0 in the path should "dive"into the orthogonal tree
    T *x =_t_get(tt,po);
    spec_is_ptr_equal(x,t);
    spec_is_str_equal((char *)_t_get_surface(tt,po1),"groups");

    _t_free(tt);
    //! [testTreePathGetSurface]
}

void testTreePathGetPath() {
    //! [testTreePathGetPath]
    T *t = _makeTestHTTPRequestTree(); // GET /groups/5/users.json?sort_by=last_name?page=2 HTTP/1.0
    int p0[] = {TREE_PATH_TERMINATOR};
    int p1[] = {1,TREE_PATH_TERMINATOR};
    int p2[] = {3,1,TREE_PATH_TERMINATOR};
    int p3[] = {3,1,1,TREE_PATH_TERMINATOR};
    int *path;

    path = _t_get_path(_t_get(t,p0));
    spec_is_path_equal(path,p0);
    free(path);
    path = _t_get_path(_t_get(t,p1));
    spec_is_path_equal(path,p1);
    free(path);
    path = _t_get_path(_t_get(t,p3));
    spec_is_path_equal(path,p3);
    free(path);
    path = _t_get_path(_t_get(t,p2));
    spec_is_path_equal(path,p2);
    free(path);

    _t_free(t);
    //! [testTreePathGetPath]
 }

void testTreePathEqual() {
    //! [testTreePathEqual]
    int p0[] = {TREE_PATH_TERMINATOR};
    int p1[] = {1,TREE_PATH_TERMINATOR};
    int p2[] = {2,TREE_PATH_TERMINATOR};
    int p3[] = {2,1,1,TREE_PATH_TERMINATOR};
    int p4[] = {3,0,TREE_PATH_TERMINATOR};
    int p5[] = {3,0,2,1,1,TREE_PATH_TERMINATOR};

    spec_is_true(_t_path_equal(p0,p0));
    spec_is_true(_t_path_equal(p1,p1));
    spec_is_true(_t_path_equal(p2,p2));
    spec_is_true(_t_path_equal(p3,p3));
    spec_is_true(!_t_path_equal(p0,p1));
    spec_is_true(!_t_path_equal(p2,p3));
    spec_is_true(!_t_path_equal(p4,p3));
    spec_is_true(!_t_path_equal(p0,p3));
    spec_is_true(!_t_path_equal(p4,p5));
    //! [testTreePathEqual]
}

void testTreePathDepth() {
    //! [testTreePathDepth]
    int p0[] = {TREE_PATH_TERMINATOR};
    int p1[] = {1,TREE_PATH_TERMINATOR};
    int p2[] = {3,0,TREE_PATH_TERMINATOR};
    int p3[] = {2,1,1,TREE_PATH_TERMINATOR};
    int p5[] = {3,0,2,1,1,TREE_PATH_TERMINATOR};

    spec_is_equal(_t_path_depth(p0),0);
    spec_is_equal(_t_path_depth(p1),1);
    spec_is_equal(_t_path_depth(p2),2);
    spec_is_equal(_t_path_depth(p3),3);
    spec_is_equal(_t_path_depth(p5),5);
    //! [testTreePathDepth]
}

void testTreePathCopy() {
    //! [testTreePathCopy]
    int pp[10];
    int p5[] = {3,0,2,1,1,TREE_PATH_TERMINATOR};
    _t_pathcpy(pp,p5);

    spec_is_path_equal(pp,p5);
    //! [testTreePathCopy]
}

void testTreePathSprint() {
    //! [testTreePathSprint]
    char buf[255];
    int p5[] = {3,0,2,1,1,TREE_PATH_TERMINATOR};

    spec_is_str_equal(_t_sprint_path(p5,buf),"/3/0/2/1/1");
    //! [testTreePathSprint]
}

void testTreeNodeIndex() {
    //! [testTreeNodeIndex]
    T *t = _makeTestHTTPRequestTree(); // GET /groups/5/users.json?sort_by=last_name?page=2 HTTP/1.0
    spec_is_equal(_t_node_index(_t_child(t,1)),1);
    spec_is_equal(_t_node_index(_t_child(t,2)),2);
    spec_is_equal(_t_node_index(_t_child(t,3)),3);
    spec_is_equal(_t_node_index(t),0);

   _t_free(t);
    //! [testTreeNodeIndex]
}

void testTreeClone() {
    //! [testTreeClone]
    T *t = _makeTestHTTPRequestTree();
    T *c = _t_clone(t);

    spec_is_true(t!=c);
    spec_is_equal(_t_children(c),_t_children(t));

    char buf1[2000];
    char buf2[2000];
    __t_dump(&test_HTTP_defs,c,0,buf1);
    __t_dump(&test_HTTP_defs,t,0,buf2);

    spec_is_str_equal(buf1,buf2);

    spec_is_equal((int)_t_size(t),(int)_t_size(c)); // test cloning of 0 size items (i.e. roots)

    _t_free(t);
    _t_free(c);

    //! [testTreeClone]
}

void testTreeReplace() {
    //! [testTreeReplace]
    T *t = _makeTestHTTPRequestTree(); // GET /groups/5/users.json?sort_by=last_name?page=2 HTTP/1.0

    // replace the version with a new version
    T *t_version = _t_newr(0,HTTP_REQUEST_VERSION);
    _t_newi(t_version,VERSION_MAJOR,1);
    _t_newi(t_version,VERSION_MINOR,1);

    _t_replace(t,1,t_version);
    int p[] = {1,TREE_PATH_TERMINATOR};
    spec_is_str_equal(_t2s(&test_HTTP_defs,_t_get(t,p)),"(HTTP_REQUEST_VERSION (VERSION_MAJOR:1) (VERSION_MINOR:1))");

    _t_free(t);
    //! [testTreeReplace]
}

void testTreeSwap() {
    //! [testTreeSwap]
    T *t = _makeTestHTTPRequestTree(); // GET /groups/5/users.json?sort_by=last_name?page=2 HTTP/1.0

    // replace the version with a new version
    T *t_version = _t_newr(0,HTTP_REQUEST_VERSION);
    _t_newi(t_version,VERSION_MAJOR,1);
    _t_newi(t_version,VERSION_MINOR,1);

    T *s = _t_swap(t,1,t_version);
    spec_is_str_equal(_t2s(&test_HTTP_defs,s),"(HTTP_REQUEST_VERSION (VERSION_MAJOR:1) (VERSION_MINOR:0))");
    spec_is_ptr_equal(_t_parent(s),NULL);

    _t_free(s);
    _t_free(t);
    //! [testTreeSwap]
}

void testTreeInsertAt() {
    //! [testTreeInsertAt]
    T *t = _makeTestHTTPRequestTree(); // GET /groups/5/users.json?sort_by=last_name?page=2 HTTP/1.0
    int p[] = {3,1,2,TREE_PATH_TERMINATOR};
    T *c = _t_new(0,HTTP_REQUEST_PATH_SEGMENT,"a",2);
    _t_insert_at(t,p,c);
    char buf[2000];
    p[2] = TREE_PATH_TERMINATOR;
    c = _t_get(t,p);
    __t_dump(&test_HTTP_defs,c,0,buf);
    spec_is_str_equal(buf,"(HTTP_REQUEST_PATH_SEGMENTS (HTTP_REQUEST_PATH_SEGMENT:groups) (HTTP_REQUEST_PATH_SEGMENT:a) (HTTP_REQUEST_PATH_SEGMENT:5))");

    _t_free(t);
    //! [testTreeInsertAt]

    t = _t_new_root(ASCII_CHARS);
    p[0] = 1;
    p[1] = TREE_PATH_TERMINATOR;
    _t_insert_at(t,p,_t_newi(0,ASCII_CHAR,'x'));

    spec_is_str_equal(t2s(t),"(ASCII_CHARS (ASCII_CHAR:'x'))");
    _t_free(t);
}

void testTreeMorph() {
    //! [testTreeMorph]
    T *x = _t_newi(0,TEST_INT_SYMBOL,123);
    T *z = _t_new(0,TEST_STR_SYMBOL,"fish",5);

    _t_morph(x,z);
    spec_is_str_equal(t2s(x),"(TEST_STR_SYMBOL:fish)");

    _t_free(x);
    _t_free(z);
    //! [testTreeMorph]
}

void testTreeMorphLowLevel() {
    //! [testTreeMorphLowLevel]
    T *x = _t_new(0,TEST_STR_SYMBOL,"fish",5);
    int i = 789;

    __t_morph(x,TEST_INT_SYMBOL,&i,sizeof(int),0);
    spec_is_str_equal(t2s(x),"(TEST_INT_SYMBOL:789)");

    _t_free(x);
    //! [testTreeMorphLowLevel]
}

void testTreeDetach() {
    //! [testTreeDetach]
    T *t = _makeTestHTTPRequestTree(); // GET /groups/5/users.json?sort_by=last_name?page=2 HTTP/1.0
    char buf[2000];
    int p1[] = {1,TREE_PATH_TERMINATOR};

    // remove the version from the tree
    spec_is_symbol_equal(0,_t_symbol(_t_get(t,p1)),HTTP_REQUEST_VERSION);
    T *t_version = _t_detach_by_idx(t,1);
    spec_is_symbol_equal(0,_t_symbol(_t_get(t,p1)),HTTP_REQUEST_METHOD);

    // detached nodes shouldn't have a parent
    spec_is_ptr_equal(_t_parent(t_version),NULL);

    _t_free(t);
    _t_free(t_version);
    //! [testTreeDetach]
}

void testTreeHash() {
    //! [testTreeHash]
    T *t = _makeTestHTTPRequestTree(); // GET /groups/5/users.json?sort_by=last_name?page=2 HTTP/1.0
    TreeHash h = _t_hash(test_HTTP_symbols,test_HTTP_structures,t);

    // test that changing a symbol changes the hash
    t->contents.symbol.id++;
    spec_is_true(!_t_hash_equal(h,_t_hash(test_HTTP_symbols,test_HTTP_structures,t)));
    t->contents.symbol.id--;

    // test that changing a surface changes the hash
    int p[] = {1,2,TREE_PATH_TERMINATOR};
    T *v = _t_get(t,p);
    int orig_version = *(int *)&v->contents.surface;
    *(int *)&v->contents.surface = orig_version + 1;
    spec_is_true(!_t_hash_equal(h,_t_hash(test_HTTP_symbols,test_HTTP_structures,t)));
    *(int *)&v->contents.surface = orig_version; // change value back

    // test that changing child order changes the hash
    T *t_version = _t_detach_by_idx(t,1);
    _t_add(t,t_version);
    spec_is_true(!_t_hash_equal(h,_t_hash(test_HTTP_symbols,test_HTTP_structures,t)));

    _t_free(t);
    //! [testTreeHash]
}

void testUUID() {
    spec_is_long_equal(sizeof(UUIDt),16); //128 bits
    UUIDt u = __uuid_gen();

    struct timespec c;
    clock_gettime(CLOCK_MONOTONIC, &c);
    uint64_t t = ((c.tv_sec * (1000000)) + (c.tv_nsec / 1000));

    // time should be right about now, i.e. within X ms
    spec_is_long_equal(u.time,t);

    // @todo something else for the other bits of the UUID.
}

void testTreeSerialize() {
    //! [testTreeSerialize]
    char buf[2000] = {0};
    char buf1[2000] = {0};
    T *t1,*t = _makeTestHTTPRequestTree(); // GET /groups/5/users.json?sort_by=last_name?page=2 HTTP/1.0
    __t_dump(&test_HTTP_defs,t,0,buf);

    size_t l;
    void *surface,*s;
    G_d = &test_HTTP_defs;
    _t_serialize(&test_HTTP_defs,t,&surface,&l);
    s = surface;
    t1 = _t_unserialize(&test_HTTP_defs,&surface,&l,0);
    __t_dump(&test_HTTP_defs,t1,0,buf1);

    spec_is_str_equal(buf1,buf);

    _t_free(t);
    _t_free(t1);
    free(s);

    //! [testTreeSerialize]
}

void testTreeJSON() {
    //! [testTreeJSON]
    char buf[5000] = {0};
    T *t = _makeTestHTTPRequestTree(); // GET /groups/5/users.json?sort_by=last_name?page=2 HTTP/1.0
    _t2json(&test_HTTP_defs,t,INDENT,buf);

    spec_is_str_equal(buf,"{ \"symbol\":{ \"context\":255,\"id\":17 },\"type\":\"HTTP_REQUEST_V09\",\"name\":\"HTTP_REQUEST\",\"children\":[\n   { \"symbol\":{ \"context\":255,\"id\":16 },\"type\":\"VERSION\",\"name\":\"HTTP_REQUEST_VERSION\",\"children\":[\n      { \"symbol\":{ \"context\":255,\"id\":14 },\"type\":\"INTEGER\",\"name\":\"VERSION_MAJOR\",\"surface\":1},\n      { \"symbol\":{ \"context\":255,\"id\":15 },\"type\":\"INTEGER\",\"name\":\"VERSION_MINOR\",\"surface\":0}]},\n   { \"symbol\":{ \"context\":255,\"id\":2 },\"type\":\"CSTRING\",\"name\":\"HTTP_REQUEST_METHOD\",\"surface\":\"GET\"},\n   { \"symbol\":{ \"context\":255,\"id\":13 },\"type\":\"URI\",\"name\":\"HTTP_REQUEST_PATH\",\"children\":[\n      { \"symbol\":{ \"context\":255,\"id\":3 },\"type\":\"LIST\",\"name\":\"HTTP_REQUEST_PATH_SEGMENTS\",\"children\":[\n         { \"symbol\":{ \"context\":255,\"id\":4 },\"type\":\"CSTRING\",\"name\":\"HTTP_REQUEST_PATH_SEGMENT\",\"surface\":\"groups\"},\n         { \"symbol\":{ \"context\":255,\"id\":4 },\"type\":\"CSTRING\",\"name\":\"HTTP_REQUEST_PATH_SEGMENT\",\"surface\":\"5\"}]},\n      { \"symbol\":{ \"context\":255,\"id\":7 },\"type\":\"FILE_HANDLE\",\"name\":\"HTTP_REQUEST_PATH_FILE\",\"children\":[\n         { \"symbol\":{ \"context\":255,\"id\":5 },\"type\":\"CSTRING\",\"name\":\"FILE_NAME\",\"surface\":\"users\"},\n         { \"symbol\":{ \"context\":255,\"id\":6 },\"type\":\"CSTRING\",\"name\":\"FILE_EXTENSION\",\"surface\":\"json\"}]},\n      { \"symbol\":{ \"context\":255,\"id\":8 },\"type\":\"LIST\",\"name\":\"HTTP_REQUEST_PATH_QUERY\",\"children\":[\n         { \"symbol\":{ \"context\":255,\"id\":9 },\"type\":\"LIST\",\"name\":\"HTTP_REQUEST_PATH_QUERY_PARAMS\",\"children\":[\n            { \"symbol\":{ \"context\":255,\"id\":12 },\"type\":\"KEY_VALUE_PARAM\",\"name\":\"HTTP_REQUEST_PATH_QUERY_PARAM\",\"children\":[\n               { \"symbol\":{ \"context\":255,\"id\":10 },\"type\":\"CSTRING\",\"name\":\"PARAM_KEY\",\"surface\":\"sort_by\"},\n               { \"symbol\":{ \"context\":255,\"id\":11 },\"type\":\"CSTRING\",\"name\":\"PARAM_VALUE\",\"surface\":\"last_name\"}]},\n            { \"symbol\":{ \"context\":255,\"id\":12 },\"type\":\"KEY_VALUE_PARAM\",\"name\":\"HTTP_REQUEST_PATH_QUERY_PARAM\",\"children\":[\n               { \"symbol\":{ \"context\":255,\"id\":10 },\"type\":\"CSTRING\",\"name\":\"PARAM_KEY\",\"surface\":\"page\"},\n               { \"symbol\":{ \"context\":255,\"id\":11 },\"type\":\"CSTRING\",\"name\":\"PARAM_VALUE\",\"surface\":\"2\"}]}]}]}]}]}");

    wjson(&test_HTTP_defs,t,"httpreq",0);
    char *stxs = "/%<HTTP_REQUEST_PATH_SEGMENTS:HTTP_REQUEST_PATH_SEGMENTS,HTTP_REQUEST_PATH_FILE>";
    T *stx;
    stx = parseSemtrex(&test_HTTP_defs,stxs);
    wjson(&test_HTTP_defs,stx,"httpreq",1);

    T *r;
    if (_t_matchr(stx,t,&r)) {
	wjson(&test_HTTP_defs,r,"httpreq",2);
	_t_free(r);
    }

    _t_free(stx);
    _t_free(t);
    //! [testTreeJSON]
}

void testProcessHTML() {
    T *t = parseHTML("<html><body><div id=\"314\" class=\"contents\">Hello world<img src=\"test.png\"/></div></body></html>");
    spec_is_str_equal(_t2s(&test_HTTP_defs,t),"(HTML_HTML (HTML_ATTRIBUTES) (HTML_CONTENT (HTML_BODY (HTML_ATTRIBUTES) (HTML_CONTENT (HTML_DIV (HTML_ATTRIBUTES (HTML_ATTRIBUTE (PARAM_KEY:id) (PARAM_VALUE:314)) (HTML_ATTRIBUTE (PARAM_KEY:class) (PARAM_VALUE:contents))) (HTML_CONTENT (HTML_TEXT:Hello world) (HTML_IMG (HTML_ATTRIBUTES (HTML_ATTRIBUTE (PARAM_KEY:src) (PARAM_VALUE:test.png))) (HTML_CONTENT))))))))");
    _t_free(t);
}

void testTree() {
    _setup_HTTPDefs();

    testCreateTreeNodes();
    testTreeNewReceptor();
    testTreeScape();
    testTreeStream();
    testTreeOrthogonal();
    testTreeRealloc();
    testTreeNodeIndex();
    testTreePathGet();
    testTreePathGetSurface();
    testTreePathGetPath();
    testTreePathEqual();
    testTreePathDepth();
    testTreePathCopy();
    testTreePathSprint();
    testTreeClone();
    testTreeReplace();
    testTreeSwap();
    testTreeInsertAt();
    testTreeMorph();
    testTreeMorphLowLevel();
    testTreeDetach();
    testTreeHash();
    testUUID();
    testTreeSerialize();
    testTreeJSON();
    testProcessHTML();

    _cleanup_HTTPDefs();
}
