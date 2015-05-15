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

    T *p3 = _t_newi(0,BOOLEAN,1);
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
    spec_is_symbol_equal(0,_t_symbol(t),BOOLEAN);
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
 * generate an example process definition that acts as an if for even numbers
 *
 * @snippet spec/process_spec.h defIfEven
 */
//! [defIfEven]
Process _defIfEven(T *processes) {
    T *code,*input,*output;

    /* a process that would look something like this in lisp:
       (defun if_even (val true_branch false_branch) (if (eq (mod val 2 ) 0) (true_branch) (false_branch)))

       or something like this in c:

       void if_even(int val,void (*true_branch)(),void (*false_branch)() ){
           if (val%2==0) (*true_branch)();
	   else (*false_branch)();
       }
    */
    code = _t_new_root(IF);                       // IF is a system process
    T *eq = _t_newi(code,EQ_INT,0);
    T *mod = _t_newi(eq,MOD_INT,0);
    int p1[] = {2,1,TREE_PATH_TERMINATOR};        // paths to the parameter refrences in the run tree: second branch, b1
    int p2[] = {2,2,TREE_PATH_TERMINATOR};        // second branch, b2
    int p3[] = {2,3,TREE_PATH_TERMINATOR};        // second branch, b3
    _t_new(mod,PARAM_REF,p1,sizeof(int)*3);       // param_ref should be a path_param_ref, also to be added is a label_param_ref
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
    T *n = _t_new_root(if_even);
    _t_newi(n,TEST_INT_SYMBOL,99);
    _t_newi(n,TEST_INT_SYMBOL,123);
    _t_newi(n,TEST_INT_SYMBOL,124);

    T *c = _t_rclone(n);
    _t_add(t,c);
    // confirm that it reduces correctly
    spec_is_equal(_p_reduce(defs,t),noReductionErr);
    spec_is_str_equal(t2s(_t_child(t,1)),"(TEST_INT_SYMBOL:124)");

    _t_free(processes);
    _t_free(t);_t_free(n);
    //! [testProcessReduceDefinedProcess]
}

void testProcessSignatureMatching() {
    T *processes = _t_new_root(PROCESSES);
    Defs defs = {0,0,processes};
    Process if_even = _defIfEven(processes);

    T *t = _t_new_root(RUN_TREE);
    T *n = _t_new_root(if_even);
    _t_new(n,TEST_STR_SYMBOL,"test",5);  // this should be an INTEGER!!
    _t_newi(n,TEST_INT_SYMBOL,123);
    _t_newi(n,TEST_INT_SYMBOL,124);

    T *c = _t_rclone(n);
    _t_add(t,c);
    spec_is_equal(_p_reduce(defs,t),badSignatureReductionErr);
    _t_free(_t_detach_by_idx(t,1));

    // too few params
    c = _t_rclone(n);
    _t_add(t,c);
    _t_free(_t_detach_by_idx(c,1));
    spec_is_equal(_p_reduce(defs,t),tooFewParamsReductionErr);
    _t_free(_t_detach_by_idx(t,1));

    // add too many params
    c = _t_rclone(n);
    _t_add(t,c);
    __t_newi(c,TEST_INT_SYMBOL,124,sizeof(rT));
    spec_is_equal(_p_reduce(defs,t),tooManyParamsReductionErr);

    _t_free(processes);
    _t_free(t);
    _t_free(n);
}

void testProcessInterpolateMatch() {
    Defs defs;
    T *t = _t_new_root(RUN_TREE);
    // test INTERPOLATE_FROM_MATCH which takes three params, the tree to interpolate, the stx-match and the tree it matched on
    T *n = _t_new_root(INTERPOLATE_FROM_MATCH);
    T *p1 = _t_newi(n,TEST_INT_SYMBOL2,0);
    _t_news(p1,INTERPOLATE_SYMBOL,TEST_INT_SYMBOL);
    T *p2 = _t_newi(n,SEMTREX_MATCH,1);
    _t_news(p2,SEMTREX_MATCH,TEST_INT_SYMBOL);
    int path[] = {TREE_PATH_TERMINATOR};
    _t_new(p2,SEMTREX_MATCH_PATH,path,2*sizeof(int));
    _t_newi(p2,SEMTREX_MATCH_SIBLINGS_COUNT,1);
    T *p3 = _t_newi(n,TEST_INT_SYMBOL,314);

    T *c = _t_rclone(n);
    _t_add(t,c);
    _p_reduce(defs,t);

    spec_is_str_equal(t2s(_t_child(t,1)),"(TEST_INT_SYMBOL2:0 (TEST_INT_SYMBOL:314))");
    _t_free(t);
    _t_free(n);
}

/// @todo when interpolating from a match, how do we handle non-leaf interpollations, i.e. where do you hook children onto?

void testProcessIf() {
    Defs defs;
    // test IF which takes three parameters, the condition, the true code tree and the false code tree
    T *n = _t_new_root(IF);
    T *p1 = _t_newi(n,BOOLEAN,1);
    T *p2 = _t_newi(n,TEST_INT_SYMBOL,99);
    T *p3 = _t_newi(n,TEST_INT_SYMBOL,100);

    __p_reduce_sys_proc(&defs,IF,n);
    spec_is_str_equal(t2s(n),"(TEST_INT_SYMBOL:99)");

    _t_free(n);
}

void testProcessIntMath() {
    Defs defs;

    T *t;

    // test addition
    T *n = _t_new_root(ADD_INT);
    _t_newi(n,TEST_INT_SYMBOL,99);
    _t_newi(n,TEST_INT_SYMBOL,100);
    __p_reduce_sys_proc(&defs,ADD_INT,n);
    spec_is_str_equal(t2s(n),"(TEST_INT_SYMBOL:199)");
    _t_free(n);

    // test subtraction
    n = _t_new_root(SUB_INT);
    _t_newi(n,TEST_INT_SYMBOL,100);
    _t_newi(n,TEST_INT_SYMBOL,98);
    __p_reduce_sys_proc(&defs,SUB_INT,n);
    spec_is_str_equal(t2s(n),"(TEST_INT_SYMBOL:2)");
    _t_free(n);

    // test multiplication
    n = _t_new_root(MULT_INT);
    _t_newi(n,TEST_INT_SYMBOL,100);
    _t_newi(n,TEST_INT_SYMBOL,98);
    __p_reduce_sys_proc(&defs,MULT_INT,n);
    spec_is_str_equal(t2s(n),"(TEST_INT_SYMBOL:9800)");
    _t_free(n);

    // test division
    n = _t_new_root(DIV_INT);
    _t_newi(n,TEST_INT_SYMBOL,100);
    _t_newi(n,TEST_INT_SYMBOL,48);
    __p_reduce_sys_proc(&defs,DIV_INT,n);
    spec_is_str_equal(t2s(n),"(TEST_INT_SYMBOL:2)");
    _t_free(n);

    // test division with divide by zero
    n = _t_new_root(DIV_INT);
    _t_newi(n,TEST_INT_SYMBOL,100);
    _t_newi(n,TEST_INT_SYMBOL,0);
    spec_is_equal(__p_reduce_sys_proc(&defs,DIV_INT,n),divideByZeroReductionErr);
    _t_free(n);

    // test modulo
    n = _t_new_root(MOD_INT);
    _t_newi(n,TEST_INT_SYMBOL,100);
    _t_newi(n,TEST_INT_SYMBOL,2);
    __p_reduce_sys_proc(&defs,MOD_INT,n);
    spec_is_str_equal(t2s(n),"(TEST_INT_SYMBOL:0)");
    _t_free(n);

    // test modulo with divide by zero
    n = _t_new_root(MOD_INT);
    _t_newi(n,TEST_INT_SYMBOL,100);
    _t_newi(n,TEST_INT_SYMBOL,0);
    spec_is_equal(__p_reduce_sys_proc(&defs,MOD_INT,n),divideByZeroReductionErr);
    _t_free(n);

    // test equals
    n = _t_new_root(EQ_INT);
    _t_newi(n,TEST_INT_SYMBOL,100);
    _t_newi(n,TEST_INT_SYMBOL,2);
    __p_reduce_sys_proc(&defs,EQ_INT,n);
    spec_is_str_equal(t2s(n),"(BOOLEAN:0)");
    _t_free(n);

    n = _t_new_root(EQ_INT);
    _t_newi(n,TEST_INT_SYMBOL,100);
    _t_newi(n,TEST_INT_SYMBOL,100);
    __p_reduce_sys_proc(&defs,EQ_INT,n);
    spec_is_str_equal(t2s(n),"(BOOLEAN:1)");
    _t_free(n);

    // test <
    n = _t_new_root(LT_INT);
    _t_newi(n,TEST_INT_SYMBOL,2);
    _t_newi(n,TEST_INT_SYMBOL,100);
    __p_reduce_sys_proc(&defs,LT_INT,n);
    spec_is_str_equal(t2s(n),"(BOOLEAN:1)");
    _t_free(n);

    n = _t_new_root(LT_INT);
    _t_newi(n,TEST_INT_SYMBOL,100);
    _t_newi(n,TEST_INT_SYMBOL,100);
    __p_reduce_sys_proc(&defs,LT_INT,n);
    spec_is_str_equal(t2s(n),"(BOOLEAN:0)");
   _t_free(n);

    // test >
    n = _t_new_root(GT_INT);
    _t_newi(n,TEST_INT_SYMBOL,2);
    _t_newi(n,TEST_INT_SYMBOL,100);
    __p_reduce_sys_proc(&defs,GT_INT,n);
    spec_is_str_equal(t2s(n),"(BOOLEAN:0)");
    _t_free(n);

    n = _t_new_root(GT_INT);
    _t_newi(n,TEST_INT_SYMBOL,101);
    _t_newi(n,TEST_INT_SYMBOL,100);
    __p_reduce_sys_proc(&defs,GT_INT,n);
    spec_is_str_equal(t2s(n),"(BOOLEAN:1)");
    _t_free(n);

}

void testProcessString() {
    Defs defs;

    // test string concatenation
    T *n = _t_new_root(CONCAT_STR);
    _t_news(n,RESULT_SYMBOL,TEST_NAME_SYMBOL);
    _t_new_str(n,TEST_STR_SYMBOL,"Fred");
    _t_new_str(n,TEST_STR_SYMBOL," ");
    _t_new_str(n,TEST_STR_SYMBOL,"Smith");

    __p_reduce_sys_proc(&defs,CONCAT_STR,n);

    spec_is_str_equal(t2s(n),"(TEST_NAME_SYMBOL:Fred Smith)");

    _t_free(n);
}

void testProcessReduce() {
    Defs defs;
    T *t = _t_new_root(RUN_TREE);

    T *n = _t_new_root(IF); // multi-level IF to test descending a number of levels
    _t_newi(n,BOOLEAN,1);
    T *n1 = _t_newr(n,IF);
    T *n2 = _t_newr(n1,IF);
    _t_newi(n2,BOOLEAN,0);
    _t_newi(n2,BOOLEAN,1);
    _t_newi(n2,BOOLEAN,0);
    _t_newi(n1,TEST_INT_SYMBOL,98);
    _t_newi(n1,TEST_INT_SYMBOL,99);
    _t_newi(n,TEST_INT_SYMBOL,100);

    T *c = _t_rclone(n);
    _t_add(t,c);

    R context;
    _p_init_context(t,&context);

    spec_is_equal(rt_cur_child(c),0);
    _p_step(defs,t,&context);
    // first step goes into the boolean
    spec_is_equal(rt_cur_child(c),1);
    spec_is_ptr_equal(context.node_pointer,_t_child(c,1));

    // second step ascends back to top if and marks boolean complete
    _p_step(defs,t,&context);
    spec_is_equal(rt_cur_child(_t_child(c,1)),RUN_TREE_EVALUATED);
    spec_is_ptr_equal(context.node_pointer,c);

    // third step goes into the second level if
    _p_step(defs,t,&context);
    spec_is_equal(rt_cur_child(c),2);
    spec_is_ptr_equal(context.node_pointer,_t_child(c,2));

    // not specing out all the steps because there are soooo many...

    // just re-running them all for final result
    _t_free(_t_detach_by_idx(t,1));
    c = _t_rclone(n);
    _t_add(t,c);
    _p_reduce(defs,t);

    spec_is_str_equal(t2s(c),"(TEST_INT_SYMBOL:99)");

    _t_free(n);
    _t_free(t);
}

void testProcessError() {
    Defs defs;
    T *t = _t_new_root(RUN_TREE);
    T *n = _t_new_root(RESPOND);
    T *d = _t_newr(n,DIV_INT);
    _t_newi(d,TEST_INT_SYMBOL,100);
    _t_newi(d,TEST_INT_SYMBOL,0);
    T *c = _t_rclone(n);
    _t_add(t,c);
    T *ps = _t_newr(t,PARAMS);

    // error routine is just a param ref to pass back the error tree
    int pt[] = {2,1,TREE_PATH_TERMINATOR};
    _t_new(t,PARAM_REF,pt,sizeof(int)*4);

    Error e = _p_reduce(defs,t);
    spec_is_equal(e,noReductionErr);
    spec_is_str_equal(t2s(_t_child(t,1)),"(ZERO_DIVIDE_ERR (ERROR_LOCATION:/1/1))");
    _t_free(n);
    _t_free(t);
}


void testProcessRaise() {
    Defs defs;
    T *n = _t_new_root(RAISE);
    _t_news(n,REDUCTION_ERROR_SYMBOL,ZERO_DIVIDE_ERR);

    spec_is_equal(__p_reduce_sys_proc(&defs,RAISE,n),raiseReductionErr);
    _t_free(n);

    T *t = _t_new_root(RUN_TREE);
    n = _t_new_root(RAISE);
    _t_news(n,REDUCTION_ERROR_SYMBOL,NOT_A_PROCESS_ERR); // pick a random error to raise
    T *c = _t_rclone(n);
    _t_add(t,c);
    T *ps = _t_newr(t,PARAMS);

    // error routine is just a param ref to pass back the error tree
    int pt[] = {2,1,TREE_PATH_TERMINATOR};
    _t_new(t,PARAM_REF,pt,sizeof(int)*4);

    Error e = _p_reduce(defs,t);
    spec_is_equal(e,noReductionErr);
    spec_is_str_equal(t2s(_t_child(t,1)),"(NOT_A_PROCESS_ERR (ERROR_LOCATION:/1))");
    _t_free(n);
    _t_free(t);
}

void testProcess() {
    testRunTree();
    testProcessReduceDefinedProcess();
    testProcessSignatureMatching();
    testProcessInterpolateMatch();
    testProcessIf();
    testProcessIntMath();
    testProcessString();
    testProcessReduce();
    testProcessError();
    testProcessRaise();
}
