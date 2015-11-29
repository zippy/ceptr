/**
 * @file protocol_spec.h
 * @copyright Copyright (C) 2013-2015, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 * @ingroup tests
 */

#include "../src/protocol.h"

/* Receptor *_makeRequestingProtocolReceptor() { */
/*     Receptor *r; */
/*     r = _r_new(G_sem,TEST_RECEPTOR_SYMBOL); */

/*     Symbol requesting = _r_declare_symbol(r,PROTOCOL_DEF,"requesting"); */
/*     Symbol requester = _r_declare_symbol(r,RECEPTOR_ADDRESS,"requester"); */
/*     Symbol responder = _r_declare_symbol(r,RECEPTOR_ADDRESS,"responder"); */
/*     Symbol request = _r_declare_symbol(r,SYMBOL,"request"); */
/*     Symbol response = _r_declare_symbol(r,SYMBOL,"response"); */
/*     Symbol handler = _r_declare_symbol(r,PROCESS,"handler"); */

/*     T *pattern = _t_new_root(PATTERN); */
/*     _sl(pattern,request); */
/*     T *resp = _t_new_root(RESPOND); */
/*     // use the carrier it came in on as the response carrier. */
/*     _t_news(resp,RESPONSE_CARRIER,requesting); */
/*     _t_newr(resp,response); */
/*     Process proc = _r_code_process(r,resp,"respond","long desc...",NULL); */
/*     T *action = _t_newp(0,ACTION,proc); */

/*     T *e = __r_build_expectation(requesting,pattern,action,0,0); */

/*     pattern = _t_new_root(PATTERN); */
/*     _sl(pattern,response); */
/*     action = _t_news(0,GOAL,handler); */
/*     T *e2 = __r_build_expectation(alive,pattern,action,0,0); */

/*     /\* */
/*     requesting { */
/*         roles { */
/*             requester; */
/*             responder; */
/*         } */
/*         conversations { */
/*             requesting { */
/*                 responder expects request from requester { */
/*                     respond with response */
/*                         } */
/*                 requester expects response from responder { */
/*                     handler */
/*                         } */
/*             } */
/*         } */
/*     } */
/*     *\/ */

/*     T *p = _o_new(r,requesting, */
/*                   ROLE,requester, */
/*                   ROLE,responder, */
/*                   GOAL,handler, */
/*                   USAGE,request, */
/*                   USAGE,response, */
/*                   CONVERSATION,"exchange", */
/*                     requester,responder,e, */
/*                     responder,requester,e2, */
/*                   NULL_SYMBOL); */
/*     return r; */
/* } */

/* Receptor *_makeAlive2ProtocolReceptor() { */

/*     Receptor *rqp = _makeRequestingProtocolReceptor(); */
/*     Receptor *r; */
/*     r = _r_new(G_sem,TEST_RECEPTOR_SYMBOL); */
/*     Symbol alive = _r_declare_symbol(r,PROTOCOL_DEF,"alive"); */

/*     Symbol request = _r_get_sem_by_label(rqp,"request"); */

/*     T *p = _o_new(r,alive, */
/*                   COMPOSITION,requesting,    //does this change carrier in requesting to alive? */
/*                     request,ping, */
/*                     response,yup, */
/*                   NULL_SYMBOL); */

/*     T *p = _o_new(r,requesting, */
/*                   ROLE,requester, */
/*                   ROLE,responder, */
/*                   GOAL,handler, */
/*                   USAGE,request, */
/*                   USAGE,response, */
/*                   CONVERSATION,"exchange", */
/*                   requester,responder,e, */
/*                   responder,requester,e2, */
/*                   NULL_SYMBOL); */
/* } */

Receptor *_makeAliveProtocolReceptor(VMHost *v) {
    Receptor *r;
    SemTable *sem = v->r->sem;
    r = _r_new(sem,TEST_RECEPTOR_SYMBOL);

    Symbol server = _r_declare_symbol(r,RECEPTOR_ADDRESS,"server");
    Symbol client = _r_declare_symbol(r,RECEPTOR_ADDRESS,"client");
    Symbol ping = _r_declare_symbol(r,NULL_STRUCTURE,"ping");
    Symbol yup = _r_declare_symbol(r,NULL_STRUCTURE,"yup");
    Symbol handler = _r_declare_symbol(r,PROCESS_FORM,"handler");
    T *pattern1 = _t_new_root(PATTERN);
    _sl(pattern1,ping);
    T *ping_resp = _t_new_root(RESPOND);
    //    _t_news(ping_resp,RESPONSE_CARRIER,alive);
    _t_newr(ping_resp,yup);
    Process proc = _r_code_process(r,ping_resp,"respond with yup","long desc...",NULL);
    T *action1 = _t_newp(0,ACTION,proc);

    T *pattern2 = _t_new_root(PATTERN);
    _sl(pattern2,yup);
    T *action2 = _t_news(0,GOAL,handler);


    /*
    alive {
        roles {
            server;
            client;
        }
        conversations {
            alive {
                server expects ping from client {
                    respond with yup
                        }
                client expects yup from server {
                    OUTPUT
                        }
            }
        }
    }
    */

    T *def = _o_make_protocol_def("alive",
                  ROLE,server,
                  ROLE,client,
                  GOAL,handler,
                  CONVERSATION,"alive",
                      server,client,pattern1,action1,
                      client,server,pattern2,action2,
                  NULL_SYMBOL);
    _r_define_protocol(r,def);

    return r;
}

void testProtocolAlive() {
    Receptor *alive_r = _makeAliveProtocolReceptor(G_vm);
    Protocol alive = _r_get_sem_by_label(alive_r,"alive");
    Symbol ping =_r_get_sem_by_label(alive_r,"ping");
    Symbol server = _r_get_sem_by_label(alive_r,"server");
    Symbol client = _r_get_sem_by_label(alive_r,"client");

    // check the protocol def
    spec_is_str_equal(_td(alive_r,alive_r->defs.protocols),"(PROTOCOLS (PROTOCOL_DEFINITION (PROTOCOL_LABEL:alive) (ROLE:server) (ROLE:client) (GOAL:handler) (CONVERSATION (CONVERSATION_LABEL:alive) (ROLE_PROCESS (ROLE:server) (SOURCE (ROLE:client)) (PATTERN (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:ping))) (ACTION:respond with yup)) (ROLE_PROCESS (ROLE:client) (SOURCE (ROLE:server)) (PATTERN (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:yup))) (GOAL:handler)))))");

    Receptor *r =  _r_new(alive_r->sem,TEST_RECEPTOR_SYMBOL);

    _o_express_role(r,alive,server,DEFAULT_ASPECT,NULL);

    // check that it was expressed
    spec_is_str_equal(_td(r,r->flux),"(FLUX (DEFAULT_ASPECT (EXPECTATIONS (EXPECTATION (CARRIER:alive) (PATTERN (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:ping))) (ACTION:respond with yup) (PARAMS) (END_CONDITIONS (UNLIMITED)))) (SIGNALS)))");
    return;

    Receptor *r2 =  _r_new(alive_r->sem,TEST_RECEPTOR_SYMBOL);
    T *noop = _t_new_root(NOOP);
    _t_newi(noop,TEST_INT_SYMBOL,314);
    Process proc = _r_code_process(r2,noop,"do nothing","long desc...",NULL);
    T *handler = _t_newp(0,ACTION,proc);
    _o_express_role(r2,alive,client,DEFAULT_ASPECT,handler);
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
    spec_is_str_equal(_td(r,r->pending_signals),"(PENDING_SIGNALS (SIGNAL (ENVELOPE (FROM_ADDRESS (CONTEXT_NUM:0)) (TO_ADDRESS (CONTEXT_NUM:0)) (ASPECT_IDENT:DEFAULT_ASPECT) (CARRIER:alive) (SIGNAL_UUID) (IN_RESPONSE_TO_UUID)) (BODY:{(yup)})))");

    _r_free(r);
    _r_free(r2);
}

void testProtocol() {
    G_vm = _v_new();
    testProtocolAlive();
    _v_free(G_vm);
}
