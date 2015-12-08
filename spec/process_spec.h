/**
 * @file process_spec.h
 * @copyright Copyright (C) 2013-2015, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 * @ingroup tests
 */

#include "../src/ceptr.h"
#include "../src/process.h"

void testRunTree() {
    T *processes = __sem_get_defs(G_sem,SEM_TYPE_PROCESS,TEST_CONTEXT);
    T *code,*signature;

    // a process that would look something like this in lisp:
    // (defun my_if (true_branch false_branch condition) (if (condition) (true_branch) (false_branch)))
    code = _t_new_root(IF);                             // IF is a system process
    int pt1[] = {2,1,TREE_PATH_TERMINATOR};
    int pt2[] = {2,2,TREE_PATH_TERMINATOR};
    int pt3[] = {2,3,TREE_PATH_TERMINATOR};

    _t_new(code,PARAM_REF,pt3,sizeof(int)*4);
    _t_new(code,PARAM_REF,pt1,sizeof(int)*4);
    _t_new(code,PARAM_REF,pt2,sizeof(int)*4);

    T *xsignature = __p_make_signature("result",SIGNATURE_PASSTHRU,NULL_STRUCTURE,
                                   "condition",SIGNATURE_PROCESS,BOOLEAN,
                                   "true_branch",SIGNATURE_ANY,NULL_STRUCTURE,
                                   "false_branch",SIGNATURE_ANY,NULL_STRUCTURE,
                                   NULL);
    char buf[1000];
    __t_dump(G_sem,xsignature,NO_INDENT,buf);
    _t_free(xsignature);

    signature = _t_new_root(PROCESS_SIGNATURE);
    T *o = _t_newr(signature,OUTPUT_SIGNATURE);
    _t_new_str(o,SIGNATURE_LABEL,"result");
    _t_news(o,SIGNATURE_PASSTHRU,NULL_STRUCTURE);
    T *i3 = _t_newr(signature,INPUT_SIGNATURE);
    _t_new_str(i3,SIGNATURE_LABEL,"condition");
    _t_news(i3,SIGNATURE_PROCESS,BOOLEAN); // a process that returns a boolean
    T *i1 = _t_newr(signature,INPUT_SIGNATURE);
    _t_new_str(i1,SIGNATURE_LABEL,"true_branch");
    _t_news(i1,SIGNATURE_ANY,NULL_STRUCTURE);
    T *i2 = _t_newr(signature,INPUT_SIGNATURE);
    _t_new_str(i2,SIGNATURE_LABEL,"false_branch");
    _t_news(i2,SIGNATURE_ANY,NULL_STRUCTURE);

    // test that __p_make_signature does what we think it should
    spec_is_str_equal(buf,t2s(signature));

    Process p = _d_define_process(G_sem,code,"myif","a duplicate of the sys if process with params in different order",signature,TEST_CONTEXT);

    T *p3 = _t_newi(0,BOOLEAN,1);
    T *p1 = _t_newi(0,TEST_INT_SYMBOL,123);
    T *p2 = _t_newi(0,TEST_INT_SYMBOL,321);

    T *act = _t_newp(0,ACTION,p);

    T *r = _p_make_run_tree(processes,act,3,p1,p2,p3);

    spec_is_symbol_equal(0,_t_symbol(r),RUN_TREE);

    T *t = _t_child(r,1);  // first child should be clone of code
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

    spec_is_equal(_p_reduce(G_sem,r),noReductionErr);

    spec_is_str_equal(t2s(_t_child(r,1)),"(TEST_INT_SYMBOL:123)");

    _t_free(act);
    _t_free(r);

    // you can also create a run tree with a system process
    act = _t_newp(0,ACTION,ADD_INT);

    r = _p_make_run_tree(processes,act,2,p1,p2);
    spec_is_str_equal(t2s(r),"(RUN_TREE (process:ADD_INT (TEST_INT_SYMBOL:123) (TEST_INT_SYMBOL:321)))");

    _t_free(act);
    _t_free(r);
    _t_free(p1);
    _t_free(p2);
    _t_free(p3);
}

//-----------------------------------------------------------------------------------------
// tests of system processes

void testProcessGet() {
    Receptor *r = _r_new(G_sem,TEST_RECEPTOR);
    Q *q = r->q;

    T *t = _t_newi(0,TEST_INT_SYMBOL,314);
    Xaddr x = _r_new_instance(r,t);

    T *n = _t_newr(0,GET);
    _t_new(n,GET_XADDR,&x,sizeof(Xaddr));
    T *run_tree = __p_build_run_tree(n,0);
    _t_free(n);
    Qe *e = _p_addrt2q(q,run_tree);
    spec_is_equal(_p_reduceq(q),noReductionErr);

    spec_is_str_equal(t2s(run_tree),"(RUN_TREE (TEST_INT_SYMBOL:314) (PARAMS))");

    _r_free(r);
}

void testProcessInterpolateMatch() {
    T *t = _t_new_root(RUN_TREE);
    // test INTERPOLATE_FROM_MATCH which takes three params, the tree to interpolate, the stx-match and the tree it matched on
    T *n = _t_new_root(INTERPOLATE_FROM_MATCH);
    T *p1 = _t_newr(n,TEST_ANYTHING_SYMBOL);
    _t_news(p1,INTERPOLATE_SYMBOL,TEST_INT_SYMBOL2);
    T *p2 = _t_newi(n,SEMTREX_MATCH,1);
    _t_news(p2,SEMTREX_MATCH,TEST_INT_SYMBOL2);
    int path[] = {TREE_PATH_TERMINATOR};
    _t_new(p2,SEMTREX_MATCH_PATH,path,2*sizeof(int));
    _t_newi(p2,SEMTREX_MATCH_SIBLINGS_COUNT,1);
    T *p3 = _t_newi(n,TEST_INT_SYMBOL,314);

    T *c = _t_rclone(n);
    _t_add(t,c);
    _p_reduce(G_sem,t);

    spec_is_str_equal(t2s(_t_child(t,1)),"(TEST_ANYTHING_SYMBOL (TEST_INT_SYMBOL2:314))");
    _t_free(t);
    _t_free(n);
}

void testProcessInterpolateMatchFull() {
    T *t = _t_new_root(RUN_TREE);
    // test INTERPOLATE_FROM_MATCH which takes three params, the tree to interpolate, the stx-match and the tree it matched on
    T *n = _t_new_root(INTERPOLATE_FROM_MATCH);
    T *p1 = _t_newr(n,TEST_ANYTHING_SYMBOL);
    _t_news(p1,INTERPOLATE_SYMBOL,NULL_SYMBOL);
    T *p2 = _t_newi(n,SEMTREX_MATCH,1);
    _t_news(p2,SEMTREX_MATCH,NULL_SYMBOL);
    int path[] = {TREE_PATH_TERMINATOR};
    _t_new(p2,SEMTREX_MATCH_PATH,path,2*sizeof(int));
    _t_newi(p2,SEMTREX_MATCH_SIBLINGS_COUNT,1);
    T *p3 = _t_newi(n,TEST_INT_SYMBOL,314);

    T *c = _t_rclone(n);
    _t_add(t,c);
    _p_reduce(G_sem,t);

    spec_is_str_equal(t2s(_t_child(t,1)),"(TEST_ANYTHING_SYMBOL (TEST_INT_SYMBOL:314))");
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

    __p_reduce_sys_proc(0,IF,n,0);
    spec_is_str_equal(t2s(n),"(TEST_INT_SYMBOL:99)");

    _t_free(n);
}

void testProcessIntMath() {
    T *t;

    // test addition
    T *n = _t_new_root(ADD_INT);
    _t_newi(n,TEST_INT_SYMBOL,99);
    _t_newi(n,TEST_INT_SYMBOL,100);
    __p_reduce_sys_proc(0,ADD_INT,n,0);
    spec_is_str_equal(t2s(n),"(TEST_INT_SYMBOL:199)");
    _t_free(n);

    /// @todo structure type checking for integer math?  Too expensive?
    // where do we get the defs.  Right now they are referred to in the process q
    // which the contexts don't have direct access to.
    /* n = _t_new_root(ADD_INT); */
    /* spec_is_sem_equal(_sem_get_symbol_structure(G_sem,s),INTEGER); */
    /* _t_newi(n,TEST_INT_SYMBOL,99); */
    /* _t_news(n,INTERPOLATE_SYMBOL,TEST_INT_SYMBOL); */
    /* spec_is_equal(__p_reduce_sys_proc(0,ADD_INT,n,0),incompatibleTypeReductionErr); */
    /* spec_is_str_equal(t2s(n),"(TEST_INT_SYMBOL:199)"); */
    /* _t_free(n); */

    // test subtraction
    n = _t_new_root(SUB_INT);
    _t_newi(n,TEST_INT_SYMBOL,100);
    _t_newi(n,TEST_INT_SYMBOL,98);
    __p_reduce_sys_proc(0,SUB_INT,n,0);
    spec_is_str_equal(t2s(n),"(TEST_INT_SYMBOL:2)");
    _t_free(n);

    // test multiplication
    n = _t_new_root(MULT_INT);
    _t_newi(n,TEST_INT_SYMBOL,100);
    _t_newi(n,TEST_INT_SYMBOL,98);
    __p_reduce_sys_proc(0,MULT_INT,n,0);
    spec_is_str_equal(t2s(n),"(TEST_INT_SYMBOL:9800)");
    _t_free(n);

    // test division
    n = _t_new_root(DIV_INT);
    _t_newi(n,TEST_INT_SYMBOL,100);
    _t_newi(n,TEST_INT_SYMBOL,48);
    __p_reduce_sys_proc(0,DIV_INT,n,0);
    spec_is_str_equal(t2s(n),"(TEST_INT_SYMBOL:2)");
    _t_free(n);

    // test division with divide by zero
    n = _t_new_root(DIV_INT);
    _t_newi(n,TEST_INT_SYMBOL,100);
    _t_newi(n,TEST_INT_SYMBOL,0);
    spec_is_equal(__p_reduce_sys_proc(0,DIV_INT,n,0),divideByZeroReductionErr);
    _t_free(n);

    // test modulo
    n = _t_new_root(MOD_INT);
    _t_newi(n,TEST_INT_SYMBOL,100);
    _t_newi(n,TEST_INT_SYMBOL,2);
    __p_reduce_sys_proc(0,MOD_INT,n,0);
    spec_is_str_equal(t2s(n),"(TEST_INT_SYMBOL:0)");
    _t_free(n);

    // test modulo with divide by zero
    n = _t_new_root(MOD_INT);
    _t_newi(n,TEST_INT_SYMBOL,100);
    _t_newi(n,TEST_INT_SYMBOL,0);
    spec_is_equal(__p_reduce_sys_proc(0,MOD_INT,n,0),divideByZeroReductionErr);
    _t_free(n);

    // test equals
    n = _t_new_root(EQ_INT);
    _t_newi(n,TEST_INT_SYMBOL,100);
    _t_newi(n,TEST_INT_SYMBOL,2);
    __p_reduce_sys_proc(0,EQ_INT,n,0);
    spec_is_str_equal(t2s(n),"(BOOLEAN:0)");
    _t_free(n);

    n = _t_new_root(EQ_INT);
    _t_newi(n,TEST_INT_SYMBOL,100);
    _t_newi(n,TEST_INT_SYMBOL,100);
    __p_reduce_sys_proc(0,EQ_INT,n,0);
    spec_is_str_equal(t2s(n),"(BOOLEAN:1)");
    _t_free(n);

    // test <
    n = _t_new_root(LT_INT);
    _t_newi(n,TEST_INT_SYMBOL,2);
    _t_newi(n,TEST_INT_SYMBOL,100);
    __p_reduce_sys_proc(0,LT_INT,n,0);
    spec_is_str_equal(t2s(n),"(BOOLEAN:1)");
    _t_free(n);

    n = _t_new_root(LT_INT);
    _t_newi(n,TEST_INT_SYMBOL,100);
    _t_newi(n,TEST_INT_SYMBOL,100);
    __p_reduce_sys_proc(0,LT_INT,n,0);
    spec_is_str_equal(t2s(n),"(BOOLEAN:0)");
   _t_free(n);

    // test >
    n = _t_new_root(GT_INT);
    _t_newi(n,TEST_INT_SYMBOL,2);
    _t_newi(n,TEST_INT_SYMBOL,100);
    __p_reduce_sys_proc(0,GT_INT,n,0);
    spec_is_str_equal(t2s(n),"(BOOLEAN:0)");
    _t_free(n);

    n = _t_new_root(GT_INT);
    _t_newi(n,TEST_INT_SYMBOL,101);
    _t_newi(n,TEST_INT_SYMBOL,100);
    __p_reduce_sys_proc(0,GT_INT,n,0);
    spec_is_str_equal(t2s(n),"(BOOLEAN:1)");
    _t_free(n);

}

void testProcessString() {

    // test string concatenation
    T *n = _t_new_root(CONCAT_STR);
    _t_news(n,RESULT_SYMBOL,TEST_NAME_SYMBOL);
    _t_new_str(n,TEST_STR_SYMBOL,"Fred");
    _t_new_str(n,TEST_STR_SYMBOL," ");
    _t_new_str(n,TEST_STR_SYMBOL,"Smith");

    __p_reduce_sys_proc(0,CONCAT_STR,n,0);

    spec_is_str_equal(t2s(n),"(TEST_NAME_SYMBOL:Fred Smith)");

    _t_free(n);
}

void testProcessRespond() {
    // testing responding to a signal requires setting up a sending signal context

    T *signal_contents = _t_newi(0,TEST_INT_SYMBOL,314);
    ReceptorAddress f = {3}; // DUMMY ADDR
    ReceptorAddress t = {4}; // DUMMY ADDR

    T *s = __r_make_signal(f,t,DEFAULT_ASPECT,TESTING,signal_contents,0,defaultRequestUntil());

    T *run_tree = _t_new_root(RUN_TREE);
    T *n = _t_newr(run_tree,RESPOND);
    _t_news(n,CARRIER,TESTING);

    T *response_contents = _t_newi(n,TEST_INT_SYMBOL,271);

    R *c = __p_make_context(run_tree,0,0);

    // if this is a run-tree that's not a child of a signal we can't respond!
    spec_is_equal(__p_reduce_sys_proc(c,RESPOND,n,0),notInSignalContextReductionError);

    // now add it to the signal and try again
    _t_add(s,run_tree);

    Receptor *r = _r_new(G_sem,TEST_RECEPTOR);

    // it should create a response signal with the source UUID as the responding to UUID
    spec_is_equal(__p_reduce_sys_proc(c,RESPOND,n,r->q),noReductionErr);
    spec_is_str_equal(t2s(n),"(SIGNAL_UUID)");
    spec_is_str_equal(_td(r,r->pending_signals),"(PENDING_SIGNALS (SIGNAL (ENVELOPE (FROM_ADDRESS (RECEPTOR_ADDR:4)) (TO_ADDRESS (RECEPTOR_ADDR:3)) (ASPECT_IDENT:DEFAULT_ASPECT) (CARRIER:TESTING) (SIGNAL_UUID) (IN_RESPONSE_TO_UUID)) (BODY:{(TEST_INT_SYMBOL:271)})))");
    T *u1 = _t_child(_t_child(s,SignalEnvelopeIdx),EnvelopeUUIDIdx);
    int p[] = {1,SignalEnvelopeIdx,EnvelopeExtraIdx,TREE_PATH_TERMINATOR};
    T *u2 = _t_get(r->pending_signals,p);
    spec_is_true(__uuid_equal(_t_surface(u1),_t_surface(u2)));

    _p_free_context(c);
    _t_free(s);

    _r_free(r);
}
extern int G_next_process_id;

void testProcessSay() {
    T *p = _t_newr(0,SAY);
    ReceptorAddress to = {99}; // DUMMY ADDR

    __r_make_addr(p,TO_ADDRESS,to);
    _t_news(p,ASPECT_IDENT,DEFAULT_ASPECT);
    _t_news(p,CARRIER,TESTING);
    _t_newi(p,TEST_INT_SYMBOL,314);

    Receptor *r = _r_new(G_sem,TEST_RECEPTOR);

    T *run_tree = __p_build_run_tree(p,0);
    _t_free(p);

    // add the run tree into a queue and run it
    G_next_process_id = 0; // reset the process ids so the test will always work
    Q *q = r->q;
    T *ps = r->pending_signals;
    Qe *e =_p_addrt2q(q,run_tree);
    R *c = e->context;

    // after reduction the context should be in the blocked state
    // and the signal should be on the pending signals list
    // the UUID should be in pending responses list
    spec_is_equal(_p_reduceq(q),noReductionErr);
    spec_is_equal(q->contexts_count,0);

    // say reduces to the UUID generated for the sent signal
    spec_is_str_equal(t2s(run_tree),"(RUN_TREE (SIGNAL_UUID) (PARAMS))");
    spec_is_str_equal(t2s(ps),"(PENDING_SIGNALS (SIGNAL (ENVELOPE (FROM_ADDRESS (RECEPTOR_ADDR:3)) (TO_ADDRESS (RECEPTOR_ADDR:99)) (ASPECT_IDENT:DEFAULT_ASPECT) (CARRIER:TESTING) (SIGNAL_UUID)) (BODY:{(TEST_INT_SYMBOL:314)})))");
    _r_free(r);
}

void testProcessRequest() {
    T *p = _t_newr(0,REQUEST);
    ReceptorAddress to = {99}; // DUMMY ADDR

    __r_make_addr(p,TO_ADDRESS,to);
    _t_news(p,ASPECT_IDENT,DEFAULT_ASPECT);
    _t_news(p,CARRIER,TESTING);
    _t_newi(p,TEST_INT_SYMBOL,314);
    _t_news(p,RESPONSE_CARRIER,TESTING);
    T *ec = _t_newr(p,END_CONDITIONS);
    _t_newi(ec,COUNT,1);

    T *code =_t_rclone(p);
    Receptor *r = _r_new(G_sem,TEST_RECEPTOR);

    T *run_tree = __p_build_run_tree(code,0);

    // add the run tree into a queue and run it
    G_next_process_id = 0; // reset the process ids so the test will always work
    Q *q = r->q;
    T *ps = r->pending_signals;
    Qe *e =_p_addrt2q(q,run_tree);
    R *c = e->context;

    // after reduction the context should be in the blocked state
    // and the signal should be on the pending signals list
    // the UUID should be in pending responses list
    spec_is_equal(_p_reduceq(q),noReductionErr);
    spec_is_equal(q->contexts_count,0);
    spec_is_ptr_equal(q->blocked,e);
    spec_is_equal(c->state,Block);
    spec_is_str_equal(_td(r,r->pending_responses),"(PENDING_RESPONSES (PENDING_RESPONSE (SIGNAL_UUID) (CARRIER:TESTING) (WAKEUP_REFERENCE (PROCESS_IDENT:1) (CODE_PATH:/1)) (END_CONDITIONS (COUNT:1))))");

    // request reduces to the UUID generated for the sent signal
    spec_is_str_equal(t2s(run_tree),"(RUN_TREE (SIGNAL_UUID) (PARAMS))");
    spec_is_str_equal(t2s(ps),"(PENDING_SIGNALS (SIGNAL (ENVELOPE (FROM_ADDRESS (RECEPTOR_ADDR:3)) (TO_ADDRESS (RECEPTOR_ADDR:99)) (ASPECT_IDENT:DEFAULT_ASPECT) (CARRIER:TESTING) (SIGNAL_UUID) (END_CONDITIONS (COUNT:1))) (BODY:{(TEST_INT_SYMBOL:314)})))");

    // debug_enable(D_SIGNALS);
    // generate a response signal

    T *s = __r_make_signal(r->addr,r->addr,DEFAULT_ASPECT,TESTING,_t_new_str(0,TEST_STR_SYMBOL,"one fish"),_t_surface(_t_child(run_tree,1)),0);
    _r_deliver(r,s);
    spec_is_str_equal(_td(r,r->pending_responses),"(PENDING_RESPONSES)");

    //   _p_freeq(q);
    // clear off the signal in the list
    _t_free(_t_detach_by_idx(r->pending_signals,1));

    /* // now test the callback request case */
    /* _t_newi(p,BOOLEAN,1); */

    /* run_tree = __p_build_run_tree(p,0); */

    /* r->q = q = _p_newq(r); */
    /* ps = r->pending_signals; */
    /* e = _p_addrt2q(q,run_tree); */
    /* c = e->context; */

    /* // after reduction the context should have been moved to the completed list */
    /* // and the signal should be on the pending signals list */
    /* spec_is_equal(_p_reduceq(q),noReductionErr); */

    /* spec_is_equal(q->contexts_count,0); */
    /* spec_is_ptr_equal(q->completed,e); */

    /* spec_is_str_equal(t2s(run_tree),"(RUN_TREE (SIGNAL_UUID) (PARAMS))"); */
    /* spec_is_str_equal(t2s(ps),"(PENDING_SIGNALS (SIGNAL (ENVELOPE (FROM_ADDRESS (CONTEXT_NUM:0)) (TO_ADDRESS (CONTEXT_NUM:3)) (DEFAULT_ASPECT) (CARRIER:TEST_INT_SYMBOL) (SIGNAL_UUID)) (BODY:{(TEST_INT_SYMBOL:314)})))"); */

    debug_disable(D_SIGNALS);

    _r_free(r);
    _t_free(p);
    _t_free(code);
}

void testProcessQuote() {
    T *t = _t_new_root(RUN_TREE);

    // test process quoting
    T *n = _t_new_root(QUOTE);

    T *a = _t_newr(n,ADD_INT);
    _t_newi(a,TEST_INT_SYMBOL,99);
    _t_newi(a,TEST_INT_SYMBOL,100);

    T *c = _t_rclone(n);
    _t_add(t,c);
    _p_reduce(G_sem,t);

    spec_is_str_equal(t2s(t),"(RUN_TREE (process:ADD_INT (TEST_INT_SYMBOL:99) (TEST_INT_SYMBOL:100)))");

    _t_free(n);
    _t_free(t);
}

void testProcessStream() {
    Receptor *r = _r_new(G_sem,TEST_RECEPTOR);
    Q *q = r->q;

    FILE *stream;
    char buffer[500] = "line1\nline2\n";
    stream = fmemopen(buffer, 500, "r+");

    // test the basic case of the STREAM_ALIVE process which returns
    // a boolean if the stream is readable or not.
    T *n = _t_new_root(STREAM_ALIVE);
    Stream *st = _st_new_unix_stream(stream,1);
    _t_new_stream(n,TEST_STREAM_SYMBOL,st);
    __p_reduce_sys_proc(0,STREAM_ALIVE,n,0);
    spec_is_str_equal(t2s(n),"(BOOLEAN:1)");
    _t_free(n);

    // test reading a stream
    n = _t_new_root(STREAM_READ);
    _t_new_stream(n,TEST_STREAM_SYMBOL,st);
    _t_news(n,RESULT_SYMBOL,TEST_STR_SYMBOL);

    T *run_tree = __p_build_run_tree(n,0);
    _t_free(n);
    Qe *e = _p_addrt2q(q,run_tree);
    R *c = e->context;

    // after reduction the context should be in the blocked state

    spec_is_equal(_p_reduceq(q),noReductionErr);
    spec_is_equal(q->contexts_count,0);
    spec_is_ptr_equal(q->blocked,e);
    spec_is_equal(c->state,Block);

    // wait for read to complete, after which it should have also unblocked the
    // context which should thus be ready for reduction again.
    while(!(st->flags&StreamHasData) && st->flags&StreamAlive ) {sleepms(1);};
    spec_is_equal(q->contexts_count,1);

    spec_is_equal(_p_reduceq(q),noReductionErr);
    spec_is_str_equal(t2s(run_tree),"(RUN_TREE (TEST_STR_SYMBOL:line1) (PARAMS))");

    // test writing to the stream
    n = _t_new_root(STREAM_WRITE);
    _t_new_stream(n,TEST_STREAM_SYMBOL,st);
    _t_new_str(n,TEST_STR_SYMBOL,"fish\n");
    _t_new_str(n,LINE,"cow");
    _t_newi(n,TEST_INT_SYMBOL,314);

    run_tree = __p_build_run_tree(n,0);
    _t_free(n);
    e = _p_addrt2q(q,run_tree);

    spec_is_equal(_p_reduceq(q),noReductionErr);

    spec_is_str_equal(buffer,"line1\nfish\ncow\n(TEST_INT_SYMBOL:314)\n");

    _st_free(st);

    // test writing to a readonly stream
    stream = fmemopen(buffer, strlen (buffer), "r");
    st = _st_new_unix_stream(stream,0);
    n = _t_new_root(STREAM_WRITE);

    _t_new_stream(n,TEST_STREAM_SYMBOL,st);
    _t_new_str(n,TEST_STR_SYMBOL,"fish\n");

    run_tree = __p_build_run_tree(n,0);
    _t_free(n);
    e = _p_addrt2q(q,run_tree);
    spec_is_equal(_p_reduceq(q),noReductionErr);

    // context should have recorded the err
    spec_is_equal(e->context->err,unixErrnoReductionErr);

    // buffer should remain unchanged
    spec_is_str_equal(buffer,"line1\nfish\ncow\n(TEST_INT_SYMBOL:314)\n");

    while ((fgetc (stream)) != EOF);
    n = _t_new_root(STREAM_ALIVE);
    _t_new_stream(n,TEST_STREAM_SYMBOL,st);
    __p_reduce_sys_proc(0,STREAM_ALIVE,n,0);
    spec_is_str_equal(t2s(n),"(BOOLEAN:0)");
    _t_free(n);

    _st_free(st);
    _r_free(r);
}

//-----------------------------------------------------------------------------------------
// tests of process execution (reduction)

void testProcessReduce() {
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

    R *context = __p_make_context(t,0,99);

    spec_is_equal(context->id,99);

    // build a fake Receptor and Q on the stack so _p_step will work
    Receptor r;
    Q q;
    r.root = NULL;
    r.q = &q;
    q.r = &r;

    spec_is_equal(rt_cur_child(c),0);
    // first step is Eval and next step is Descend
    spec_is_equal(_p_step(&q,&context),Descend);
    spec_is_equal(rt_cur_child(c),0);

    // next step after Descend changes node pointer and moved to Eval
    spec_is_equal(_p_step(&q,&context),Eval);
    spec_is_ptr_equal(context->node_pointer,_t_child(c,1));
    spec_is_equal(rt_cur_child(c),1);

    // after Eval next step will be Ascend
    spec_is_equal(_p_step(&q,&context),Ascend);

    // step ascends back to top if and marks boolean complete
    spec_is_equal(_p_step(&q,&context),Eval);

    spec_is_equal(rt_cur_child(_t_child(c,1)),RUN_TREE_EVALUATED);
    spec_is_ptr_equal(context->node_pointer,c);

    spec_is_equal(_p_step(&q,&context),Descend);

    // third step goes into the second level if
    spec_is_equal(_p_step(&q,&context),Eval);
    spec_is_equal(rt_cur_child(c),2);
    spec_is_ptr_equal(context->node_pointer,_t_child(c,2));

    free(context);
    // not specing out all the steps because there are soooo many...

    // just re-running them all for final result
    _t_free(_t_detach_by_idx(t,1));
    c = _t_rclone(n);
    _t_add(t,c);
    _p_reduce(G_sem,t);

    spec_is_str_equal(t2s(c),"(TEST_INT_SYMBOL:99)");

    _t_free(n);
    _t_free(t);
}

void testProcessRefs() {
    Receptor *r = _r_new(G_sem,TEST_RECEPTOR);
    Q *q = r->q;

    int pt1[] = {2,1,TREE_PATH_TERMINATOR};
    int pt2[] = {SignalBodyIdx,0,TREE_PATH_TERMINATOR};

    T *n = _t_newr(0,NOOP);
    T *t = _t_newr(n,TEST_ANYTHING_SYMBOL);
    _t_new(t,PARAM_REF,pt1,sizeof(int)*4);
    _t_new(t,SIGNAL_REF,pt2,sizeof(int)*4);
    t = _t_newi(0,TEST_INT_SYMBOL,314);  // a param to the run tree
    T *run_tree = __p_build_run_tree(n,1,t);
    _t_free(n);
    _t_free(t);
    ReceptorAddress fm = {3}; // DUMMY ADDR
    ReceptorAddress to = {4}; // DUMMY ADDR
    T *signal = __r_make_signal(fm,to,DEFAULT_ASPECT,TESTING,_t_new_str(0,TEST_STR_SYMBOL,"foo"),0,0);

    // simulate that this run-tree is on the flux.
    _t_add(signal,run_tree);

    Qe *e = _p_addrt2q(q,run_tree);
    spec_is_equal(_p_reduceq(q),noReductionErr);

    spec_is_str_equal(t2s(run_tree), "(RUN_TREE (TEST_ANYTHING_SYMBOL (TEST_INT_SYMBOL:314) (TEST_STR_SYMBOL:foo)) (PARAMS (TEST_INT_SYMBOL:314)))");

    _r_free(r);
}

/**
 * helper to generate an example process definition that acts as an if for even numbers
 *
 * @snippet spec/process_spec.h defIfEven
 */
//! [defIfEven]
Process _defIfEven() {
    T *code;

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

    T *signature = __p_make_signature("result",SIGNATURE_PASSTHRU,NULL_STRUCTURE,
                                      "val",SIGNATURE_STRUCTURE,INTEGER,
                                      "true_branch",SIGNATURE_ANY,NULL_STRUCTURE,
                                      "false_branch",SIGNATURE_ANY,NULL_STRUCTURE,
                                      NULL);

    return _d_define_process(G_sem,code,"if even","return 2nd child if even, third if not",signature,TEST_CONTEXT);
}
//! [defIfEven]

/**
 * helper to generate an example process definition that creates a divide by zero error
 *
 * @snippet spec/process_spec.h defDivZero
 */
//! [defDivZero]
Process _defDivZero() {
    T *code;

    /* a process that would look something like this in lisp:
       (defun div_zero (val) (/ val 0))
    */
    code = _t_new_root(DIV_INT);                       // IF is a system process
    _t_newi(code,TEST_INT_SYMBOL,2);
    _t_newi(code,TEST_INT_SYMBOL,0);

    T *signature = __p_make_signature("result",SIGNATURE_SYMBOL,NULL_SYMBOL,
                                      "val",SIGNATURE_STRUCTURE,INTEGER,
                                      NULL);

    return _d_define_process(G_sem,code,"divByZero","create a divide by zero error",signature,TEST_CONTEXT);
}
//! [defDivZero]

void testProcessReduceDefinedProcess() {
    //! [testProcessReduceDefinedProcess]

    Process if_even = _defIfEven();  // add the if_even process to our defs
    T *processes = __sem_get_defs(G_sem,SEM_TYPE_PROCESS,TEST_CONTEXT);

    // check that it dumps nicely, including showing the param_refs as paths
    int p[] = {if_even.id,ProcessDefCodeIdx,TREE_PATH_TERMINATOR};

    spec_is_str_equal(t2s(_t_get(processes,p)),"(process:IF (process:EQ_INT (process:MOD_INT (PARAM_REF:/2/1) (TEST_INT_SYMBOL:2)) (TEST_INT_SYMBOL:0)) (PARAM_REF:/2/2) (PARAM_REF:/2/3))");


    // create a run tree right in the state to "call"this function
    T *n = _t_new_root(PARAMS);
    _t_newi(n,TEST_INT_SYMBOL,99);
    _t_newi(n,TEST_INT_SYMBOL,123);
    _t_newi(n,TEST_INT_SYMBOL,124);
    T *t = __p_make_run_tree(processes,if_even,n);

    // confirm that it reduces correctly
    spec_is_equal(_p_reduce(G_sem,t),noReductionErr);
    spec_is_str_equal(t2s(_t_child(t,1)),"(TEST_INT_SYMBOL:124)");

    _t_free(t);_t_free(n);
    //! [testProcessReduceDefinedProcess]
}

void testProcessSignatureMatching() {
    Process if_even = _defIfEven();

    T *t = _t_new_root(RUN_TREE);
    T *n = _t_new_root(if_even);
    _t_new(n,TEST_STR_SYMBOL,"test",5);  // this should be an INTEGER!!
    _t_newi(n,TEST_INT_SYMBOL,123);
    _t_newi(n,TEST_INT_SYMBOL,124);

    spec_is_equal(__p_check_signature(G_sem,if_even,n),signatureMismatchReductionErr);

    T *c = _t_rclone(n);
    _t_add(t,c);
    spec_is_equal(_p_reduce(G_sem,t),signatureMismatchReductionErr);
    _t_free(_t_detach_by_idx(t,1));

    // too few params
    c = _t_rclone(n);
    _t_add(t,c);
    _t_free(_t_detach_by_idx(c,1));
    spec_is_equal(_p_reduce(G_sem,t),tooFewParamsReductionErr);
    _t_free(_t_detach_by_idx(t,1));

    // add too many params
    c = _t_rclone(n);
    _t_add(t,c);
    __t_newi(c,TEST_INT_SYMBOL,124,1);
    spec_is_equal(_p_reduce(G_sem,t),tooManyParamsReductionErr);

    _t_free(t);
    _t_free(n);
}

void testProcessError() {
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
    __t_new(t,PARAM_REF,pt,sizeof(int)*4,1);

    Error e = _p_reduce(G_sem,t);
    spec_is_equal(e,noReductionErr);
    spec_is_str_equal(t2s(_t_child(t,1)),"(ZERO_DIVIDE_ERR (ERROR_LOCATION:/1/1))");
    _t_free(n);
    _t_free(t);
}

void testProcessRaise() {
    T *n = _t_new_root(RAISE);
    _t_news(n,REDUCTION_ERROR_SYMBOL,ZERO_DIVIDE_ERR);

    spec_is_equal(__p_reduce_sys_proc(0,RAISE,n,0),raiseReductionErr);
    _t_free(n);

    T *t = _t_new_root(RUN_TREE);
    n = _t_new_root(RAISE);
    _t_news(n,REDUCTION_ERROR_SYMBOL,NOT_A_PROCESS_ERR); // pick a random error to raise
    T *c = _t_rclone(n);
    _t_add(t,c);
    T *ps = _t_newr(t,PARAMS);

    // error routine is just a param ref to pass back the error tree
    int pt[] = {4,1,TREE_PATH_TERMINATOR};
    __t_new(t,PARAM_REF,pt,sizeof(int)*4,1);

    Error e = _p_reduce(G_sem,t);
    spec_is_equal(e,noReductionErr);
    spec_is_str_equal(t2s(_t_child(t,1)),"(NOT_A_PROCESS_ERR (ERROR_LOCATION:/1))");
    _t_free(n);
    _t_free(t);
}

void testProcessReplicate() {
    FILE *output;

    char *output_data = NULL;
    size_t size;
    output = open_memstream(&output_data,&size);

    // a replicate process that writes to a stream and 3 times
    T *code = _t_new_root(ITERATE);
    T *params = _t_newr(code,PARAMS);
    _t_newi(code,TEST_INT_SYMBOL,3);

    T *x = _t_newr(code,STREAM_WRITE);
    Stream *st = _st_new_unix_stream(output,0);
    _t_new_stream(x,TEST_STREAM_SYMBOL,st);
    _t_new_str(x,LINE,"testing");

    T *t = __p_build_run_tree(code,0);
    Error e = _p_reduce(G_sem,t);
    spec_is_equal(e,noReductionErr);
    spec_is_str_equal(output_data,"testing\ntesting\ntesting\n");
    //    spec_is_str_equal(t2s(t),"xxx"); @todo something here when we figure out return value

    _t_free(t);

    // now test replication with a condition instead of an INTEGER
    //  a condition that checks to see if the param is less than 3
    x = _t_newr(0,LT_INT);
    int p[] = {1,1,1,TREE_PATH_TERMINATOR};
    _t_new(x,PARAM_REF,p,sizeof(int)*4);
    _t_newi(x,TEST_INT_SYMBOL,3);

    _t_newi(params,TEST_INT_SYMBOL,314);

    _t_replace(code,2,x);

    t = __p_build_run_tree(code,0);
    e = _p_reduce(G_sem,t);
    spec_is_equal(e,noReductionErr);
    spec_is_str_equal(output_data,"testing\ntesting\ntesting\n");

    _st_free(st);
    free(output_data);
    _t_free(code);
    _t_free(t);
}

void testProcessListen() {
    Receptor *r = _r_new(G_sem,TEST_RECEPTOR);

    // test regular asynchronous listening.
    T *n = _t_new_root(LISTEN);
    _t_news(n,ASPECT_IDENT,DEFAULT_ASPECT);
    _t_news(n,CARRIER,TICK);
    T *match = _t_newr(n,PATTERN);
    _sl(match,TICK);
    T *a = _t_newp(n,ACTION,NOOP);
    _t_newi(a,TEST_INT_SYMBOL,314);
    spec_is_equal(__p_reduce_sys_proc(0,LISTEN,n,r->q),noReductionErr);
    spec_is_str_equal(t2s(n),"(REDUCTION_ERROR_SYMBOL:NULL_SYMBOL)"); //@todo is this right??
    _t_free(n);

    T *ex = __r_get_expectations(r,DEFAULT_ASPECT);
    spec_is_str_equal(t2s(ex),"(EXPECTATIONS (EXPECTATION (CARRIER:TICK) (PATTERN (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:TICK))) (ACTION:NOOP (TEST_INT_SYMBOL:314)) (PARAMS (INTERPOLATE_SYMBOL:NULL_SYMBOL)) (END_CONDITIONS (UNLIMITED))))");

    _r_remove_expectation(r, _t_child(ex,1));

    // test listen that blocks
    n = _t_new_root(LISTEN);
    _t_news(n,ASPECT_IDENT,DEFAULT_ASPECT);
    _t_news(n,CARRIER,TESTING);
    match = _t_newr(n,PATTERN);
    _sl(match,TEST_STR_SYMBOL);

    G_next_process_id = 0; // reset the process ids so the test will always work
    T *run_tree = __p_build_run_tree(n,0);
    _t_free(n);
    Q *q = r->q;
    Qe *e = _p_addrt2q(q,run_tree);

    //debug_enable(D_LISTEN+D_SIGNALS);
    spec_is_equal(_p_reduceq(q),noReductionErr);
    spec_is_equal(q->contexts_count,0);
    spec_is_ptr_equal(q->blocked,e);
    spec_is_str_equal(t2s(run_tree),"(RUN_TREE (process:LISTEN) (PARAMS))");

    spec_is_str_equal(t2s(__r_get_expectations(r,DEFAULT_ASPECT)),"(EXPECTATIONS (EXPECTATION (CARRIER:TESTING) (PATTERN (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:TEST_STR_SYMBOL))) (WAKEUP_REFERENCE (PROCESS_IDENT:1) (CODE_PATH:/1)) (PARAMS (INTERPOLATE_SYMBOL:NULL_SYMBOL)) (END_CONDITIONS (COUNT:1))))");

    T *s = __r_make_signal(r->addr,r->addr,DEFAULT_ASPECT,TESTING,_t_new_str(0,TEST_STR_SYMBOL,"fishy!"),0,0);
    _r_deliver(r,s);
    spec_is_equal(_p_reduceq(q),noReductionErr);

    // because this listen blocked, and can thus only be woken up once, the expectation
    // had to have been removed afterwards.
    spec_is_str_equal(t2s(__r_get_expectations(r,DEFAULT_ASPECT)),"(EXPECTATIONS)");

    // @todo, ok the two params thing here is wrong, but we don't actually have
    // a use case for the blocking listen, so I don't quite know how it should work... @FIXME
    spec_is_str_equal(t2s(run_tree),"(RUN_TREE (PARAMS (TEST_STR_SYMBOL:fishy!)) (PARAMS))");
    debug_disable(D_LISTEN);
    _r_free(r);
}

void testProcessErrorTrickleUp() {
    //! [testProcessErrorTrickleUp]
    Process divz = _defDivZero();  // add the if_even process to our defs
    T *processes = __sem_get_defs(G_sem,SEM_TYPE_PROCESS,TEST_CONTEXT);

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
    __t_new(t,PARAM_REF,pt,sizeof(int)*4,1);

    // confirm that it reduces correctly
    spec_is_equal(_p_reduce(G_sem,t),noReductionErr);
    spec_is_str_equal(t2s(_t_child(t,1)),"(ZERO_DIVIDE_ERR (ERROR_LOCATION:/1/1))");

    _t_free(t);_t_free(n);
    //! [testProcessErrorTrickleUp]
}

void testProcessMulti() {
    //! [testProcessMulti]

    Receptor *r = _r_new(G_sem,TEST_RECEPTOR);
    T *processes = __sem_get_defs(G_sem,SEM_TYPE_PROCESS,TEST_CONTEXT);
    Q *q = r->q;

    Process if_even = _defIfEven();  // add the if_even process to our defs

    // create two run trees
    T *n = _t_new_root(PARAMS);
    __t_newi(n,TEST_INT_SYMBOL,99,1);
    __t_newi(n,TEST_INT_SYMBOL,123,1);
    __t_newi(n,TEST_INT_SYMBOL,124,1);
    T *t1 = __p_make_run_tree(processes,if_even,n);

    __t_newi(n,TEST_INT_SYMBOL,100,1);
    T *l2 = __t_new(n,if_even,0,0,1);
    __t_newi(l2,TEST_INT_SYMBOL,2,1);
    __t_newi(l2,TEST_INT_SYMBOL,123,1);
    __t_newi(l2,TEST_INT_SYMBOL,124,1);
    __t_newi(n,TEST_INT_SYMBOL,314,1);

    T *t2 = __p_make_run_tree(processes,if_even,n);

    // add them to a processing queue
    spec_is_equal(q->contexts_count,0);
    spec_is_ptr_equal(q->active,NULL);
    _p_addrt2q(q,t1);
    spec_is_equal(q->contexts_count,1);
    spec_is_ptr_equal(q->active->context->run_tree,t1);
    spec_is_ptr_equal(q->active->prev,NULL);
    _p_addrt2q(q,t2);
    spec_is_equal(q->contexts_count,2);
    spec_is_ptr_equal(q->active->context->run_tree,t1);
    spec_is_ptr_equal(q->active->next->context->run_tree,t2);
    spec_is_ptr_equal(q->active->next->prev, q->active);

    spec_is_long_equal(q->active->accounts.elapsed_time,0);
    spec_is_long_equal(q->active->next->accounts.elapsed_time,0);

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

    // contexts have been moved to the completed list and now have
    // some elapsed time data associated with them.
    spec_is_equal(q->contexts_count,0);
    spec_is_ptr_equal(q->completed->context->run_tree,t2);
    uint64_t et1,et2;
    spec_is_true((et1 = q->completed->accounts.elapsed_time)>0);
    spec_is_ptr_equal(q->completed->next->context->run_tree,t1);
    spec_is_true((et2 = q->completed->next->accounts.elapsed_time)>0);

    // confirm the results of the two run trees
    spec_is_str_equal(t2s(_t_child(t1,1)),"(TEST_INT_SYMBOL:124)");
    spec_is_str_equal(t2s(_t_child(t2,1)),"(TEST_INT_SYMBOL:123)");

    _p_cleanup(q);

    spec_is_ptr_equal(q->completed,NULL);
    // confirm that after cleaning up the two processes, the accounting information
    // is updated in a receptor state structure
    char buf[200];
    sprintf(buf,"(RECEPTOR_ELAPSED_TIME:%ld)",et1+et2);
    spec_is_str_equal(t2s(_t_child(_t_child(r->root,ReceptorInstanceStateIdx),ReceptorElapsedTimeIdx)),buf);

    _t_free(n);
    _r_free(r);
    //! [testProcessMulti]
}

void testRunTreeMaker() {
    T *processes = __sem_get_defs(G_sem,SEM_TYPE_PROCESS,TEST_CONTEXT);

    T *n = _t_new_root(PARAMS);
    _t_newi(n,TEST_INT_SYMBOL,314);
    T *t = __p_make_run_tree(processes,NOOP,n);
    spec_is_str_equal(t2s(t),"(RUN_TREE (process:NOOP (TEST_INT_SYMBOL:314)) (PARAMS))");
    _t_free(t);

    // run-tree maker should also work on defined processes
    Process if_even = _defIfEven();  // add the if_even process to our defs

    _t_free(n);

    n = _t_new_root(PARAMS);
    _t_newi(n,TEST_INT_SYMBOL,99);
    _t_newi(n,TEST_INT_SYMBOL,123);
    _t_newi(n,TEST_INT_SYMBOL,124);

    t = __p_make_run_tree(processes,if_even,n);

    spec_is_str_equal(t2s(t),"(RUN_TREE (process:IF (process:EQ_INT (process:MOD_INT (PARAM_REF:/2/1) (TEST_INT_SYMBOL:2)) (TEST_INT_SYMBOL:0)) (PARAM_REF:/2/2) (PARAM_REF:/2/3)) (PARAMS (TEST_INT_SYMBOL:99) (TEST_INT_SYMBOL:123) (TEST_INT_SYMBOL:124)))");

    _t_free(t);_t_free(n);
}

void testProcess() {
    testRunTree();
    testProcessGet();
    testProcessInterpolateMatch();
    testProcessInterpolateMatchFull();
    testProcessIf();
    testProcessIntMath();
    testProcessString();
    testProcessRespond();
    testProcessSay();
    testProcessRequest();
    testProcessQuote();
    testProcessStream();
    testProcessReduce();
    testProcessRefs();
    testProcessReduceDefinedProcess();
    testProcessSignatureMatching();
    testProcessError();
    testProcessRaise();
    testProcessReplicate();
    testProcessListen();
    testProcessErrorTrickleUp();
    testProcessMulti();
    testRunTreeMaker();
}
