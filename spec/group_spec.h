/**
 * @file group_spec.h
 * @copyright Copyright (C) 2013-2015, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 * @ingroup tests
 */

#include "../src/group.h"

void testGroupCreate() {
    Receptor *r = makeGroup(G_vm,"ceptr chat");
    Receptor *m = _r_new(G_sem,TEST_RECEPTOR);
    Xaddr mx = _v_new_receptor(G_vm,G_vm->r,TEST_RECEPTOR,m);
    _v_activate(G_vm,mx);

    spec_is_str_equal(_td(r,_t_child(r->root,1)),"(INSTANCE_OF:ceptr chat)");
    spec_is_str_equal(_td(r,r->flux),"(FLUX (DEFAULT_ASPECT (EXPECTATIONS (EXPECTATION (CARRIER:enrollment) (PATTERN (SEMTREX_SYMBOL_ANY)) (ACTION:enroll) (PARAMS) (END_CONDITIONS (UNLIMITED))) (EXPECTATION (CARRIER:converse) (PATTERN (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:MESSAGE))) (ACTION:group_listen) (PARAMS) (END_CONDITIONS (UNLIMITED)))) (SIGNALS)))");

    // bindings for m to request membership
    T *bindings = _t_new_root(PROTOCOL_BINDINGS);
    T *res = _t_newr(bindings,RESOLUTION);
    T *w = _t_newr(res,WHICH_RECEPTOR);
    _t_news(w,ROLE,GROUP);
    __r_make_addr(w,ACTUAL_RECEPTOR,r->addr);
    res = _t_newr(bindings,RESOLUTION);
    w = _t_newr(res,WHICH_RECEPTOR);
    _t_news(w,ROLE,MEMBER);
    __r_make_addr(w,ACTUAL_RECEPTOR,m->addr);

    // @todo bleah, this should be a better proc, at least with a SIGNAL_REF
    // or something.
    T *noop = _t_new_root(NOOP);
    _t_newi(noop,TEST_INT_SYMBOL,314);
    Process proc = _r_define_process(m,noop,"do nothing","long desc...",NULL);

    T *bindings2 = _t_build(G_sem,0,PROTOCOL_BINDINGS,RESOLUTION,WHICH_PROCESS,GOAL,HANDLER,ACTUAL_PROCESS,proc,NULL_SYMBOL,NULL_SYMBOL,NULL_SYMBOL);

    _o_express_role(m,group1,MEMBER,DEFAULT_ASPECT,bindings2);
    _t_free(bindings2);
    _o_initiate(m,group1,enrollment,bindings);

    _p_reduceq(m->q);
    _v_deliver_signals(G_vm,m);
    _p_reduceq(r->q);

    // see that member got added to the group
    spec_is_str_equal(_td(r,_t_child(__r_get_signals(r,DEFAULT_ASPECT),1)),"(SIGNAL (ENVELOPE (FROM_ADDRESS (RECEPTOR_ADDR:4)) (TO_ADDRESS (RECEPTOR_ADDR:3)) (ASPECT_IDENT:DEFAULT_ASPECT) (CARRIER:enrollment) (SIGNAL_UUID)) (BODY:{(YUP)}) (RUN_TREE (NEW_XADDR:MEMBER.1) (PARAMS)))");

    T *t = _t_new_root(ITERATION_DATA);
    _a_get_instances(&r->instances,MEMBER,t);
    spec_is_str_equal(t2s(t),"(ITERATION_DATA (MEMBER (RECEPTOR_ADDR:4)))");
    _t_free(t);

    res = _t_newr(bindings,RESOLUTION);
    w = _t_newr(res,WHICH_VALUE);
    _t_news(w,USAGE,MESSAGE);
    T *val = _t_newr(w,ACTUAL_VALUE);
    val = _t_new_str(val,MESSAGE,"hi there!");


    //debug_enable(D_PROTOCOL+D_SIGNALS+D_TREE);
    _o_initiate(m,group1,converse,bindings);
    /* _test_reduce_signals(r); */
    _p_reduceq(m->q);
    _v_deliver_signals(G_vm,m);
    _p_reduceq(r->q);
    _v_deliver_signals(G_vm,r);
    _p_reduceq(m->q);

    spec_is_str_equal(_td(r,_t_child(__r_get_signals(r,DEFAULT_ASPECT),2)),"(SIGNAL (ENVELOPE (FROM_ADDRESS (RECEPTOR_ADDR:4)) (TO_ADDRESS (RECEPTOR_ADDR:3)) (ASPECT_IDENT:DEFAULT_ASPECT) (CARRIER:converse) (SIGNAL_UUID)) (BODY:{(MESSAGE:hi there!)}) (RUN_TREE (SIGNAL_UUID) (PARAMS)))");

    t = _t_new_root(ITERATION_DATA);
    _a_get_instances(&r->instances,MESSAGE,t);
    spec_is_str_equal(t2s(t),"(ITERATION_DATA (MESSAGE:hi there!))");
    _t_free(t);

    t = _t_new_root(ITERATION_DATA);
    _a_get_instances(&r->instances,MEMBER,t);
    spec_is_str_equal(t2s(t),"(ITERATION_DATA (MEMBER (RECEPTOR_ADDR:4)))");
    _t_free(t);

    spec_is_str_equal(_td(m,_t_child(__r_get_signals(m,DEFAULT_ASPECT),1)),"(SIGNAL (ENVELOPE (FROM_ADDRESS (RECEPTOR_ADDR:3)) (TO_ADDRESS (RECEPTOR_ADDR:4)) (ASPECT_IDENT:DEFAULT_ASPECT) (CARRIER:converse) (SIGNAL_UUID)) (BODY:{(MESSAGE:hi there!)}) (RUN_TREE (TEST_INT_SYMBOL:314) (PARAMS)))");
    debug_disable(D_PROTOCOL+D_SIGNALS);

    _t_free(bindings);
    // @todo find bug that causes _r_free to freak out, seems like a loop in the tree
    // _r_free(r);
    // _r_free(m);
}

void testGroup() {
    G_vm = _v_new();
    testGroupCreate();
    _v_free(G_vm);
}
