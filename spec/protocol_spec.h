/**
 * @file protocol_spec.h
 * @copyright Copyright (C) 2013-2015, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 * @ingroup tests
 */

#include "../src/protocol.h"
void testProtocolRequesting() {
    spec_is_str_equal(t2s(_sem_get_def(G_sem,REQUESTING)),"(PROTOCOL_DEFINITION (PROTOCOL_LABEL:REQUESTING) (PROTOCOL_SEMANTICS (ROLE:REQUESTER) (ROLE:RESPONDER) (GOAL:REQUEST_HANDLER) (GOAL:RESPONSE_HANDLER) (USAGE:REQUEST_DATA) (USAGE:RESPONSE_DATA)) (backnforth (INITIATE (ROLE:REQUESTER) (DESTINATION (ROLE:RESPONDER)) (ACTION:send_request)) (EXPECT (ROLE:RESPONDER) (SOURCE (ROLE:REQUESTER)) (PATTERN (SEMTREX_SYMBOL_LITERAL (SLOT (USAGE:REQUEST_DATA) (SLOT_IS_VALUE_OF:SEMTREX_SYMBOL)))) (ACTION:send_response))))");
    spec_is_str_equal(t2s(_sem_get_def(G_sem,send_request)),"(PROCESS_DEFINITION (PROCESS_NAME:send_request) (PROCESS_INTENTION:send request) (SLOT (GOAL:REQUEST_HANDLER) (SLOT_CHILDREN (process:REQUEST (SLOT (ROLE:RESPONDER) (SLOT_IS_VALUE_OF:TO_ADDRESS)) (ASPECT_IDENT:DEFAULT_ASPECT) (CARRIER:backnforth) (SLOT (USAGE:REQUEST_DATA)) (CARRIER:backnforth)))) (PROCESS_SIGNATURE (OUTPUT_SIGNATURE (SIGNATURE_LABEL:response) (SIGNATURE_ANY)) (TEMPLATE_SIGNATURE (EXPECTED_SLOT (ROLE:RESPONDER) (SLOT_IS_VALUE_OF:TO_ADDRESS)) (EXPECTED_SLOT (USAGE:REQUEST_DATA)) (EXPECTED_SLOT (GOAL:REQUEST_HANDLER)))))");

    spec_is_str_equal(t2s(_sem_get_def(G_sem,send_response)),"(PROCESS_DEFINITION (PROCESS_NAME:send_response) (PROCESS_INTENTION:send response) (process:RESPOND (SIGNAL_REF:/1/4) (SLOT (GOAL:RESPONSE_HANDLER))) (PROCESS_SIGNATURE (OUTPUT_SIGNATURE (SIGNATURE_LABEL:response id) (SIGNATURE_SYMBOL:SIGNAL_UUID)) (TEMPLATE_SIGNATURE (EXPECTED_SLOT (GOAL:RESPONSE_HANDLER)))))");

    //@todo trying to express request protocol should fail because it's not concrete enough

}

void testProtocolRecognize() {

    T *recog = _sem_get_def(G_sem,RECOGNIZE);

    // check the recognize protocol definition
    spec_is_str_equal(t2s(recog),"(PROTOCOL_DEFINITION (PROTOCOL_LABEL:RECOGNIZE) (PROTOCOL_SEMANTICS) (INCLUSION (PNAME:REQUESTING) (CONNECTION (WHICH_ROLE (ROLE:REQUESTER) (ROLE:RECOGNIZER))) (CONNECTION (WHICH_ROLE (ROLE:RESPONDER) (ROLE:RECOGNIZEE))) (CONNECTION (WHICH_GOAL (GOAL:REQUEST_HANDLER) (GOAL:RECOGNITION))) (RESOLUTION (WHICH_SYMBOL (USAGE:REQUEST_DATA) (ACTUAL_SYMBOL:are_you))) (RESOLUTION (WHICH_SYMBOL (USAGE:RESPONSE_DATA) (ACTUAL_SYMBOL:i_am))) (RESOLUTION (WHICH_PROCESS (GOAL:RESPONSE_HANDLER) (ACTUAL_PROCESS:fill_i_am)))))");

    T *sem_map = _t_new_root(SEMANTIC_MAP);
    T *t = _o_unwrap(G_sem,recog,sem_map);
    // and also check how it gets unwrapped because it's defined in terms of REQUESTING
    spec_is_str_equal(t2s(t),"(PROTOCOL_DEFINITION (PROTOCOL_LABEL:RECOGNIZE) (PROTOCOL_SEMANTICS (ROLE:RECOGNIZER) (GOAL:RECOGNITION)) (backnforth (INITIATE (ROLE:RECOGNIZER) (DESTINATION (ROLE:RECOGNIZEE)) (ACTION:send_request)) (EXPECT (ROLE:RECOGNIZEE) (SOURCE (ROLE:RECOGNIZER)) (PATTERN (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:are_you))) (ACTION:send_response))))");

    // the unwrapping should build up a semantic map
    spec_is_str_equal(t2s(sem_map),"(SEMANTIC_MAP (SEMANTIC_LINK (ROLE:REQUESTER) (REPLACEMENT_VALUE (ROLE:RECOGNIZER))) (SEMANTIC_LINK (ROLE:RESPONDER) (REPLACEMENT_VALUE (ROLE:RECOGNIZEE))) (SEMANTIC_LINK (GOAL:REQUEST_HANDLER) (REPLACEMENT_VALUE (GOAL:RECOGNITION))) (SEMANTIC_LINK (USAGE:REQUEST_DATA) (REPLACEMENT_VALUE (ACTUAL_SYMBOL:are_you))) (SEMANTIC_LINK (USAGE:RESPONSE_DATA) (REPLACEMENT_VALUE (ACTUAL_SYMBOL:i_am))) (SEMANTIC_LINK (GOAL:RESPONSE_HANDLER) (REPLACEMENT_VALUE (ACTUAL_PROCESS:fill_i_am))))");
    _t_free(sem_map);

    // create a receptor to express the proctocl
    Receptor *self =  _r_new(G_sem,TEST_RECEPTOR);

    // and a noop-action to run as the result of recognizing
    T *noop = _t_new_root(NOOP);
    _t_newi(noop,TEST_INT_SYMBOL,314);
    Process proc = _r_define_process(self,noop,"do nothing","long desc...",NULL);

    // which we add to the bindings
    T *bindings = _t_new_root(PROTOCOL_BINDINGS);
    T *res = _t_newr(bindings,RESOLUTION);
    T *w = _t_newr(res,WHICH_PROCESS);
    _t_news(w,GOAL,RECOGNITION);
    _t_news(w,ACTUAL_PROCESS,proc);

    // that get used to express the RECOGNIZER half of the protocol
    _o_express_role(self,RECOGNIZE,RECOGNIZER,DEFAULT_ASPECT,bindings);
    _t_free(bindings);

    //recognizer doesn't add expectations
    spec_is_str_equal(_td(self,self->flux),"(FLUX (DEFAULT_ASPECT (EXPECTATIONS) (SIGNALS)))");

    // recognizee doesn't need any bindings because that role's completed by unwrapping
    _o_express_role(self,RECOGNIZE,RECOGNIZEE,DEFAULT_ASPECT,NULL);
    // however it does add expectation
    spec_is_str_equal(_td(self,self->flux),"(FLUX (DEFAULT_ASPECT (EXPECTATIONS (EXPECTATION (CARRIER:backnforth) (PATTERN (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:are_you))) (ACTION:send_response) (PARAMS) (END_CONDITIONS (UNLIMITED)) (SEMANTIC_MAP (SEMANTIC_LINK (ROLE:REQUESTER) (REPLACEMENT_VALUE (ROLE:RECOGNIZER))) (SEMANTIC_LINK (ROLE:RESPONDER) (REPLACEMENT_VALUE (ROLE:RECOGNIZEE))) (SEMANTIC_LINK (GOAL:REQUEST_HANDLER) (REPLACEMENT_VALUE (GOAL:RECOGNITION))) (SEMANTIC_LINK (USAGE:REQUEST_DATA) (REPLACEMENT_VALUE (ACTUAL_SYMBOL:are_you))) (SEMANTIC_LINK (USAGE:RESPONSE_DATA) (REPLACEMENT_VALUE (ACTUAL_SYMBOL:i_am))) (SEMANTIC_LINK (GOAL:RESPONSE_HANDLER) (REPLACEMENT_VALUE (ACTUAL_PROCESS:fill_i_am)))))) (SIGNALS)))");

    VMHost *v = G_vm;
    Xaddr x = _v_new_receptor(v,v->r,TEST_RECEPTOR,self);
    _v_activate(v,x);

    // now create bindings for the RECONIZEE half of the protocols
    bindings = _t_new_root(PROTOCOL_BINDINGS);
    res = _t_newr(bindings,RESOLUTION);
    w = _t_newr(res,WHICH_RECEPTOR);
    _t_news(w,ROLE,RECOGNIZEE);
    __r_make_addr(w,ACTUAL_RECEPTOR,self->addr);
    res = _t_newr(bindings,RESOLUTION);
    w = _t_newr(res,WHICH_PROCESS);
    _t_news(w,GOAL,RECOGNITION);
    _t_news(w,ACTUAL_PROCESS,NOOP);
    res = _t_newr(bindings,RESOLUTION);
    w = _t_newr(res,WHICH_VALUE);
    _t_news(w,ACTUAL_SYMBOL,are_you);
    T *val = _t_newr(w,ACTUAL_VALUE);
    val = _t_newr(val,are_you);
    _t_newr(val,SEMTREX_WALK);

    spec_is_str_equal(t2s(bindings),"(PROTOCOL_BINDINGS (RESOLUTION (WHICH_RECEPTOR (ROLE:RECOGNIZEE) (ACTUAL_RECEPTOR (RECEPTOR_ADDR:3)))) (RESOLUTION (WHICH_PROCESS (GOAL:RECOGNITION) (ACTUAL_PROCESS:NOOP))) (RESOLUTION (WHICH_VALUE (ACTUAL_SYMBOL:are_you) (ACTUAL_VALUE (are_you (SEMTREX_WALK))))))");

    /* debug_enable(D_PROTOCOL); */
    /* debug_enable(D_TREE); */
    /* debug_enable(D_SIGNALS); */
    // now initiate recognition
    _o_initiate(self,RECOGNIZE,backnforth,bindings);
    _p_reduceq(self->q);
    debug_disable(D_SIGNALS);

    // which should produce signals on the flux
    spec_is_str_equal(t2s(_t_getv(self->pending_signals,1,SignalEnvelopeIdx,EnvelopeCarrierIdx,TREE_PATH_TERMINATOR)),"(CARRIER:backnforth)");
    spec_is_str_equal(t2s(_t_getv(self->pending_signals,1,SignalBodyIdx,TREE_PATH_TERMINATOR)),"(BODY:{(are_you (SEMTREX_WALK))})");
    spec_is_str_equal(t2s(_t_getv(self->pending_responses,1,PendingResponseCarrierIdx,TREE_PATH_TERMINATOR)),"(CARRIER:backnforth)");

    _v_deliver_signals(v,self);
    // check the signal's envelope, body, and run-tree
    T *signals = __r_get_signals(self,DEFAULT_ASPECT);
    T *e = _t_clone(_t_getv(signals,1,SignalEnvelopeIdx,TREE_PATH_TERMINATOR));
    _t_free(_t_detach_by_idx(e,EnvelopeExtraIdx)); // get rid of the end conditions which are variable so we can't test them!
    spec_is_str_equal(t2s(e),"(ENVELOPE (FROM_ADDRESS (RECEPTOR_ADDR:3)) (TO_ADDRESS (RECEPTOR_ADDR:3)) (ASPECT_IDENT:DEFAULT_ASPECT) (CARRIER:backnforth) (SIGNAL_UUID))");
    _t_free(e);
    spec_is_str_equal(t2s(_t_getv(signals,1,SignalBodyIdx,TREE_PATH_TERMINATOR)),"(BODY:{(are_you (SEMTREX_WALK))})");
    spec_is_str_equal(t2s(_t_getv(signals,1,SignalBodyIdx+1,TREE_PATH_TERMINATOR)),"(RUN_TREE (process:RESPOND (SIGNAL_REF:/1/4) (process:fill_i_am)) (PARAMS))");

    //    debug_enable(D_REDUCE+D_REDUCEV);

    // which when get reduced, return the i_am result
    _p_reduceq(self->q);
    debug_disable(D_TREE);
    debug_disable(D_PROTOCOL);
    debug_disable(D_REDUCE+D_REDUCEV);

    spec_is_str_equal(_td(self,self->pending_signals),"(PENDING_SIGNALS (SIGNAL (ENVELOPE (FROM_ADDRESS (RECEPTOR_ADDR:3)) (TO_ADDRESS (RECEPTOR_ADDR:3)) (ASPECT_IDENT:DEFAULT_ASPECT) (CARRIER:backnforth) (SIGNAL_UUID) (IN_RESPONSE_TO_UUID)) (BODY:{(i_am (RECEPTOR_LABEL:super cept) (RECEPTOR_IDENTIFIER:314159))})))");
    _v_deliver_signals(v,self);

    spec_is_str_equal(t2s(self->q->active->context->run_tree),"(RUN_TREE (process:NOOP (i_am (RECEPTOR_LABEL:super cept) (RECEPTOR_IDENTIFIER:314159))) (PARAMS))");

    debug_disable(D_SIGNALS);

    //@todo cleanup! which we can't do yet because we haven't written code to remove installed receptors
}

void testProtocolAlive() {
    SemTable *sem = G_vm->r->sem;

    spec_is_str_equal(t2s(_sem_get_def(G_sem,ALIVE)),"(PROTOCOL_DEFINITION (PROTOCOL_LABEL:ALIVE) (PROTOCOL_SEMANTICS (ROLE:SERVER) (ROLE:CLIENT) (GOAL:HANDLER)) (alive (EXPECT (ROLE:SERVER) (SOURCE (ROLE:CLIENT)) (PATTERN (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:PING))) (ACTION:respond_with_yup)) (EXPECT (ROLE:CLIENT) (SOURCE (ROLE:SERVER)) (PATTERN (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:YUP))) (SLOT (GOAL:HANDLER) (SLOT_IS_VALUE_OF:ACTION)))))");

    Receptor *r =  _r_new(sem,TEST_RECEPTOR);

    _o_express_role(r,ALIVE,SERVER,DEFAULT_ASPECT,NULL);

    // check that it was expressed
    spec_is_str_equal(_td(r,r->flux),"(FLUX (DEFAULT_ASPECT (EXPECTATIONS (EXPECTATION (CARRIER:alive) (PATTERN (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:PING))) (ACTION:respond_with_yup) (PARAMS) (END_CONDITIONS (UNLIMITED)))) (SIGNALS)))");

    Receptor *r2 =  _r_new(sem,TEST_RECEPTOR);
    T *noop = _t_new_root(NOOP);
    _t_newi(noop,TEST_INT_SYMBOL,314);
    Process proc = _r_define_process(r2,noop,"do nothing","long desc...",NULL);

    T *bindings = _t_build(G_sem,0,PROTOCOL_BINDINGS,RESOLUTION,WHICH_PROCESS,GOAL,HANDLER,ACTUAL_PROCESS,proc,NULL_SYMBOL,NULL_SYMBOL,NULL_SYMBOL);

    _o_express_role(r2,ALIVE,CLIENT,DEFAULT_ASPECT,bindings);
    _t_free(bindings);

    // check that the expressed expectation is bound correctly to the handler
    spec_is_str_equal(_td(r2,r2->flux),"(FLUX (DEFAULT_ASPECT (EXPECTATIONS (EXPECTATION (CARRIER:alive) (PATTERN (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:YUP))) (ACTION:do nothing) (PARAMS) (END_CONDITIONS (UNLIMITED)) (SEMANTIC_MAP (SEMANTIC_LINK (GOAL:HANDLER) (REPLACEMENT_VALUE (ACTUAL_PROCESS:do nothing)))))) (SIGNALS)))");

    T *s = __r_make_signal(r->addr,r->addr,DEFAULT_ASPECT,alive,_t_new_root(PING),0,0);
    //debug_enable(D_SIGNALS);
    spec_is_equal(_r_deliver(r,s),noDeliveryErr);
    spec_is_str_equal(_td(r,r->q->active->context->run_tree),"(RUN_TREE (process:RESPOND (SIGNAL_REF:/1/4) (YUP)) (PARAMS))");  // responds on the carrier in the signal envelope
    debug_disable(D_SIGNALS);
    spec_is_equal(_p_reduceq(r->q),noReductionErr);
    spec_is_str_equal(_td(r,r->q->completed->context->run_tree),"(RUN_TREE (SIGNAL_UUID) (PARAMS))");
    spec_is_str_equal(_td(r,r->pending_signals),"(PENDING_SIGNALS (SIGNAL (ENVELOPE (FROM_ADDRESS (RECEPTOR_ADDR:3)) (TO_ADDRESS (RECEPTOR_ADDR:3)) (ASPECT_IDENT:DEFAULT_ASPECT) (CARRIER:alive) (SIGNAL_UUID) (IN_RESPONSE_TO_UUID)) (BODY:{(YUP)})))");

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

    Symbol act = _d_define_symbol(G_sem,INTERACTION,"act",TEST_CONTEXT);

    simple_def = _o_make_protocol_def(G_sem,TEST_CONTEXT,"do",
                                      ROLE,agent,
                                      GOAL,process,
                                      USAGE,data,
                                      INTERACTION,act,
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
    T *sem_map = _o_bindings2sem_map(bindings,NULL);
    spec_is_str_equal(t2s(sem_map),"(SEMANTIC_MAP (SEMANTIC_LINK (GOAL:process) (REPLACEMENT_VALUE (ACTUAL_PROCESS:do nothing))) (SEMANTIC_LINK (USAGE:data) (REPLACEMENT_VALUE (ACTUAL_SYMBOL:TEST_INT_SYMBOL))))");
    T *unbound = _o_resolve(G_sem,resolved,sem_map);
    spec_is_str_equal(t2s(unbound),"(PROTOCOL_SEMANTICS (ROLE:agent))");
    spec_is_str_equal(t2s(resolved),"(PROTOCOL_DEFINITION (PROTOCOL_LABEL:do) (PROTOCOL_SEMANTICS (ROLE:agent) (GOAL:process) (USAGE:data)) (act (EXPECT (ROLE:agent) (SOURCE (ROLE:agent)) (PATTERN (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:TEST_INT_SYMBOL))) (ACTION:do nothing))))");

    _t_free(bindings);
    _t_free(sem_map);
    _t_free(resolved);
}

void testProtocolUnwrap() {
    T *sem_map = _t_new_root(SEMANTIC_MAP);
    //    debug_enable(D_PROTOCOL);
    T *unwrapped = _o_unwrap(G_sem,shake_def,sem_map);
    debug_disable(D_PROTOCOL);
    spec_is_str_equal(t2s(sem_map),"(SEMANTIC_MAP (SEMANTIC_LINK (ROLE:agent) (REPLACEMENT_VALUE (ROLE:mover))) (SEMANTIC_LINK (GOAL:process) (REPLACEMENT_VALUE (GOAL:shaking))) (SEMANTIC_LINK (USAGE:data) (REPLACEMENT_VALUE (USAGE:info))))");
    _t_free(sem_map);
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
