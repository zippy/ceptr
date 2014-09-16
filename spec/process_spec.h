/**
 * @file process_spec.h
 * @copyright Copyright (C) 2013-2014, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 * @ingroup tests
 */

#include "../src/ceptr.h"
#include "../src/process.h"

void testRunTree() {
    Tnode *processes = _t_new_root(PROCESSES);
    Defs defs = {0,0,processes};
    Tnode *code,*input,*output,*t;
    char buf[2000];

    // a process that would look something like this in lisp:
    // (defun my_if (true_branch false_branch condition) (if (condition) (true_branch) (false_branch)))
    code = _t_new_root(IF);
    int pt1[] = {2,1,TREE_PATH_TERMINATOR};
    int pt2[] = {2,2,TREE_PATH_TERMINATOR};
    int pt3[] = {2,3,TREE_PATH_TERMINATOR};

    _t_new(code,PARAM_REF,pt3,sizeof(int)*4);
    _t_new(code,PARAM_REF,pt1,sizeof(int)*4);
    _t_new(code,PARAM_REF,pt2,sizeof(int)*4);
    input = _t_new_root(INPUT_SIGNATURE);
    _t_newi(input,SIGNATURE_STRUCTURE,TREE);
    _t_newi(input,SIGNATURE_STRUCTURE,TREE);
    _t_newi(input,SIGNATURE_STRUCTURE,BOOLEAN);

    output = _t_new_root(OUTPUT_SIGNATURE);
    Process p = _d_code_process(processes,code,"myif","a duplicate of the sys if process with params in different order",input,output);

    Tnode *p3 = _t_newi(0,TRUE_FALSE,1);
    Tnode *p1 = _t_newi(0,TEST_INT_SYMBOL,123);
    Tnode *p2 = _t_newi(0,TEST_INT_SYMBOL,321);

    Tnode *act = _t_newp(0,ACTION,p);

    Tnode *r = _p_make_run_tree(processes,act,3,p1,p2,p3);

    spec_is_symbol_equal(0,_t_symbol(r),RUN_TREE);

    t = _t_child(r,1);  // first child should be clone of code
    spec_is_equal(_t_symbol(t),IF);
    spec_is_true(t!=code);  //should be a clone

    Tnode *ps = _t_child(r,2); //second child should be params
    spec_is_symbol_equal(0,_t_symbol(ps),PARAMS);

    t = _t_child(ps,1);
    spec_is_symbol_equal(0,_t_symbol(t),TEST_INT_SYMBOL);
    spec_is_true(t!=p1);  //should be a clone

    t = _t_child(ps,2);  // third child should be params
    spec_is_symbol_equal(0,_t_symbol(t),TEST_INT_SYMBOL);
    spec_is_true(t!=p2);  //should be a clone


    t = _t_child(ps,3);  // third child should be params
    spec_is_symbol_equal(0,_t_symbol(t),TRUE_FALSE);
    spec_is_true(t!=p3);  //should be a clone

    _p_reduce(defs,r);
    __t_dump(0,_t_child(r,1),0,buf);
    spec_is_str_equal(buf," (TEST_INT_SYMBOL:123)");

    _t_free(act);
    _t_free(r);
    _t_free(processes);
    _t_free(p1);
    _t_free(p2);
    _t_free(p3);
}

Process _defIfEven(Tnode *processes) {
    Tnode *code,*input,*output;

    // a process that would look something like this in lisp:
    // (defun if_even (val true_branch false_branch) (if (eq (mod val 2 ) 0) (true_branch) (false_branch)))
    code = _t_new_root(IF);
    Tnode *eq = _t_newi(code,EQ_INT,0);
    Tnode *mod = _t_newi(eq,MOD_INT,0);
    int p1[] = {2,1,TREE_PATH_TERMINATOR};
    int p2[] = {2,2,TREE_PATH_TERMINATOR};
    int p3[] = {2,3,TREE_PATH_TERMINATOR};
    _t_new(mod,PARAM_REF,p1,sizeof(int)*4);
    _t_newi(mod,TEST_INT_SYMBOL,2);
    _t_newi(eq,TEST_INT_SYMBOL,0);
    _t_new(code,PARAM_REF,p2,sizeof(int)*4);
    _t_new(code,PARAM_REF,p3,sizeof(int)*4);
    input = _t_new_root(INPUT_SIGNATURE);
    _t_newi(input,SIGNATURE_STRUCTURE,INTEGER);
    _t_newi(input,SIGNATURE_STRUCTURE,TREE);
    _t_newi(input,SIGNATURE_STRUCTURE,TREE);
    output = _t_new_root(OUTPUT_SIGNATURE);

    return _d_code_process(processes,code,"if even","return 2nd child if even, third if not",input,output);

}

void testProcessReduceDefinedProcess() {
    Tnode *processes = _t_new_root(PROCESSES);
    Defs defs = {0,0,processes};
    char buf[2000];

    Process if_even = _defIfEven(processes);

    int p[] = {1,3,TREE_PATH_TERMINATOR};
    __t_dump(0,_t_get(processes,p),0,buf);
    spec_is_str_equal(buf," (process:IF (process:EQ_INT (process:MOD_INT (PARAM_REF:/2/1) (TEST_INT_SYMBOL:2)) (TEST_INT_SYMBOL:0)) (PARAM_REF:/2/2) (PARAM_REF:/2/3))");

    Tnode *t = _t_new_root(RUN_TREE);
    Tnode *n = _t_newr(t,if_even);
    _t_newi(n,TEST_INT_SYMBOL,99);
    _t_newi(n,TEST_INT_SYMBOL,123);
    _t_newi(n,TEST_INT_SYMBOL,124);

    spec_is_equal(__p_reduce(defs,t,n),noReductionErr);
    __t_dump(0,_t_child(t,1),0,buf);
    spec_is_str_equal(buf," (TEST_INT_SYMBOL:124)");

    _t_free(processes);
    _t_free(t);
}

void testProcessSignatureMatching() {
    Tnode *processes = _t_new_root(PROCESSES);
    Defs defs = {0,0,processes};
    Process if_even = _defIfEven(processes);

    Tnode *t = _t_new_root(RUN_TREE);
    Tnode *n = _t_newr(t,if_even);
    _t_new(n,TEST_STR_SYMBOL,"test",5);  // this should be an INTEGER!!
    _t_newi(n,TEST_INT_SYMBOL,123);
    _t_newi(n,TEST_INT_SYMBOL,124);
    spec_is_equal(__p_reduce(defs,t,n),badSignatureReductionErr);

    // too few params
    _t_free(_t_detach_by_idx(n,1));
    spec_is_equal(__p_reduce(defs,t,n),tooFewParamsReductionErr);

    // add too many params
    _t_newi(n,TEST_INT_SYMBOL,124);
    _t_newi(n,TEST_INT_SYMBOL,124);
    spec_is_equal(__p_reduce(defs,t,n),tooManyParamsReductionErr);

    _t_free(processes);
    _t_free(t);
}

void testProcessInterpolateMatch() {
    Defs defs;
    Tnode *t = _t_new_root(RUN_TREE);
    // test INTERPOLATE_FROM_MATCH which takes three params, the tree to interpolate, the stx-match and the tree it matched on
    Tnode *n = _t_newr(t,INTERPOLATE_FROM_MATCH);
    Tnode *p1 = _t_newi(n,TEST_INT_SYMBOL2,0);
    _t_newi(p1,INTERPOLATE_SYMBOL,TEST_INT_SYMBOL);
    Tnode *p2 = _t_newi(n,SEMTREX_MATCH_RESULTS,0);
    Tnode *sm = _t_newi(p2,SEMTREX_MATCH,TEST_INT_SYMBOL);
    int path[] = {TREE_PATH_TERMINATOR};
    _t_new(sm,SEMTREX_MATCHED_PATH,path,2*sizeof(int));
    _t_newi(sm,SEMTREX_MATCH_SIBLINGS_COUNT,1);
    Tnode *p3 = _t_newi(n,TEST_INT_SYMBOL,314);
    __p_reduce(defs,t,n);
    char buf[2000];
    __t_dump(0,_t_child(t,1),0,buf);
    spec_is_str_equal(buf," (TEST_INT_SYMBOL2:0 (TEST_INT_SYMBOL:314))");
    _t_free(t);
}

/// @todo when interpolating from a match, how do we handle non-leaf interpollations, i.e. where do you hook children onto?

void testProcessIf() {
    Defs defs;
    // test IF which takes three parameters, the condition, the true code tree and the false code tree
    Tnode *t = _t_new_root(RUN_TREE);
    Tnode *n = _t_newr(t,IF);
    Tnode *p1 = _t_newi(n,TRUE_FALSE,1);
    Tnode *p2 = _t_newi(n,TEST_INT_SYMBOL,99);
    Tnode *p3 = _t_newi(n,TEST_INT_SYMBOL,100);

    __p_reduce(defs,t,n);
    char buf[2000];
    __t_dump(0,_t_child(t,1),0,buf);
    spec_is_str_equal(buf," (TEST_INT_SYMBOL:99)");

    _t_free(t);
}

void testProcessIntMath() {
    Defs defs;
    char buf[2000];
    Tnode *t = _t_new_root(RUN_TREE);

    // test addition
    Tnode *n = _t_newr(t,ADD_INT);
    _t_newi(n,TEST_INT_SYMBOL,99);
    _t_newi(n,TEST_INT_SYMBOL,100);
    __p_reduce(defs,t,n);
    __t_dump(0,_t_child(t,1),0,buf);
    spec_is_str_equal(buf," (TEST_INT_SYMBOL:199)");

    // test subtraction
    n = _t_detach_by_idx(t,1);
    _t_free(n);
    n = _t_newr(t,SUB_INT);
    _t_newi(n,TEST_INT_SYMBOL,100);
    _t_newi(n,TEST_INT_SYMBOL,98);

    __p_reduce(defs,t,n);
    __t_dump(0,_t_child(t,1),0,buf);
    spec_is_str_equal(buf," (TEST_INT_SYMBOL:2)");

    // test multiplication
    n = _t_detach_by_idx(t,1);
    _t_free(n);
    n = _t_newr(t,MULT_INT);
    _t_newi(n,TEST_INT_SYMBOL,100);
    _t_newi(n,TEST_INT_SYMBOL,98);

    __p_reduce(defs,t,n);
    __t_dump(0,_t_child(t,1),0,buf);
    spec_is_str_equal(buf," (TEST_INT_SYMBOL:9800)");

    // test division
    n = _t_detach_by_idx(t,1);
    _t_free(n);
    n = _t_newr(t,DIV_INT);
    _t_newi(n,TEST_INT_SYMBOL,100);
    _t_newi(n,TEST_INT_SYMBOL,48);

    __p_reduce(defs,t,n);
    __t_dump(0,_t_child(t,1),0,buf);
    spec_is_str_equal(buf," (TEST_INT_SYMBOL:2)");

    // test modulo
    n = _t_detach_by_idx(t,1);
    _t_free(n);
    n = _t_newr(t,MOD_INT);
    _t_newi(n,TEST_INT_SYMBOL,100);
    _t_newi(n,TEST_INT_SYMBOL,2);

    __p_reduce(defs,t,n);
    __t_dump(0,_t_child(t,1),0,buf);
    spec_is_str_equal(buf," (TEST_INT_SYMBOL:0)");

    // test equals
    n = _t_detach_by_idx(t,1);
    _t_free(n);
    n = _t_newr(t,EQ_INT);
    _t_newi(n,TEST_INT_SYMBOL,100);
    _t_newi(n,TEST_INT_SYMBOL,2);

    __p_reduce(defs,t,n);
    __t_dump(0,_t_child(t,1),0,buf);
    spec_is_str_equal(buf," (TRUE_FALSE:0)");

    n = _t_detach_by_idx(t,1);
    _t_free(n);
    n = _t_newr(t,EQ_INT);
    _t_newi(n,TEST_INT_SYMBOL,100);
    _t_newi(n,TEST_INT_SYMBOL,100);

    __p_reduce(defs,t,n);
    __t_dump(0,_t_child(t,1),0,buf);
    spec_is_str_equal(buf," (TRUE_FALSE:1)");

    // test <
    n = _t_detach_by_idx(t,1);
    _t_free(n);
    n = _t_newr(t,LT_INT);
    _t_newi(n,TEST_INT_SYMBOL,2);
    _t_newi(n,TEST_INT_SYMBOL,100);

    __p_reduce(defs,t,n);
    __t_dump(0,_t_child(t,1),0,buf);
    spec_is_str_equal(buf," (TRUE_FALSE:1)");

    n = _t_detach_by_idx(t,1);
    _t_free(n);
    n = _t_newr(t,LT_INT);
    _t_newi(n,TEST_INT_SYMBOL,100);
    _t_newi(n,TEST_INT_SYMBOL,100);

    __p_reduce(defs,t,n);
    __t_dump(0,_t_child(t,1),0,buf);
    spec_is_str_equal(buf," (TRUE_FALSE:0)");

    // test >
    n = _t_detach_by_idx(t,1);
    _t_free(n);
    n = _t_newr(t,GT_INT);
    _t_newi(n,TEST_INT_SYMBOL,2);
    _t_newi(n,TEST_INT_SYMBOL,100);

    __p_reduce(defs,t,n);
    __t_dump(0,_t_child(t,1),0,buf);
    spec_is_str_equal(buf," (TRUE_FALSE:0)");

    n = _t_detach_by_idx(t,1);
    _t_free(n);
    n = _t_newr(t,GT_INT);
    _t_newi(n,TEST_INT_SYMBOL,101);
    _t_newi(n,TEST_INT_SYMBOL,100);

    __p_reduce(defs,t,n);
    __t_dump(0,_t_child(t,1),0,buf);
    spec_is_str_equal(buf," (TRUE_FALSE:1)");

    _t_free(t);
}


void testProcess() {
    testRunTree();
    testProcessReduceDefinedProcess();
     testProcessSignatureMatching();
    testProcessInterpolateMatch();
    testProcessIf();
    testProcessIntMath();
}
