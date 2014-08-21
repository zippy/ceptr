#include "../src/ceptr.h"
#include "../src/process.h"


void testRunTree() {
    Tnode *act = _t_new_root(ACTION);
    Tnode *resp = _t_newi(act,RESPOND,0);
    Tnode *t = _t_newi(resp,TEST_SYMBOL,0);
    Tnode *p1 = _t_newi(0,TEST_SYMBOL,5413);
    Tnode *p2 = _t_newi(0,TEST_SYMBOL2,3145);

    Tnode *r = _p_make_run_tree(act,2,p1,p2);

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
    _p_reduce(t);
    char buf[2000];
    __t_dump(0,_t_child(t,1),0,buf);
    spec_is_str_equal(buf," (TEST_SYMBOL2 (TEST_SYMBOL:314))");
    _t_free(t);
}

//TODO: when interpolating from a match, how do we handle non-leaf interpollations, i.e. where do you hook children onto?

void testProcess() {
    testRunTree();
    testRunTreeReduce();
}
