/**
 * @file process_spec.h
 * @copyright Copyright (C) 2013-2015, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 * @ingroup tests
 */

#include "../src/ceptr.h"
#include "../src/process.h"

void testRunTree() {
    T *processes = _t_new_root(PROCESSES);
    Defs defs = {0,0,processes};
    T *code,*input,*output,*t;

    // a process that would look something like this in lisp:
    // (defun my_if (true_branch false_branch condition) (if (condition) (true_branch) (false_branch)))
    code = _t_new_root(IF);								// IF is a system process
    int pt1[] = {2,1,TREE_PATH_TERMINATOR};
    int pt2[] = {2,2,TREE_PATH_TERMINATOR};
    int pt3[] = {2,3,TREE_PATH_TERMINATOR};

    _t_new(code,PARAM_REF,pt3,sizeof(int)*4);
    _t_new(code,PARAM_REF,pt1,sizeof(int)*4);
    _t_new(code,PARAM_REF,pt2,sizeof(int)*4);

    input = _t_new_root(INPUT);
    T *i3 = _t_newr(input,INPUT_SIGNATURE);
    _t_news(i3,SIGNATURE_STRUCTURE,INTEGER);
    _t_new(i3,INPUT_LABEL,"condition",4);
    T *i1 = _t_newr(input,INPUT_SIGNATURE);
    _t_news(i1,SIGNATURE_STRUCTURE,TREE);
    _t_new(i1,INPUT_LABEL,"true_branch",4);
    T *i2 = _t_newr(input,INPUT_SIGNATURE);
    _t_news(i2,SIGNATURE_STRUCTURE,TREE);
    _t_new(i2,INPUT_LABEL,"false_branch",4);


    output = _t_new_root(OUTPUT_SIGNATURE);
    Process p = _d_code_process(processes,code,"myif","a duplicate of the sys if process with params in different order",input,output,RECEPTOR_CONTEXT);

    T *p3 = _t_newi(0,TRUE_FALSE,1);
    T *p1 = _t_newi(0,TEST_INT_SYMBOL,123);
    T *p2 = _t_newi(0,TEST_INT_SYMBOL,321);

    T *act = _t_newp(0,ACTION,p);

    T *r = _p_make_run_tree(processes,act,3,p1,p2,p3);

    spec_is_symbol_equal(0,_t_symbol(r),RUN_TREE);

    t = _t_child(r,1);  // first child should be clone of code
    spec_is_sem_equal(_t_symbol(t),IF);
    spec_is_true(t!=code);  //should be a clone

    T *ps = _t_child(r,2); //second child should be params
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

    spec_is_equal(_p_reduce(defs,r),noReductionErr);

    spec_is_str_equal(t2s(_t_child(r,1)),"(TEST_INT_SYMBOL:123)");

    _t_free(act);
    _t_free(r);
    _t_free(processes);
    _t_free(p1);
    _t_free(p2);
    _t_free(p3);
}

/**
 * generate an example process definition for acts as an if for even numbers
 *
 * @snippet spec/process_spec.h defIfEven
 */
//! [defIfEven]
Process _defIfEven(T *processes) {
    T *code,*input,*output;

    // a process that would look something like this in lisp:
    // (defun if_even (val true_branch false_branch) (if (eq (mod val 2 ) 0) (true_branch) (false_branch)))
    code = _t_new_root(IF);															// IF is a system process
    T *eq = _t_newi(code,EQ_INT,0);
    T *mod = _t_newi(eq,MOD_INT,0);
    int p1[] = {2,1,TREE_PATH_TERMINATOR};			// paths to the parameter refrences in the run tree: second branch, b1
    int p2[] = {2,2,TREE_PATH_TERMINATOR};			// second branch, b2
    int p3[] = {2,3,TREE_PATH_TERMINATOR};			// second branch, b3
    _t_new(mod,PARAM_REF,p1,sizeof(int)*3);			// param_ref should be a path_param_ref, also to be added is a label_param_ref
    _t_newi(mod,TEST_INT_SYMBOL,2);
    _t_newi(eq,TEST_INT_SYMBOL,0);
    _t_new(code,PARAM_REF,p2,sizeof(int)*3);
    _t_new(code,PARAM_REF,p3,sizeof(int)*3);
    input = _t_new_root(INPUT);
    T *i1 = _t_newr(input,INPUT_SIGNATURE);
    _t_news(i1,SIGNATURE_STRUCTURE,INTEGER);
    _t_new(i1,INPUT_LABEL,"val",4);
    T *i2 = _t_newr(input,INPUT_SIGNATURE);
    _t_news(i2,SIGNATURE_STRUCTURE,TREE);
    _t_new(i2,INPUT_LABEL,"true_branch",4);
    T *i3 = _t_newr(input,INPUT_SIGNATURE);
    _t_news(i3,SIGNATURE_STRUCTURE,TREE);
    _t_new(i3,INPUT_LABEL,"false_branch",4);

    output = _t_new_root(OUTPUT_SIGNATURE);

    return _d_code_process(processes,code,"if even","return 2nd child if even, third if not",input,output,RECEPTOR_CONTEXT);
}
//! [defIfEven]

void testProcessReduceDefinedProcess() {
    //! [testProcessReduceDefinedProcess]
    T *processes = _t_new_root(PROCESSES);
    Defs defs = {0,0,processes};

    Process if_even = _defIfEven(processes);  // add the if_even process to our defs

    // check that it dumps nicely, including showing the param_refs as paths
    int p[] = {1,3,TREE_PATH_TERMINATOR};

    spec_is_str_equal(t2s(_t_get(processes,p)),"(process:IF (process:EQ_INT (process:MOD_INT (PARAM_REF:/2/1) (TEST_INT_SYMBOL:2)) (TEST_INT_SYMBOL:0)) (PARAM_REF:/2/2) (PARAM_REF:/2/3))");

    // create a run tree right in the position to "call"this function
    T *t = _t_new_root(RUN_TREE);
    T *n = _t_newr(t,if_even);
    _t_newi(n,TEST_INT_SYMBOL,99);
    _t_newi(n,TEST_INT_SYMBOL,123);
    _t_newi(n,TEST_INT_SYMBOL,124);

    // confirm that it reduces correctly
    spec_is_equal(__p_reduce(defs,t,n),noReductionErr);
    spec_is_str_equal(t2s(_t_child(t,1)),"(TEST_INT_SYMBOL:124)");

    _t_free(processes);
    _t_free(t);
    //! [testProcessReduceDefinedProcess]
}

void testProcessSignatureMatching() {
    T *processes = _t_new_root(PROCESSES);
    Defs defs = {0,0,processes};
    Process if_even = _defIfEven(processes);

    T *t = _t_new_root(RUN_TREE);
    T *n = _t_newr(t,if_even);
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
    T *t = _t_new_root(RUN_TREE);
    // test INTERPOLATE_FROM_MATCH which takes three params, the tree to interpolate, the stx-match and the tree it matched on
    T *n = _t_newr(t,INTERPOLATE_FROM_MATCH);
    T *p1 = _t_newi(n,TEST_INT_SYMBOL2,0);
    _t_news(p1,INTERPOLATE_SYMBOL,TEST_INT_SYMBOL);
    T *p2 = _t_newi(n,SEMTREX_MATCH,1);
    _t_news(p2,SEMTREX_MATCH,TEST_INT_SYMBOL);
    int path[] = {TREE_PATH_TERMINATOR};
    _t_new(p2,SEMTREX_MATCHED_PATH,path,2*sizeof(int));
    _t_newi(p2,SEMTREX_MATCH_SIBLINGS_COUNT,1);
    T *p3 = _t_newi(n,TEST_INT_SYMBOL,314);
    __p_reduce(defs,t,n);

    spec_is_str_equal(t2s(_t_child(t,1)),"(TEST_INT_SYMBOL2:0 (TEST_INT_SYMBOL:314))");
    _t_free(t);
}

/// @todo when interpolating from a match, how do we handle non-leaf interpollations, i.e. where do you hook children onto?

void testProcessIf() {
    Defs defs;
    // test IF which takes three parameters, the condition, the true code tree and the false code tree
    T *t = _t_new_root(RUN_TREE);
    T *n = _t_newr(t,IF);
    T *p1 = _t_newi(n,TRUE_FALSE,1);
    T *p2 = _t_newi(n,TEST_INT_SYMBOL,99);
    T *p3 = _t_newi(n,TEST_INT_SYMBOL,100);

    __p_reduce(defs,t,n);
    spec_is_str_equal(t2s(_t_child(t,1)),"(TEST_INT_SYMBOL:99)");

    _t_free(t);
}

void testProcessIntMath() {
    Defs defs;

    T *t = _t_new_root(RUN_TREE);

    // test addition
    T *n = _t_newr(t,ADD_INT);
    _t_newi(n,TEST_INT_SYMBOL,99);
    _t_newi(n,TEST_INT_SYMBOL,100);
    __p_reduce(defs,t,n);

    spec_is_str_equal(t2s(_t_child(t,1)),"(TEST_INT_SYMBOL:199)");

    // test subtraction
    n = _t_detach_by_idx(t,1);
    _t_free(n);
    n = _t_newr(t,SUB_INT);
    _t_newi(n,TEST_INT_SYMBOL,100);
    _t_newi(n,TEST_INT_SYMBOL,98);

    __p_reduce(defs,t,n);
    spec_is_str_equal(t2s(_t_child(t,1)),"(TEST_INT_SYMBOL:2)");

    // test multiplication
    n = _t_detach_by_idx(t,1);
    _t_free(n);
    n = _t_newr(t,MULT_INT);
    _t_newi(n,TEST_INT_SYMBOL,100);
    _t_newi(n,TEST_INT_SYMBOL,98);

    __p_reduce(defs,t,n);
    spec_is_str_equal(t2s(_t_child(t,1)),"(TEST_INT_SYMBOL:9800)");

    // test division
    n = _t_detach_by_idx(t,1);
    _t_free(n);
    n = _t_newr(t,DIV_INT);
    _t_newi(n,TEST_INT_SYMBOL,100);
    _t_newi(n,TEST_INT_SYMBOL,48);

    __p_reduce(defs,t,n);
    spec_is_str_equal(t2s(_t_child(t,1)),"(TEST_INT_SYMBOL:2)");

    // test modulo
    n = _t_detach_by_idx(t,1);
    _t_free(n);
    n = _t_newr(t,MOD_INT);
    _t_newi(n,TEST_INT_SYMBOL,100);
    _t_newi(n,TEST_INT_SYMBOL,2);

    __p_reduce(defs,t,n);
    spec_is_str_equal(t2s(_t_child(t,1)),"(TEST_INT_SYMBOL:0)");

    // test equals
    n = _t_detach_by_idx(t,1);
    _t_free(n);
    n = _t_newr(t,EQ_INT);
    _t_newi(n,TEST_INT_SYMBOL,100);
    _t_newi(n,TEST_INT_SYMBOL,2);

    __p_reduce(defs,t,n);
    spec_is_str_equal(t2s(_t_child(t,1)),"(TRUE_FALSE:0)");

    n = _t_detach_by_idx(t,1);
    _t_free(n);
    n = _t_newr(t,EQ_INT);
    _t_newi(n,TEST_INT_SYMBOL,100);
    _t_newi(n,TEST_INT_SYMBOL,100);

    __p_reduce(defs,t,n);
    spec_is_str_equal(t2s(_t_child(t,1)),"(TRUE_FALSE:1)");

    // test <
    n = _t_detach_by_idx(t,1);
    _t_free(n);
    n = _t_newr(t,LT_INT);
    _t_newi(n,TEST_INT_SYMBOL,2);
    _t_newi(n,TEST_INT_SYMBOL,100);

    __p_reduce(defs,t,n);
    spec_is_str_equal(t2s(_t_child(t,1)),"(TRUE_FALSE:1)");

    n = _t_detach_by_idx(t,1);
    _t_free(n);
    n = _t_newr(t,LT_INT);
    _t_newi(n,TEST_INT_SYMBOL,100);
    _t_newi(n,TEST_INT_SYMBOL,100);

    __p_reduce(defs,t,n);
    spec_is_str_equal(t2s(_t_child(t,1)),"(TRUE_FALSE:0)");

    // test >
    n = _t_detach_by_idx(t,1);
    _t_free(n);
    n = _t_newr(t,GT_INT);
    _t_newi(n,TEST_INT_SYMBOL,2);
    _t_newi(n,TEST_INT_SYMBOL,100);

    __p_reduce(defs,t,n);
    spec_is_str_equal(t2s(_t_child(t,1)),"(TRUE_FALSE:0)");

    n = _t_detach_by_idx(t,1);
    _t_free(n);
    n = _t_newr(t,GT_INT);
    _t_newi(n,TEST_INT_SYMBOL,101);
    _t_newi(n,TEST_INT_SYMBOL,100);

    __p_reduce(defs,t,n);
    spec_is_str_equal(t2s(_t_child(t,1)),"(TRUE_FALSE:1)");

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
