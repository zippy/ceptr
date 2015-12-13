/**
 * @file protocol_spec.h
 * @copyright Copyright (C) 2013-2015, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 * @ingroup tests
 */

#include "../src/protocol.h"

void testProtocolRequesting() {

    spec_is_str_equal(t2s(_sem_get_def(G_sem,REQUESTING)),"(PROTOCOL_DEFINITION (PROTOCOL_LABEL:REQUESTING) (PROTOCOL_SEMANTICS (ROLE:REQUESTER) (ROLE:RESPONDER) (GOAL:REQUEST_HANDLER) (GOAL:RESPONSE_HANDLER) (USAGE:REQUEST_DATA) (USAGE:RESPONSE_DATA)) (backnforth (INITIATE (ROLE:REQUESTER) (DESTINATION (ROLE:RESPONDER)) (ACTION:send_request)) (EXPECT (ROLE:RESPONDER) (SOURCE (ROLE:REQUESTER)) (PATTERN (SEMTREX_SYMBOL_LITERAL (SLOT (USAGE:REQUEST_DATA) (SLOT_IS_VALUE_OF:SEMTREX_SYMBOL)))) (ACTION:send_response))))");

    spec_is_str_equal(t2s(_sem_get_def(G_sem,send_request)),"(PROCESS_DEFINITION (PROCESS_NAME:send_request) (PROCESS_INTENTION:send request) (process:REQUEST (SLOT (ROLE:RESPONDER)) (SLOT (USAGE:REQUEST_DATA)) (SLOT (USAGE:REQUEST_DATA)) (SLOT (USAGE:RESPONSE_DATA)) (SLOT (GOAL:REQUEST_HANDLER))) (PROCESS_SIGNATURE (OUTPUT_SIGNATURE (SIGNATURE_LABEL:response) (SIGNATURE_ANY))))");

    spec_is_str_equal(t2s(_sem_get_def(G_sem,send_response)),"(PROCESS_DEFINITION (PROCESS_NAME:send_response) (PROCESS_INTENTION:send response) (process:RESPOND (SIGNAL_REF:/1/4) (GOAL:RESPONSE_HANDLER)) (PROCESS_SIGNATURE (OUTPUT_SIGNATURE (SIGNATURE_LABEL:response id) (SIGNATURE_SYMBOL:SIGNAL_UUID))))");

    //@todo trying to express request protocol should fail because it's not concrete enough

}

void testProtocolRecognize() {

    T *recog = _sem_get_def(G_sem,RECOGNIZE);

    spec_is_str_equal(t2s(recog),"(PROTOCOL_DEFINITION (PROTOCOL_LABEL:RECOGNIZE) (PROTOCOL_SEMANTICS) (INCLUSION (PNAME:REQUESTING) (CONNECTION (WHICH_ROLE (ROLE:REQUESTER) (ROLE:RECOGNIZER))) (CONNECTION (WHICH_ROLE (ROLE:RESPONDER) (ROLE:RECOGNIZEE))) (CONNECTION (WHICH_GOAL (GOAL:REQUEST_HANDLER) (GOAL:RECOGNITION))) (RESOLUTION (WHICH_SYMBOL (USAGE:REQUEST_DATA) (ACTUAL_SYMBOL:are_you))) (RESOLUTION (WHICH_SYMBOL (USAGE:RESPONSE_DATA) (ACTUAL_SYMBOL:i_am))) (RESOLUTION (WHICH_PROCESS (GOAL:RESPONSE_HANDLER) (ACTUAL_PROCESS:fill_i_am)))))");

    T *t = _o_unwrap(G_sem,recog);
    spec_is_str_equal(t2s(t),"(PROTOCOL_DEFINITION (PROTOCOL_LABEL:RECOGNIZE) (PROTOCOL_SEMANTICS (ROLE:RECOGNIZER) (GOAL:RECOGNITION)) (backnforth (INITIATE (ROLE:RECOGNIZER) (DESTINATION (ROLE:RECOGNIZEE)) (ACTION:send_request)) (EXPECT (ROLE:RECOGNIZEE) (SOURCE (ROLE:RECOGNIZER)) (PATTERN (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:are_you))) (ACTION:send_response plus something about the template...))))");

    Receptor *self =  _r_new(G_sem,TEST_RECEPTOR);

    T *noop = _t_new_root(NOOP);
    _t_newi(noop,TEST_INT_SYMBOL,314);
    Process proc = _r_define_process(self,noop,"do nothing","long desc...",NULL);

    T *bindings = _t_new_root(PROTOCOL_BINDINGS);
    T *res = _t_newr(bindings,RESOLUTION);
    T *w = _t_newr(res,WHICH_PROCESS);
    _t_news(w,GOAL,RECOGNITION);
    _t_news(w,ACTUAL_PROCESS,proc);

    _o_express_role(self,RECOGNIZE,RECOGNIZER,DEFAULT_ASPECT,bindings);
    _t_free(bindings);

    //recognizer doesn't add expectations
    spec_is_str_equal(_td(self,self->flux),"(FLUX (DEFAULT_ASPECT (EXPECTATIONS) (SIGNALS)))");

    // recognizee doesn't need any bindings because that role's completed by unwrapping
    _o_express_role(self,RECOGNIZE,RECOGNIZEE,DEFAULT_ASPECT,NULL);
    // however it does add expectation
    spec_is_str_equal(_td(self,self->flux),"(FLUX (DEFAULT_ASPECT (EXPECTATIONS (EXPECTATION (CARRIER:RECOGNIZE) (PATTERN (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:are_you))) (ACTION:send_response) (PARAMS) (END_CONDITIONS (UNLIMITED)))) (SIGNALS)))");

    VMHost *v = G_vm;
    Xaddr x = _v_new_receptor(v,v->r,TEST_RECEPTOR,self);
    _v_activate(v,x);
    T *s = __r_make_signal(self->addr,self->addr,DEFAULT_ASPECT,RECOGNIZE,_t_newr(0,are_you),0,0);
    //    debug_enable(D_SIGNALS);
    _r_send(self,s);
    spec_is_str_equal(t2s(self->root),"(RECEPTOR_INSTANCE (CONTEXT_NUM:3) (PARENT_CONTEXT_NUM:0) (RECEPTOR_STATE (FLUX (DEFAULT_ASPECT (EXPECTATIONS (EXPECTATION (CARRIER:RECOGNIZE) (PATTERN (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:are_you))) (ACTION:send_response) (PARAMS) (END_CONDITIONS (UNLIMITED)))) (SIGNALS))) (PENDING_SIGNALS (SIGNAL (ENVELOPE (FROM_ADDRESS (RECEPTOR_ADDR:3)) (TO_ADDRESS (RECEPTOR_ADDR:3)) (ASPECT_IDENT:DEFAULT_ASPECT) (CARRIER:RECOGNIZE) (SIGNAL_UUID)) (BODY:{(are_you)}))) (PENDING_RESPONSES) (RECEPTOR_ELAPSED_TIME:0)))");
    /* _v_deliver_signals(v,self); */
    /* spec_is_str_equal(t2s(self->root),"(RECEPTOR_INSTANCE (CONTEXT_NUM:3) (PARENT_CONTEXT_NUM:0) (RECEPTOR_STATE (FLUX (DEFAULT_ASPECT (EXPECTATIONS (EXPECTATION (CARRIER:RECOGNIZE) (PATTERN (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:are_you))) (ACTION:send_response) (PARAMS) (END_CONDITIONS (UNLIMITED)))) (SIGNALS (SIGNAL (ENVELOPE (FROM_ADDRESS (RECEPTOR_ADDR:3)) (TO_ADDRESS (RECEPTOR_ADDR:3)) (ASPECT_IDENT:DEFAULT_ASPECT) (CARRIER:RECOGNIZE) (SIGNAL_UUID)) (BODY:{(are_you)}) (RUN_TREE (process:RESPOND (SIGNAL_REF:/1/4) (GOAL:RESPONSE_HANDLER)) (PARAMS)))))) (PENDING_SIGNALS) (PENDING_RESPONSES) (RECEPTOR_ELAPSED_TIME:0)))"); */
    /* _p_reduceq(self->q); */
    /* spec_is_str_equal(t2s(self->root),""); */
    /* _v_deliver_signals(v,self); */
    /* debug_disable(D_SIGNALS); */

}

void testProtocolAlive() {
    SemTable *sem = G_vm->r->sem;

    Receptor *r =  _r_new(sem,TEST_RECEPTOR);

    _o_express_role(r,ALIVE,SERVER,DEFAULT_ASPECT,NULL);

    // check that it was expressed
    spec_is_str_equal(_td(r,r->flux),"(FLUX (DEFAULT_ASPECT (EXPECTATIONS (EXPECTATION (CARRIER:ALIVE) (PATTERN (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:PING))) (ACTION:respond_with_yup) (PARAMS) (END_CONDITIONS (UNLIMITED)))) (SIGNALS)))");

    Receptor *r2 =  _r_new(sem,TEST_RECEPTOR);
    T *noop = _t_new_root(NOOP);
    _t_newi(noop,TEST_INT_SYMBOL,314);
    Process proc = _r_define_process(r2,noop,"do nothing","long desc...",NULL);

    T *bindings = _t_build(G_sem,0,PROTOCOL_BINDINGS,RESOLUTION,WHICH_PROCESS,GOAL,HANDLER,ACTUAL_PROCESS,proc,NULL_SYMBOL,NULL_SYMBOL,NULL_SYMBOL);

    _o_express_role(r2,ALIVE,CLIENT,DEFAULT_ASPECT,bindings);
    _t_free(bindings);

    // check that the expressed expectation is bound correctly to the handler
    spec_is_str_equal(_td(r2,r2->flux),"(FLUX (DEFAULT_ASPECT (EXPECTATIONS (EXPECTATION (CARRIER:ALIVE) (PATTERN (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:YUP))) (ACTION:do nothing) (PARAMS) (END_CONDITIONS (UNLIMITED)))) (SIGNALS)))");

    T *s = __r_make_signal(r->addr,r->addr,DEFAULT_ASPECT,ALIVE,_t_new_root(PING),0,0);
    //debug_enable(D_SIGNALS);
    spec_is_equal(_r_deliver(r,s),noDeliveryErr);
    spec_is_str_equal(_td(r,r->q->active->context->run_tree),"(RUN_TREE (process:RESPOND (SIGNAL_REF:/1/4) (YUP)) (PARAMS))");  // responds on the carrier in the signal envelope
    debug_disable(D_SIGNALS);
    spec_is_equal(_p_reduceq(r->q),noReductionErr);
    spec_is_str_equal(_td(r,r->q->completed->context->run_tree),"(RUN_TREE (SIGNAL_UUID) (PARAMS))");
    spec_is_str_equal(_td(r,r->pending_signals),"(PENDING_SIGNALS (SIGNAL (ENVELOPE (FROM_ADDRESS (RECEPTOR_ADDR:3)) (TO_ADDRESS (RECEPTOR_ADDR:3)) (ASPECT_IDENT:DEFAULT_ASPECT) (CARRIER:ALIVE) (SIGNAL_UUID) (IN_RESPONSE_TO_UUID)) (BODY:{(YUP)})))");

    _r_free(r);
    _r_free(r2);
}

Protocol simple;
Symbol agent;
Symbol process;
Symbol data;
T *simple_def;

Protocol shake;
Symbol mover;
Symbol shaker;
Symbol shaking;
Symbol info;
T *shake_def;

void _setupTestProtocols() {

    // a simple abstract protocol with an agent a process and some data
    agent = _d_define_symbol(G_sem,RECEPTOR_ADDRESS,"agent",TEST_CONTEXT);
    process = _d_define_symbol(G_sem,PROCESS,"process",TEST_CONTEXT);
    data = _d_define_symbol(G_sem,SYMBOL,"data",TEST_CONTEXT);
    T *action =  _t_build(G_sem,0,SLOT,GOAL,process,SLOT_IS_VALUE_OF,ACTION,NULL_SYMBOL);
    T *pattern = _t_build(G_sem,0,PATTERN,SEMTREX_SYMBOL_LITERAL,SLOT,USAGE,data,SLOT_IS_VALUE_OF,SEMTREX_SYMBOL,NULL_SYMBOL,NULL_SYMBOL);

    simple_def = _o_make_protocol_def(G_sem,TEST_CONTEXT,"do",
                                      ROLE,agent,
                                      GOAL,process,
                                      USAGE,data,
                                      INTERACTION,"act",
                                        EXPECT,agent,agent,pattern,action,
                                      NULL_SYMBOL);
    simple = _d_define_protocol(G_sem,simple_def,TEST_CONTEXT);

    // a protocol that's a wrapping of the simple protocol
    mover = _d_define_symbol(G_sem,RECEPTOR_ADDRESS,"mover",TEST_CONTEXT);
    shaker = _d_define_symbol(G_sem,RECEPTOR_ADDRESS,"shaker",TEST_CONTEXT);
    shaking = _d_define_symbol(G_sem,PROCESS,"shaking",TEST_CONTEXT);
    info = _d_define_symbol(G_sem,SYMBOL,"info",TEST_CONTEXT);

    shake_def = _o_make_protocol_def(G_sem,TEST_CONTEXT,"shake",
                                     ROLE,mover,
                                     ROLE,shaker,
                                     INCLUSION,simple,
                                       WHICH_ROLE,agent,mover,
                                       WHICH_GOAL,process,shaking,
                                       WHICH_USAGE,data,info,
                                      NULL_SYMBOL);
    shake = _d_define_protocol(G_sem,shake_def,TEST_CONTEXT);
}

void testProtocolResolve() {

    T *noop = _t_new_root(NOOP);
    _t_newi(noop,TEST_INT_SYMBOL,314);
    Process proc = _d_define_process(G_sem,noop,"do nothing","long desc...",NULL,TEST_CONTEXT);

    T *bindings = _t_build(G_sem,0,PROTOCOL_BINDINGS,RESOLUTION,WHICH_PROCESS,GOAL,process,ACTUAL_PROCESS,proc,NULL_SYMBOL,RESOLUTION,WHICH_SYMBOL,USAGE,data,ACTUAL_SYMBOL,TEST_INT_SYMBOL,NULL_SYMBOL,NULL_SYMBOL);

    spec_is_str_equal(t2s(bindings),"(PROTOCOL_BINDINGS (RESOLUTION (WHICH_PROCESS (GOAL:process) (ACTUAL_PROCESS:do nothing))) (RESOLUTION (WHICH_SYMBOL (USAGE:data) (ACTUAL_SYMBOL:TEST_INT_SYMBOL))))");
    spec_is_str_equal(t2s(simple_def),"(PROTOCOL_DEFINITION (PROTOCOL_LABEL:do) (PROTOCOL_SEMANTICS (ROLE:agent) (GOAL:process) (USAGE:data)) (act (EXPECT (ROLE:agent) (SOURCE (ROLE:agent)) (PATTERN (SEMTREX_SYMBOL_LITERAL (SLOT (USAGE:data) (SLOT_IS_VALUE_OF:SEMTREX_SYMBOL)))) (SLOT (GOAL:process) (SLOT_IS_VALUE_OF:ACTION)))))");
    T *resolved = _t_clone(simple_def);
    _o_resolve(G_sem,resolved,bindings);
    spec_is_str_equal(t2s(resolved),"(PROTOCOL_DEFINITION (PROTOCOL_LABEL:do) (PROTOCOL_SEMANTICS (ROLE:agent) (GOAL:process) (USAGE:data)) (act (EXPECT (ROLE:agent) (SOURCE (ROLE:agent)) (PATTERN (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:TEST_INT_SYMBOL))) (ACTION:do nothing))))");

    _t_free(bindings);
    _t_free(resolved);
}

void testProtocolUnwrap() {
    T *unwrapped = _o_unwrap(G_sem,shake_def);
    spec_is_str_equal(t2s(unwrapped),"(PROTOCOL_DEFINITION (PROTOCOL_LABEL:shake) (PROTOCOL_SEMANTICS (ROLE:mover) (ROLE:shaker) (GOAL:shaking) (USAGE:info)) (act (EXPECT (ROLE:mover) (SOURCE (ROLE:mover)) (PATTERN (SEMTREX_SYMBOL_LITERAL (SLOT (USAGE:info) (SLOT_IS_VALUE_OF:SEMTREX_SYMBOL)))) (SLOT (GOAL:shaking) (SLOT_IS_VALUE_OF:ACTION)))))");
    _t_free(unwrapped);
}



/* Symbol wrapper [(token,conversationbody)]; */
/* Symbol open_request,close_notice ; */
/* T *def = _o_make_protocol_def */
/*     (sem,"connect",   // establish a session connection */
/*      ROLE,connector,  // TCP socket client stack */
/*      ROLE,connectee,  // TCP socket server stack */
/*      ROLE,source,     // mail client */
/*      ROLE,destination,// mail server */
/*      USAGE,session_token, // new higher port given in open negotiation */
/*      CARRYING,conversation, // SMTP */

/*      INTERACTION,"open",  // receive on port 25 negotiate new port (as token) */
/*         INITIATE,connector,connectee, send open_request,bind response token to source. */
/*         EXPECT,connectee,connector,pattern=open_request, generate token/connector, bind pair with destination,  return token, ENABLE "converse","close" */
/*      INTERACTION,"converse", */
/*         INITIATE,connector,connectee send conversation inside wrapper */
/*         EXPECT,connectee,connector,pattern=wrapper,unwrap and map source & */
/*      INTERACTION,"close", */
/*         EXPECT,connector,connectee,pattern=close_notice, tell source about closing, */
/*         EXPECT,connectee,connector,pattern=close_notice, tell destination about closing, cleanup token. */
/*      NULL_SYMBOL); */



void testProtocol() {
    G_vm = _v_new();
    _setupTestProtocols();
    testProtocolResolve();
    testProtocolUnwrap();
    testProtocolRequesting();
    testProtocolRecognize();
    testProtocolAlive();
    _v_free(G_vm);
}
