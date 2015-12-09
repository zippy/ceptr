/**
 * @file protocol_spec.h
 * @copyright Copyright (C) 2013-2015, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 * @ingroup tests
 */

#include "../src/protocol.h"

Receptor *_makeRequestingProtocolReceptor(VMHost *v) {
    Receptor *r;
    SemTable *sem = v->r->sem;
    r = _r_new(sem,TEST_RECEPTOR);

    Symbol requester = _r_define_symbol(r,RECEPTOR_ADDRESS,"requester");
    Symbol responder = _r_define_symbol(r,RECEPTOR_ADDRESS,"responder");
    Symbol request = _r_define_symbol(r,SYMBOL,"request");
    Symbol response = _r_define_symbol(r,SYMBOL,"response");
    Symbol response_handler = _r_define_symbol(r,PROCESS,"response_handler");
    Symbol request_handler = _r_define_symbol(r,PROCESS,"request_handler");

    T *req = _t_new_root(REQUEST);
    _t_news(req,ROLE,responder);
    //@todo carrier?
    _t_news(req,USAGE,request);
    //@todo response_carrier?
    _t_news(req,GOAL,response_handler);

    Process req_proc = _r_define_process(r,req,"request","long desc...",NULL);
    T *req_action = _t_newp(0,ACTION,req_proc);

    // use the carrier it came in on as the response carrier.
    T *resp = _t_new_root(RESPOND);
    int pt[] = {SignalEnvelopeIdx,EnvelopeCarrierIdx,TREE_PATH_TERMINATOR};  // the senders address
    _t_new(resp,SIGNAL_REF,pt,sizeof(int)*4);
    _t_news(resp,GOAL,request_handler);
    Process resp_proc = _r_define_process(r,resp,"respond","long desc...",NULL);

    T *pattern = _t_new_root(PATTERN);
    T *stx = _t_newr(pattern,SEMTREX_SYMBOL_LITERAL);
    _t_news(stx,USAGE,response);
    T *resp_action = _t_news(0,GOAL,response_handler);

    T *def = _o_make_protocol_def(sem,r->context,"requesting",
                  ROLE,requester,
                  ROLE,responder,
                  GOAL,request_handler,
                  GOAL,response_handler,
                  USAGE,request,
                  USAGE,response,
                  INTERACTION,"backnforth",
                                  INITIATE,requester,responder,req_action, //say request & enable "response",
                                  EXPECT,responder,requester,pattern,resp_action,
                  NULL_SYMBOL);
    _r_define_protocol(r,def);
    return r;
}

void testProtocolRequesting() {
    Receptor *r = _makeRequestingProtocolReceptor(G_vm);
    Protocol requesting = _r_get_sem_by_label(r,"requesting");
    Symbol request =_r_get_sem_by_label(r,"request");
    Symbol response = _r_get_sem_by_label(r,"response");
    Symbol request_handler = _r_get_sem_by_label(r,"request_handler");
    Symbol response_handler = _r_get_sem_by_label(r,"response_handler");

    spec_is_str_equal(_td(r,__sem_get_defs(r->sem,SEM_TYPE_PROTOCOL,r->context)),"(PROTOCOLS (PROTOCOL_DEFINITION (PROTOCOL_LABEL:requesting) (ROLE:requester) (ROLE:responder) (GOAL:request_handler) (GOAL:response_handler) (USAGE:request) (USAGE:response) (backnforth (INITIATE (ROLE:requester) (DESTINATION (ROLE:responder)) (ACTION:request)) (EXPECT (ROLE:responder) (SOURCE (ROLE:requester)) (PATTERN (SEMTREX_SYMBOL_LITERAL (USAGE:response))) (GOAL:response_handler)))))");

    //@todo trying to express request protocol should fail because it's not concrete enough

}

Receptor *_makeRecognizeProtocolReceptor(VMHost *v) {
    Receptor *r;
    SemTable *sem = v->r->sem;
    Receptor *rqp = _makeRequestingProtocolReceptor(v);
    r = _r_new(sem,TEST_RECEPTOR);

    Symbol request = _r_get_sem_by_label(rqp,"request");
    Symbol response = _r_get_sem_by_label(rqp,"response");
    Protocol requesting = _r_get_sem_by_label(rqp,"requesting");
    Symbol requester = _r_get_sem_by_label(rqp,"requester");
    Symbol responder = _r_get_sem_by_label(rqp,"responder");
    Process request_handler = _r_get_sem_by_label(rqp,"request_handler");
    Process response_handler = _r_get_sem_by_label(rqp,"response_handler");
    Symbol recognizer = _r_define_symbol(r,RECEPTOR_ADDRESS,"recognizer");
    Symbol recognizee = _r_define_symbol(r,RECEPTOR_ADDRESS,"recognizee");
    Symbol recognition = _r_define_symbol(r,PROCESS,"recognition");

    Symbol are_you = _r_define_symbol(r,SEMTREX,"are_you");
    Symbol i_am = _r_define_symbol(r,RECEPTOR_IDENTITY,"i_am");

    T *proc = _t_new_root(i_am); //hard coded version of code for filling and I_AM structure
    _t_new_str(proc,RECEPTOR_LABEL,"super cept");
    _t_newi(proc,RECEPTOR_IDENTIFIER,314159);

    Process fill_i_am = _r_define_process(r,proc,"fill i am","long desc...",NULL);

    T *def = _o_make_protocol_def(sem,r->context,"recognize",
                               INCLUSION,requesting,
                                  WHICH_ROLE,requester,recognizer,
                                  WHICH_ROLE,responder,recognizee,
                                  WHICH_GOAL,response_handler,recognition,
                                  WHICH_SYMBOL,request,are_you,
                                  WHICH_SYMBOL,response,i_am,
                                  WHICH_PROCESS,request_handler,fill_i_am,
                                NULL_SYMBOL);
    _r_define_protocol(r,def);
    return r;
}

void testProtocolRecognize() {
    // requesting protocol has been defined in G_vm by previous test case so we can just use it
    Receptor *r = _makeRecognizeProtocolReceptor(G_vm);
    Protocol recog = _r_get_sem_by_label(r,"recognize");
    spec_is_str_equal(_td(r,__sem_get_defs(r->sem,SEM_TYPE_PROTOCOL,r->context)),"(PROTOCOLS (PROTOCOL_DEFINITION (PROTOCOL_LABEL:recognize) (INCLUSION (PNAME:requesting) (CONNECTION (WHICH_ROLE (ROLE:requester) (ROLE:recognizer))) (CONNECTION (WHICH_ROLE (ROLE:responder) (ROLE:recognizee))) (CONNECTION (WHICH_GOAL (GOAL:response_handler) (GOAL:recognition))) (RESOLUTION (WHICH_SYMBOL (USAGE:request) (ACTUAL_SYMBOL:are_you))) (RESOLUTION (WHICH_SYMBOL (USAGE:response) (ACTUAL_SYMBOL:i_am))) (RESOLUTION (WHICH_PROCESS (GOAL:request_handler) (ACTUAL_PROCESS:fill i am))))))");

    Symbol recognition = _r_get_sem_by_label(r,"recognition");
    Symbol recognizer = _r_get_sem_by_label(r,"recognizer");

    Receptor *self =  _r_new(r->sem,TEST_RECEPTOR);

    T *noop = _t_new_root(NOOP);
    _t_newi(noop,TEST_INT_SYMBOL,314);
    Process proc = _r_define_process(self,noop,"do nothing","long desc...",NULL);

    T *bindings = _t_new_root(PROTOCOL_BINDINGS);
    T *res = _t_newr(bindings,RESOLUTION);
    T *w = _t_newr(res,WHICH_PROCESS);
    _t_news(w,GOAL,recognition);
    _t_news(w,ACTUAL_PROCESS,proc);

    _o_express_role(self,recog,recognizer,DEFAULT_ASPECT,bindings);
    _t_free(bindings);

    spec_is_str_equal(_td(self,self->flux),"");

    bindings = _t_new_root(PROTOCOL_BINDINGS);
    res = _t_new_root(RESOLUTION);
    w = _t_newr(res,WHICH_PROCESS);
    //c    _t_news(w,GOAL,response_handler);
    _t_news(w,ACTUAL_PROCESS,proc);
    //    _o_express_role(self,recog,responder,DEFAULT_ASPECT,res);
    _t_free(bindings);

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
    T *action =  _t_news(0,GOAL,process);
    T *pattern = _t_new_root(PATTERN);
    T *stx = _t_newr(pattern,SEMTREX_SYMBOL_LITERAL);
    _t_news(stx,USAGE,data);

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
    T *resolved = _o_resolve(G_sem,simple_def,agent,bindings);
    spec_is_str_equal(t2s(simple_def),"(PROTOCOL_DEFINITION (PROTOCOL_LABEL:do) (ROLE:agent) (GOAL:process) (USAGE:data) (act (EXPECT (ROLE:agent) (SOURCE (ROLE:agent)) (PATTERN (SEMTREX_SYMBOL_LITERAL (USAGE:data))) (GOAL:process))))");
    spec_is_str_equal(t2s(resolved),"(PROTOCOL_DEFINITION (PROTOCOL_LABEL:do) (ROLE:agent) (GOAL:process) (USAGE:data) (act (EXPECT (ROLE:agent) (SOURCE (ROLE:agent)) (PATTERN (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:TEST_INT_SYMBOL))) (ACTION:do nothing))))");

    _t_free(bindings);
    _t_free(resolved);
}

void testProtocolUnwrap() {
    T *unwrapped = _o_unwrap(G_sem,shake_def);
    spec_is_str_equal(t2s(unwrapped),"(PROTOCOL_DEFINITION (PROTOCOL_LABEL:shake) (ROLE:mover) (ROLE:shaker) (GOAL:shaking) (USAGE:info) (act (EXPECT (ROLE:mover) (SOURCE (ROLE:mover)) (PATTERN (SEMTREX_SYMBOL_LITERAL (USAGE:info))) (GOAL:shaking))))");
    //_t_free(unwrapped);
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
    /* testProtocolUnwrap(); */
    /* testProtocolRequesting(); */
    /* testProtocolRecognize(); */
    testProtocolAlive();
    _v_free(G_vm);
}
