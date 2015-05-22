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
    code = _t_new_root(IF);                             // IF is a system process
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

    spec_is_equal(_p_reduce(&defs,r),noReductionErr);

    spec_is_str_equal(t2s(_t_child(r,1)),"(TEST_INT_SYMBOL:123)");

    _t_free(act);
    _t_free(r);
    _t_free(processes);
    _t_free(p1);
    _t_free(p2);
    _t_free(p3);
}

//-----------------------------------------------------------------------------------------
// tests of system processes

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
    _p_reduce(&defs,t);

    spec_is_str_equal(t2s(_t_child(t,1)),"(TEST_INT_SYMBOL2:0 (TEST_INT_SYMBOL:314))");
    _t_free(t);
    _t_free(n);
}

/// @todo when interpolating from a match, how do we handle non-leaf interpollations, i.e. where do you hook children onto?

void testProcessIf() {
    // test IF which takes three parameters, the condition, the true code tree and the false code tree
    T *n = _t_new_root(IF);
    T *p1 = _t_newi(n,BOOLEAN,1);
    T *p2 = _t_newi(n,TEST_INT_SYMBOL,99);
    T *p3 = _t_newi(n,TEST_INT_SYMBOL,100);

    __p_reduce_sys_proc(0,IF,n);
    spec_is_str_equal(t2s(n),"(TEST_INT_SYMBOL:99)");

    _t_free(n);
}

void testProcessIntMath() {
    T *t;

    // test addition
    T *n = _t_new_root(ADD_INT);
    _t_newi(n,TEST_INT_SYMBOL,99);
    _t_newi(n,TEST_INT_SYMBOL,100);
    __p_reduce_sys_proc(0,ADD_INT,n);
    spec_is_str_equal(t2s(n),"(TEST_INT_SYMBOL:199)");
    _t_free(n);

    // test subtraction
    n = _t_new_root(SUB_INT);
    _t_newi(n,TEST_INT_SYMBOL,100);
    _t_newi(n,TEST_INT_SYMBOL,98);
    __p_reduce_sys_proc(0,SUB_INT,n);
    spec_is_str_equal(t2s(n),"(TEST_INT_SYMBOL:2)");
    _t_free(n);

    // test multiplication
    n = _t_new_root(MULT_INT);
    _t_newi(n,TEST_INT_SYMBOL,100);
    _t_newi(n,TEST_INT_SYMBOL,98);
    __p_reduce_sys_proc(0,MULT_INT,n);
    spec_is_str_equal(t2s(n),"(TEST_INT_SYMBOL:9800)");
    _t_free(n);

    // test division
    n = _t_new_root(DIV_INT);
    _t_newi(n,TEST_INT_SYMBOL,100);
    _t_newi(n,TEST_INT_SYMBOL,48);
    __p_reduce_sys_proc(0,DIV_INT,n);
    spec_is_str_equal(t2s(n),"(TEST_INT_SYMBOL:2)");
    _t_free(n);

    // test division with divide by zero
    n = _t_new_root(DIV_INT);
    _t_newi(n,TEST_INT_SYMBOL,100);
    _t_newi(n,TEST_INT_SYMBOL,0);
    spec_is_equal(__p_reduce_sys_proc(0,DIV_INT,n),divideByZeroReductionErr);
    _t_free(n);

    // test modulo
    n = _t_new_root(MOD_INT);
    _t_newi(n,TEST_INT_SYMBOL,100);
    _t_newi(n,TEST_INT_SYMBOL,2);
    __p_reduce_sys_proc(0,MOD_INT,n);
    spec_is_str_equal(t2s(n),"(TEST_INT_SYMBOL:0)");
    _t_free(n);

    // test modulo with divide by zero
    n = _t_new_root(MOD_INT);
    _t_newi(n,TEST_INT_SYMBOL,100);
    _t_newi(n,TEST_INT_SYMBOL,0);
    spec_is_equal(__p_reduce_sys_proc(0,MOD_INT,n),divideByZeroReductionErr);
    _t_free(n);

    // test equals
    n = _t_new_root(EQ_INT);
    _t_newi(n,TEST_INT_SYMBOL,100);
    _t_newi(n,TEST_INT_SYMBOL,2);
    __p_reduce_sys_proc(0,EQ_INT,n);
    spec_is_str_equal(t2s(n),"(BOOLEAN:0)");
    _t_free(n);

    n = _t_new_root(EQ_INT);
    _t_newi(n,TEST_INT_SYMBOL,100);
    _t_newi(n,TEST_INT_SYMBOL,100);
    __p_reduce_sys_proc(0,EQ_INT,n);
    spec_is_str_equal(t2s(n),"(BOOLEAN:1)");
    _t_free(n);

    // test <
    n = _t_new_root(LT_INT);
    _t_newi(n,TEST_INT_SYMBOL,2);
    _t_newi(n,TEST_INT_SYMBOL,100);
    __p_reduce_sys_proc(0,LT_INT,n);
    spec_is_str_equal(t2s(n),"(BOOLEAN:1)");
    _t_free(n);

    n = _t_new_root(LT_INT);
    _t_newi(n,TEST_INT_SYMBOL,100);
    _t_newi(n,TEST_INT_SYMBOL,100);
    __p_reduce_sys_proc(0,LT_INT,n);
    spec_is_str_equal(t2s(n),"(BOOLEAN:0)");
   _t_free(n);

    // test >
    n = _t_new_root(GT_INT);
    _t_newi(n,TEST_INT_SYMBOL,2);
    _t_newi(n,TEST_INT_SYMBOL,100);
    __p_reduce_sys_proc(0,GT_INT,n);
    spec_is_str_equal(t2s(n),"(BOOLEAN:0)");
    _t_free(n);

    n = _t_new_root(GT_INT);
    _t_newi(n,TEST_INT_SYMBOL,101);
    _t_newi(n,TEST_INT_SYMBOL,100);
    __p_reduce_sys_proc(0,GT_INT,n);
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

    __p_reduce_sys_proc(0,CONCAT_STR,n);

    spec_is_str_equal(t2s(n),"(TEST_NAME_SYMBOL:Fred Smith)");

    _t_free(n);
}

void textProcessRespond() {
    // testing responding to a signal requires setting up a sending signal context

    T *signal_contents = _t_newi(0,TEST_INT_SYMBOL,314);
    Xaddr f = {RECEPTOR_XADDR,3};  // DUMMY XADDR
    Xaddr t = {RECEPTOR_XADDR,4};  // DUMMY XADDR
    T *s = __r_make_signal(f,t,DEFAULT_ASPECT,signal_contents);

    T *run_tree = _t_new_root(RUN_TREE);
    T *n = _t_newr(run_tree,RESPOND);
    T *response_contents = _t_newi(n,TEST_INT_SYMBOL,271);

    R *c = __p_make_context(run_tree,0);

    // if this is a run-tree that's not a child of a signal we can't respond!
    spec_is_equal(__p_reduce_sys_proc(c,RESPOND,n),notInSignalContextReductionError);

    // no add it to the signal and try again
    _t_add(s,run_tree);

    // now it should create a signal for responding
    spec_is_equal(__p_reduce_sys_proc(c,RESPOND,n),noReductionErr);

    spec_is_str_equal(t2s(s),"(SIGNAL (ENVELOPE (RECEPTOR_XADDR:RECEPTOR_XADDR.3) (RECEPTOR_XADDR:RECEPTOR_XADDR.4) (ASPECT:1)) (BODY:{(TEST_INT_SYMBOL:314)}) (RUN_TREE (TEST_INT_SYMBOL:0) (SIGNALS (SIGNAL (ENVELOPE (RECEPTOR_XADDR:RECEPTOR_XADDR.4) (RECEPTOR_XADDR:RECEPTOR_XADDR.3) (ASPECT:1)) (BODY:{(TEST_INT_SYMBOL:271)})))))");

}

//-----------------------------------------------------------------------------------------
// tests of process execution (reduction)

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

    R *context = __p_make_context(t,0);

    spec_is_equal(rt_cur_child(c),0);
    // first step is Eval and next step is Descend
    spec_is_equal(_p_step(&defs,&context),Descend);
    spec_is_equal(rt_cur_child(c),0);

    // next step after Descend changes node pointer and moved to Eval
    spec_is_equal(_p_step(&defs,&context),Eval);
    spec_is_ptr_equal(context->node_pointer,_t_child(c,1));
    spec_is_equal(rt_cur_child(c),1);

    // after Eval next step will be Ascend
    spec_is_equal(_p_step(&defs,&context),Ascend);

    // step ascends back to top if and marks boolean complete
    spec_is_equal(_p_step(&defs,&context),Eval);

    spec_is_equal(rt_cur_child(_t_child(c,1)),RUN_TREE_EVALUATED);
    spec_is_ptr_equal(context->node_pointer,c);

    spec_is_equal(_p_step(&defs,&context),Descend);

    // third step goes into the second level if
    spec_is_equal(_p_step(&defs,&context),Eval);
    spec_is_equal(rt_cur_child(c),2);
    spec_is_ptr_equal(context->node_pointer,_t_child(c,2));

    free(context);
    // not specing out all the steps because there are soooo many...

    // just re-running them all for final result
    _t_free(_t_detach_by_idx(t,1));
    c = _t_rclone(n);
    _t_add(t,c);
    _p_reduce(&defs,t);

    spec_is_str_equal(t2s(c),"(TEST_INT_SYMBOL:99)");

    _t_free(n);
    _t_free(t);
}

/**
 * helper to generate an example process definition that acts as an if for even numbers
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
    T *eq = _t_newr(code,EQ_INT);
    T *mod = _t_newr(eq,MOD_INT);
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

/**
 * helper to generate an example process definition that creates a divide by zero error
 *
 * @snippet spec/process_spec.h defDivZero
 */
//! [defDivZero]
Process _defDivZero(T *processes) {
    T *code,*input,*output;

    /* a process that would look something like this in lisp:
       (defun div_zero (val) (/ val 0))
    */
    code = _t_new_root(DIV_INT);                       // IF is a system process
    _t_newi(code,TEST_INT_SYMBOL,2);
    _t_newi(code,TEST_INT_SYMBOL,0);
    input = _t_new_root(INPUT);
    T *i1 = _t_newr(input,INPUT_SIGNATURE);
    _t_news(i1,SIGNATURE_STRUCTURE,INTEGER);
    _t_new(i1,INPUT_LABEL,"val",4);
     output = _t_new_root(OUTPUT_SIGNATURE);

    return _d_code_process(processes,code,"divByZero","create a divide by zero error",input,output,RECEPTOR_CONTEXT);
}
//! [defDivZero]

void testProcessReduceDefinedProcess() {
    //! [testProcessReduceDefinedProcess]
    T *processes = _t_new_root(PROCESSES);
    Defs defs = {0,0,processes};

    Process if_even = _defIfEven(processes);  // add the if_even process to our defs

    // check that it dumps nicely, including showing the param_refs as paths
    int p[] = {1,3,TREE_PATH_TERMINATOR};

    spec_is_str_equal(t2s(_t_get(processes,p)),"(process:IF (process:EQ_INT (process:MOD_INT (PARAM_REF:/2/1) (TEST_INT_SYMBOL:2)) (TEST_INT_SYMBOL:0)) (PARAM_REF:/2/2) (PARAM_REF:/2/3))");


    // create a run tree right in the state to "call"this function
    T *n = _t_new_root(PARAMS);
    _t_newi(n,TEST_INT_SYMBOL,99);
    _t_newi(n,TEST_INT_SYMBOL,123);
    _t_newi(n,TEST_INT_SYMBOL,124);
    T *t = __p_make_run_tree(processes,if_even,n);

    // confirm that it reduces correctly
    spec_is_equal(_p_reduce(&defs,t),noReductionErr);
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
    spec_is_equal(_p_reduce(&defs,t),signatureMismatchReductionErr);
    _t_free(_t_detach_by_idx(t,1));

    // too few params
    c = _t_rclone(n);
    _t_add(t,c);
    _t_free(_t_detach_by_idx(c,1));
    spec_is_equal(_p_reduce(&defs,t),tooFewParamsReductionErr);
    _t_free(_t_detach_by_idx(t,1));

    // add too many params
    c = _t_rclone(n);
    _t_add(t,c);
    __t_newi(c,TEST_INT_SYMBOL,124,sizeof(rT));
    spec_is_equal(_p_reduce(&defs,t),tooManyParamsReductionErr);

    _t_free(processes);
    _t_free(t);
    _t_free(n);
}

void testProcessError() {
    Defs defs;
    T *t = _t_new_root(RUN_TREE);
    T *n = _t_new_root(NOOP);
    T *d = _t_newr(n,DIV_INT);
    _t_newi(d,TEST_INT_SYMBOL,100);
    _t_newi(d,TEST_INT_SYMBOL,0);
    T *c = _t_rclone(n);
    _t_add(t,c);
    T *ps = _t_newr(t,PARAMS);

    // error routine is just a param ref to pass back the error tree
    int pt[] = {4,1,TREE_PATH_TERMINATOR};
    __t_new(t,PARAM_REF,pt,sizeof(int)*4,sizeof(rT));

    Error e = _p_reduce(&defs,t);
    spec_is_equal(e,noReductionErr);
    spec_is_str_equal(t2s(_t_child(t,1)),"(ZERO_DIVIDE_ERR (ERROR_LOCATION:/1/1))");
    _t_free(n);
    _t_free(t);
}

void testProcessRaise() {
    Defs defs;
    T *n = _t_new_root(RAISE);
    _t_news(n,REDUCTION_ERROR_SYMBOL,ZERO_DIVIDE_ERR);

    spec_is_equal(__p_reduce_sys_proc(0,RAISE,n),raiseReductionErr);
    _t_free(n);

    T *t = _t_new_root(RUN_TREE);
    n = _t_new_root(RAISE);
    _t_news(n,REDUCTION_ERROR_SYMBOL,NOT_A_PROCESS_ERR); // pick a random error to raise
    T *c = _t_rclone(n);
    _t_add(t,c);
    T *ps = _t_newr(t,PARAMS);

    // error routine is just a param ref to pass back the error tree
    int pt[] = {4,1,TREE_PATH_TERMINATOR};
    __t_new(t,PARAM_REF,pt,sizeof(int)*4,sizeof(rT));

    Error e = _p_reduce(&defs,t);
    spec_is_equal(e,noReductionErr);
    spec_is_str_equal(t2s(_t_child(t,1)),"(NOT_A_PROCESS_ERR (ERROR_LOCATION:/1))");
    _t_free(n);
    _t_free(t);
}

void testProcessErrorTrickleUp() {
    //! [testProcessErrorTrickleUp]
    T *processes = _t_new_root(PROCESSES);
    Defs defs = {0,0,processes};

    Process divz = _defDivZero(processes);  // add the if_even process to our defs

    // create a run tree right in the position to "call" this function
    T *t = _t_new_root(RUN_TREE);
    T *n = _t_new_root(NOOP);
    T *d = _t_newr(n,divz);
    _t_newi(d,TEST_INT_SYMBOL,124);

    T *c = _t_rclone(n);
    _t_add(t,c);
    T *ps = _t_newr(t,PARAMS);

    // error routine is just a param ref to pass back the error tree
    int pt[] = {4,1,TREE_PATH_TERMINATOR};
    __t_new(t,PARAM_REF,pt,sizeof(int)*4,sizeof(rT));

    // confirm that it reduces correctly
    spec_is_equal(_p_reduce(&defs,t),noReductionErr);
    spec_is_str_equal(t2s(_t_child(t,1)),"(ZERO_DIVIDE_ERR (ERROR_LOCATION:/1/1))");

    _t_free(processes);
    _t_free(t);_t_free(n);
    //! [testProcessErrorTrickleUp]
}

void testProcessMulti() {
    //! [testProcessMulti]
    T *processes = _t_new_root(PROCESSES);
    Defs defs = {0,0,processes};

    Process if_even = _defIfEven(processes);  // add the if_even process to our defs

    // create two run trees
    T *n = _t_new_root(PARAMS);
    _t_newi(n,TEST_INT_SYMBOL,99);
    _t_newi(n,TEST_INT_SYMBOL,123);
    _t_newi(n,TEST_INT_SYMBOL,124);
    T *t1 = __p_make_run_tree(processes,if_even,n);

    _t_newi(n,TEST_INT_SYMBOL,100);
    T *l2 = _t_newr(n,if_even);
    _t_newi(l2,TEST_INT_SYMBOL,2);
    _t_newi(l2,TEST_INT_SYMBOL,123);
    _t_newi(l2,TEST_INT_SYMBOL,124);
    _t_newi(n,TEST_INT_SYMBOL,314);

    T *t2 = __p_make_run_tree(processes,if_even,n);

    // add them to a processing queue
    Q *q = _p_newq(&defs);
    spec_is_equal(q->contexts_count,0);
    spec_is_ptr_equal(q->active,NULL);
    _p_addrt2q(q,t1);
    spec_is_equal(q->contexts_count,1);
    spec_is_ptr_equal(q->active->context->run_tree,t1);
    spec_is_ptr_equal(q->active->prev,NULL);
    _p_addrt2q(q,t2);
    spec_is_equal(q->contexts_count,2);
    spec_is_ptr_equal(q->active->context->run_tree,t2);
    spec_is_ptr_equal(q->active->next->context->run_tree,t1);
    spec_is_ptr_equal(q->active->next->prev, q->active);

    // confirm that they both reduce correctly
    pthread_t thread;
    int rc;
    rc = pthread_create(&thread,0,_p_reduceq_thread,q);
    if (rc){
        raise_error("ERROR; return code from pthread_create() is %d\n", rc);
    }
    void *status;
    rc = pthread_join(thread, &status);
    if (rc) {
        raise_error("ERROR; return code from pthread_join() is %d\n", rc);
    }
    spec_is_long_equal((long)status,noReductionErr);

    spec_is_str_equal(t2s(_t_child(t1,1)),"(TEST_INT_SYMBOL:124)");
    spec_is_str_equal(t2s(_t_child(t2,1)),"(TEST_INT_SYMBOL:123)");

    _p_freeq(q);
    _t_free(processes);_t_free(n);
    //! [testProcessMulti]
}

void testProcess() {
    testRunTree();
    testProcessInterpolateMatch();
    testProcessIf();
    testProcessIntMath();
    testProcessString();
    textProcessRespond();
    testProcessReduce();
    testProcessReduceDefinedProcess();
    testProcessSignatureMatching();
    testProcessError();
    testProcessRaise();
    testProcessErrorTrickleUp();
    testProcessMulti();
}
