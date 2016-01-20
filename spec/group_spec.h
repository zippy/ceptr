/**
 * @file group_spec.h
 * @copyright Copyright (C) 2013-2015, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 * @ingroup tests
 */

#include "../src/group.h"

void testGroupCreate() {
    Receptor *r = makeGroup(G_vm,"ceptr chat");
    spec_is_str_equal(_td(r,_t_child(r->root,1)),"(INSTANCE_OF:ceptr chat)");
    spec_is_str_equal(_td(r,r->flux),"(FLUX (DEFAULT_ASPECT (EXPECTATIONS (EXPECTATION (CARRIER:enroll) (PATTERN (SEMTREX_SYMBOL_ANY)) (ACTION:enroll_client) (PARAMS) (END_CONDITIONS (UNLIMITED)))) (SIGNALS)))");

    // these make the group receptor act as a member of itself
    T *bindings = _t_new_root(PROTOCOL_BINDINGS);
    T *res = _t_newr(bindings,RESOLUTION);
    T *w = _t_newr(res,WHICH_RECEPTOR);
    _t_news(w,ROLE,GROUP);
    __r_make_addr(w,ACTUAL_RECEPTOR,r->addr);
    res = _t_newr(bindings,RESOLUTION);
    w = _t_newr(res,WHICH_RECEPTOR);
    _t_news(w,ROLE,MEMBER);
    __r_make_addr(w,ACTUAL_RECEPTOR,r->addr);

    _o_initiate(r,group1,enroll,bindings);

    _test_reduce_signals(r);

    // see that member got added to the group
    spec_is_str_equal(_td(r,r->flux),"(FLUX (DEFAULT_ASPECT (EXPECTATIONS (EXPECTATION (CARRIER:enroll) (PATTERN (SEMTREX_SYMBOL_ANY)) (ACTION:enroll_client) (PARAMS) (END_CONDITIONS (UNLIMITED)))) (SIGNALS (SIGNAL (ENVELOPE (FROM_ADDRESS (RECEPTOR_ADDR:3)) (TO_ADDRESS (RECEPTOR_ADDR:3)) (ASPECT_IDENT:DEFAULT_ASPECT) (CARRIER:enroll) (SIGNAL_UUID)) (BODY:{(YUP)}) (RUN_TREE (NEW_XADDR:MEMBER.1) (PARAMS))))))");


}

void testGroup() {
    G_vm = _v_new();
    testGroupCreate();
    _v_free(G_vm);
}
