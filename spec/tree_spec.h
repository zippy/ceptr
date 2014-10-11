/**
 * @file tree_spec.h
 * @copyright Copyright (C) 2013-2014, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 * @ingroup tests
 */

#include "../src/ceptr.h"
#include "../src/receptor.h"
#include "http_example.h"

void testCreateTreeNodes() {
    /* test the creation of trees and the various function that give access to created data elements
       and basic tree structure navigation
     */
    Tnode *t, *t1, *t2, *t3, *t4, *t5;

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
}

void testTreeNewReceptor() {
    //! [testTreeNewReceptor]
    Tnode *t = _t_newi(0,TEST_INT_SYMBOL,0);
    Receptor *r = _r_new(TEST_RECEPTOR_SYMBOL);
    Tnode *tr = _t_new_receptor(t,TEST_RECEPTOR_SYMBOL,r);

    spec_is_ptr_equal(_t_surface(tr),r);

    char buf[2000];
    __t_dump(0,t,0,buf);
    spec_is_str_equal(buf," (TEST_INT_SYMBOL:0 (TEST_RECEPTOR_SYMBOL:{ (TEST_RECEPTOR_SYMBOL (DEFINITIONS (STRUCTURES) (SYMBOLS) (PROCESSES) (PROTOCOLS) (SCAPES)) (ASPECTS) (FLUX (ASPECT:1 (LISTENERS) (SIGNALS))))}))");

    _t_free(t); // note, no need to free the receptor explicitly, as _t_free knows about it
    //! [testTreeNewReceptor]
}

void testTreeNewScape() {
    //! [testTreeNewScape]
    Scape *s = _s_new(TEST_INT_SYMBOL,TEST_STR_SYMBOL);
    Tnode *ts = _t_new_scape(0,TEST_ALPHABETIZE_SCAPE_SYMBOL,s);

    spec_is_ptr_equal(_t_surface(ts),s);

    char buf[2000];
    __t_dump(0,ts,0,buf);
    spec_is_str_equal(buf," (TEST_ALPHABETIZE_SCAPE_SYMBOL:key TEST_INT_SYMBOL,data TEST_STR_SYMBOL)");

    _t_free(ts); // note, no need to free the scape explicitly, as _t_free knows about it
    //! [testTreeNewScape]
}

void testTreeOrthogonal() {
    Tnode *t = _t_newi(0,TEST_INT_SYMBOL,1234);
    Tnode *t2 = _t_newi(0,TEST_INT_SYMBOL2,99);
    Tnode *o = _t_newt(t,TEST_TREE_SYMBOL,t2);
    char buf[2000];
    __t_dump(0,t,0,buf);
    spec_is_str_equal(buf," (TEST_INT_SYMBOL:1234 (TEST_TREE_SYMBOL:{ (TEST_INT_SYMBOL2:99)}))");
    _t_free(t);
}

void testTreeRealloc() {
    Tnode *ts[12];
    Tnode *t = _t_new(0,TEST_STR_SYMBOL,"t",2);
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
    Tnode *t = _makeTestHTTPRequestTree(); // GET /groups/5/users.json?sort_by=last_name?page=2 HTTP/1.0

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

    //  _t_get returns null if tree doesn't have a node at the given path
    p311[2] = 3;
    spec_is_ptr_equal(_t_get(t,p311),NULL);

    _t_free(t);
    //! [testTreePathGet]
}

void testTreePathGetSurface() {
    //! [testTreePathGetSurface]
    Tnode *t = _makeTestHTTPRequestTree(); // GET /groups/5/users.json?sort_by=last_name?page=2 HTTP/1.0

    int p1[] = {3,1,1,TREE_PATH_TERMINATOR};
    int p2[] = {3,1,2,TREE_PATH_TERMINATOR};
    int p3[] = {3,3,1,2,2,TREE_PATH_TERMINATOR};

    spec_is_str_equal((char *)_t_get_surface(t,p1),"groups");
    spec_is_str_equal((char *)_t_get_surface(t,p2),"5");
    spec_is_str_equal((char *)_t_get_surface(t,p3),"2");

    // make a test tree with the HTTP request tree as an orthogonal tree
    Tnode *tt = _t_newt(0,TEST_TREE_SYMBOL,t);
    int po[] = {0,TREE_PATH_TERMINATOR};
    int po1[] = {0,3,1,1,TREE_PATH_TERMINATOR};

    // using 0 in the path should "dive" into the orthogonal tree
    Tnode *x =_t_get(tt,po);
    spec_is_ptr_equal(x,t);
    spec_is_str_equal((char *)_t_get_surface(tt,po1),"groups");

    _t_free(tt);
    //! [testTreePathGetSurface]
}

void testTreePathGetPath() {
    //! [testTreePathGetPath]
    Tnode *t = _makeTestHTTPRequestTree(); // GET /groups/5/users.json?sort_by=last_name?page=2 HTTP/1.0
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
    Tnode *t = _makeTestHTTPRequestTree(); // GET /groups/5/users.json?sort_by=last_name?page=2 HTTP/1.0
    spec_is_equal(_t_node_index(_t_child(t,1)),1);
    spec_is_equal(_t_node_index(_t_child(t,2)),2);
    spec_is_equal(_t_node_index(_t_child(t,3)),3);
    spec_is_equal(_t_node_index(t),0);

   _t_free(t);
    //! [testTreeNodeIndex]
}

void testTreeClone() {
    //! [testTreeClone]
    Tnode *t = _makeTestHTTPRequestTree();
    Tnode *c = _t_clone(t);

    spec_is_true(t!=c);
    spec_is_equal(_t_children(c),_t_children(t));

    char buf1[2000];
    char buf2[2000];
    __t_dump(&test_HTTP_defs,c,0,buf1);
    __t_dump(&test_HTTP_defs,t,0,buf2);

    spec_is_str_equal(buf1,buf2);

    _t_free(t);
    _t_free(c);
    //! [testTreeClone]
}

void testTreeReplace() {
    //! [testTreeReplace]
    Tnode *t = _makeTestHTTPRequestTree(); // GET /groups/5/users.json?sort_by=last_name?page=2 HTTP/1.0

    // replace the version with a new version
    Tnode *t_version = _t_newr(0,HTTP_REQUEST_VERSION);
    _t_newi(t_version,VERSION_MAJOR,1);
    _t_newi(t_version,VERSION_MINOR,1);

    int p[] = {1,2,TREE_PATH_TERMINATOR};
    spec_is_equal(*(int *)_t_get_surface(t,p),0);
    _t_replace(t,1,t_version);
    spec_is_equal(*(int *)_t_get_surface(t,p),1);

    _t_free(t);
    //! [testTreeReplace]
}

void testTreeMorph() {
    //! [testTreeMorph]
    char buf[2000];
    Tnode *x = _t_newi(0,TEST_INT_SYMBOL,123);
    Tnode *z = _t_new(0,TEST_STR_SYMBOL,"fish",5);

    _t_morph(x,z);
    __t_dump(0,x,0,buf);
    spec_is_str_equal(buf," (TEST_STR_SYMBOL:fish)");

    _t_free(x);
    _t_free(z);
    //! [testTreeMorph]
}

void testTreeMorphLowLevel() {
    //! [testTreeMorphLowLevel]
    char buf[2000];
    Tnode *x = _t_new(0,TEST_STR_SYMBOL,"fish",5);
    int i = 789;

    __t_morph(x,TEST_INT_SYMBOL,&i,sizeof(int),0);
    __t_dump(0,x,0,buf);
    spec_is_str_equal(buf," (TEST_INT_SYMBOL:789)");

    _t_free(x);
    //! [testTreeMorphLowLevel]
}

void testTreeDetach() {
    //! [testTreeDetach]
    Tnode *t = _makeTestHTTPRequestTree(); // GET /groups/5/users.json?sort_by=last_name?page=2 HTTP/1.0
    char buf[2000];
    int p1[] = {1,TREE_PATH_TERMINATOR};

    // remove the version from the tree
    spec_is_symbol_equal(0,_t_symbol(_t_get(t,p1)),HTTP_REQUEST_VERSION);
    Tnode *t_version = _t_detach_by_idx(t,1);
    spec_is_symbol_equal(0,_t_symbol(_t_get(t,p1)),HTTP_REQUEST_METHOD);

    // detached nodes shouldn't have a parent
    spec_is_ptr_equal(_t_parent(t_version),NULL);

    _t_free(t);
    _t_free(t_version);
    //! [testTreeDetach]
}

void testTreeHash() {
    //! [testTreeHash]
    Tnode *t = _makeTestHTTPRequestTree(); // GET /groups/5/users.json?sort_by=last_name?page=2 HTTP/1.0
    TreeHash h = _t_hash(test_HTTP_symbols,test_HTTP_structures,t);

    // test that changing a symbol changes the hash
    t->contents.symbol.id++;
    spec_is_true(!_t_hash_equal(h,_t_hash(test_HTTP_symbols,test_HTTP_structures,t)));
    t->contents.symbol.id--;

    // test that changing a surface changes the hash
    int p[] = {1,2,TREE_PATH_TERMINATOR};
    Tnode *v = _t_get(t,p);
    int orig_version = *(int *)&v->contents.surface;
    *(int *)&v->contents.surface = orig_version + 1;
    spec_is_true(!_t_hash_equal(h,_t_hash(test_HTTP_symbols,test_HTTP_structures,t)));
    *(int *)&v->contents.surface = orig_version; // change value back

    // test that changing child order changes the hash
    Tnode *t_version = _t_detach_by_idx(t,1);
    _t_add(t,t_version);
    spec_is_true(!_t_hash_equal(h,_t_hash(test_HTTP_symbols,test_HTTP_structures,t)));

    _t_free(t);
    //! [testTreeHash]
}

void testTree() {
    _setup_HTTPDefs();
    testCreateTreeNodes();
    testTreeNewReceptor();
    testTreeNewScape();
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
    testTreeMorph();
    testTreeMorphLowLevel();
    testTreeDetach();
    testTreeHash();
    _cleanup_HTTPDefs();
}
