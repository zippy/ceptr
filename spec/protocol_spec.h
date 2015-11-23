/**
 * @file protocol_spec.h
 * @copyright Copyright (C) 2013-2015, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 * @ingroup tests
 */

#include "../src/protocol.h"

/*
  Protocol Startpoint <- Data Source (of type carrier type)
     The carrier type is implicit in the first step's expectation semtrex.
  Protocol Endpoint -> Action Handler (which can accept data of my carrier type)
     The last step of a protocol is an "output" handler ACTION that must be provided at
     expression time
*/
Receptor *_makeAliveProtocolReceptor() {
    Receptor *r;
    r = _r_new(TEST_RECEPTOR_SYMBOL);

    Symbol alive = _r_declare_symbol(r,PROTOCOL_DEF,"alive");
    Symbol server = _r_declare_symbol(r,RECEPTOR_ADDRESS,"server");
    Symbol ping = _r_declare_symbol(r,NULL_STRUCTURE,"ping");
    Symbol yup = _r_declare_symbol(r,NULL_STRUCTURE,"yup");

    T *pattern = _t_new_root(PATTERN);
    _sl(pattern,ping);
    T *ping_resp = _t_new_root(RESPOND);
    _t_news(ping_resp,RESPONSE_CARRIER,alive);
    _t_newr(ping_resp,yup);
    Process proc = _r_code_process(r,ping_resp,"respond with yup","long desc...",NULL);
    T *action = _t_newp(0,ACTION,proc);

    T *e = __r_build_expectation(alive,pattern,action,0,0);

    pattern = _t_new_root(PATTERN);
    _sl(pattern,yup);
    action = _t_newp(0,ACTION,NULL_PROCESS);
    T *e2 = __r_build_expectation(alive,pattern,action,0,0);

    /*
    alive {
        roles {
            server;
        }
        conversations {
            alive {
                server expects ping from ! {
                    respond with yup
                        }
                ! expects yup from server {
                    OUTPUT
                        }
            }
        }
    }
    */

    T *p = _o_new(r,alive,
                  ROLE,server,
                  CONVERSATION,"alive",
                      server,ANYBODY,e,
                      ANYBODY,server,e2,
                  NULL_SYMBOL);


    return r;
}

void testProtocolAlive() {
    Receptor *r = _makeAliveProtocolReceptor();
    Symbol alive = _r_get_symbol_by_label(r,"alive");
    Symbol ping =_r_get_symbol_by_label(r,"ping");
    _o_express_role(r,alive,_r_get_symbol_by_label(r,"server"),DEFAULT_ASPECT,NULL);

    // check the protocol def
    spec_is_str_equal(_td(r,r->defs.protocols),"(PROTOCOLS (alive (ROLE:server) (CONVERSATION (CONVERSATION_LABEL:alive) (ROLE_PROCESS (ROLE:server) (SOURCE (ANYBODY)) (EXPECTATION (CARRIER:alive) (PATTERN (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:ping))) (ACTION:respond with yup) (PARAMS) (END_CONDITIONS (UNLIMITED)))) (ROLE_PROCESS (ROLE:ANYBODY) (SOURCE (ROLE:server)) (EXPECTATION (CARRIER:alive) (PATTERN (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:yup))) (ACTION:NULL_PROCESS) (PARAMS) (END_CONDITIONS (UNLIMITED)))))))");

    // check that it was expressed
    spec_is_str_equal(_td(r,r->flux),"(FLUX (DEFAULT_ASPECT (EXPECTATIONS (EXPECTATION (CARRIER:alive) (PATTERN (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:ping))) (ACTION:respond with yup) (PARAMS) (END_CONDITIONS (UNLIMITED)))) (SIGNALS)))");

    Receptor *r2 = _makeAliveProtocolReceptor();
    T *noop = _t_new_root(NOOP);
    _t_newi(noop,TEST_INT_SYMBOL,314);
    Process proc = _r_code_process(r2,noop,"do nothing","long desc...",NULL);
    T *handler = _t_newp(0,ACTION,proc);
    _o_express_role(r2,alive,ANYBODY,DEFAULT_ASPECT,handler);
    _t_free(handler);

    // check that the expressed expectation is bound correctly to the handler
    spec_is_str_equal(_td(r2,r2->flux),"(FLUX (DEFAULT_ASPECT (EXPECTATIONS (EXPECTATION (CARRIER:alive) (PATTERN (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:yup))) (ACTION:do nothing) (PARAMS) (END_CONDITIONS (UNLIMITED)))) (SIGNALS)))");

    T *s = __r_make_signal(0,0,DEFAULT_ASPECT,alive,_t_new_root(ping),0,0);
    //debug_enable(D_SIGNALS);
    spec_is_equal(_r_deliver(r,s),noDeliveryErr);
    spec_is_str_equal(_td(r,r->q->active->context->run_tree),"(RUN_TREE (process:RESPOND (RESPONSE_CARRIER:alive) (yup)) (PARAMS))");
    debug_disable(D_SIGNALS);
    spec_is_equal(_p_reduceq(r->q),noReductionErr);
    spec_is_str_equal(_td(r,r->q->completed->context->run_tree),"(RUN_TREE (SIGNAL_UUID) (PARAMS))");
    spec_is_str_equal(_td(r,r->pending_signals),"(PENDING_SIGNALS (SIGNAL (ENVELOPE (FROM_ADDRESS (INSTANCE_NUM:0)) (TO_ADDRESS (INSTANCE_NUM:0)) (ASPECT_IDENT:DEFAULT_ASPECT) (CARRIER:alive) (SIGNAL_UUID) (IN_RESPONSE_TO_UUID)) (BODY:{(yup)})))");

    _r_free(r);
}

void testProtocol() {
    G_vm = _v_new();
    testProtocolAlive();
    _v_free(G_vm);
}
