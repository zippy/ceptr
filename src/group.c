/**
 * @ingroup receptor
 *
 * @{
 * @file group.c
 * @brief implementation of some group communication receptors
 *
 * @copyright Copyright (C) 2013-2015, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 */

#include "group.h"
#include "protocol.h"

#define C_authChanId 1

/*
Receptor *_makeTokenAuthProtocolReceptor() {
    Receptor *r;
    r = _r_new(TEST_RECEPTOR);
    Symbol token_auth = _r_define_symbol(r,PROTOCOL_DEF,"authorizing");
    Symbol token = _r_define_symbol(r,INTEGER,"token");
    Symbol authenticator = _r_define_symbol(r,RECEPTOR_ADDRESS,"authenticator");
    Symbol authenticatee = _r_define_symbol(r,RECEPTOR_ADDRESS,"authenticatee");
    Symbol verifier = _r_define_symbol(r,RECEPTOR_ADDRESS,"verifier");

    e1 = {
        let t = new_token;
        associate t with authenticatee in scape;
        return t;}
    e2 = {
        if authenticatee associated with token
        send action to verifier;
        else return error;
    }
    T *t_d = _o_new(r,token_auth,
                    ROLE,authenticator,
                    ROLE,authenticatee,
                    ROLE,verifier,
                    CONVERSATION,"bind",
                    authenticator,authenticatee,e1,
                    CONVERSATION,"do",
                    authenticatee,authenticator,e2,
                    NULL_SYMBOL
                    );

    return r;

}

//Akoff scale:
// data-bits/information-structure/knowledge-semantics/wisdom

_o_new(r,authoring,"authoring"
       ROLE,author,
       ROLE,medium,
       ROLE,reader,
       ROLE,editor,
       CONVERSATION "create",
         medium,author,{store resource;return post id;}
       CONVERSATION "read",
         medium,reader,{return resource by id; }  // we should also add author and editor to the source list
       CONVERSATION "update",
         medium,author,{set resource by id;} // we should also add editor to the source list
       CONVERSATION "delete",
         medium,author,{delete resource by id;}
       )

_o_new(r,following,"following"  //watching tuning-into listening
       ROLE,channel,
       ROLE,subscriber,
       ROLE,poster,
       CONVERSATION,"subscribe",
         channel,subscriber,{e = LISTEN,new post scape,RESPOND(to the subscribe request) with post value},
       BIND_WITH,authoring {
       ROLES:
           channel -> medium;
           poster -> author;
           subscriber -> reader;
       SYMBOLS:
           post -> resource;
       ACTIONS:
       }

*/
Receptor *makeGroup(VMHost *v,char *label) {
    SemTable *sem = v->r->sem;
    Symbol group = _r_define_symbol(v->r,RECEPTOR,label);
    Receptor *r = _r_new(sem,group);
    /* Xaddr groupx = _v_new_receptor(v,v->r,group,r); */

    /* Symbol channel_addr = _r_define_symbol(r,RECEPTOR_ADDRESS,"channel"); */
    /* Symbol member =  _r_define_symbol(r,RECEPTOR_ADDRESS,"member"); */
    /* Symbol group_addr = _r_define_symbol(r,RECEPTOR_ADDRESS,"group"); */
    /* Symbol proxy_addr = _r_define_symbol(r,RECEPTOR_ADDRESS,"proxy"); */
    /* //    Symbol create = _r_define_symbol(r,NULL_STRUCTURE,"create"); */
    /* Symbol join = _r_define_symbol(r,NULL_STRUCTURE,"talking join"); */
    /* Symbol read = _r_define_symbol(r,NULL_STRUCTURE,"talking read"); */
    /* Symbol post_id = _r_define_symbol(r,INTEGER,"post_id"); */

    /* T *e_join,*e_sub,*e_crud,*e_act; */
    /* T *g_d = _o_new(r,group,talking, */
    /*                 ROLE,channel_addr, */
    /*                 ROLE,group_addr, */
    /*                 ROLE,proxy_addr, */
    /*                 ROLE,member, */
    /*                 CONVERSATION,"membership", */
    /*                     group_addr,member,e_join, */
    /*                 CONVERSATION,"subscribe", */
    /*                     channel_addr,proxy_addr,e_sub, */
    /*                 CONVERSATION,"crud", */
    /*                     proxy_addr,member,e_crud, */
    /*                 CONVERSATION,"act", */
    /*                     member,proxy_addr,e_act, */
    /*                 NULL_SYMBOL); */

    /* T *proc = _t_new_root(IF); */
    /* T *t = _t_newr(proc,SAY); */
    /* __r_make_addr(t,TO_ADDRESS,C_authChanId);  // @todo find the right address value to use by convention for the auth channel */
    /* _t_news(t,ASPECT_IDENT,DEFAULT_ASPECT);    // @todo, generalize to allow groups on different aspects */
    /* _t_news(t,CARRIER,talking); */
    /* T *w = _t_newr(t,read); */
    /* int pt[] = {SignalEnvelopeIdx,EnvelopeFromIdx,TREE_PATH_TERMINATOR};  // the senders address */
    /* _t_new(w,SIGNAL_REF,pt,sizeof(int)*4); */

    /* t = _t_newr(proc,RESPOND); */
    /* _t_news(t,RESPONSE_CARRIER,talking); */

    //  T *e = __r_build_expectation(join,join_stx,join_process,0,0);


    /* group { */
    /*     role { */
    /*         channel; */
    /*         proxy; */
    /*         group; */
    /*     } */
    /*     conversations { */
    /*         membership  { */
    /*             group expects join from ! { */
    /*                 if (read auth_channel for !) { */
    /*                     create token; */
    /*                     send to proxy for ! token; */
    /*                     respond with token; */
    /*                 } */
    /*                 else { */
    /*                     respond with err; */
    /*                 } */
    /*             } */
    /*         } */
    /*         subscribe { */
    /*             channel expects sub from proxy { */
    /*                 }*/
    /*         } */
    /*         crud post from proxy to channel (some channels have side-effects); */
    /*         act from ! to proxy; */
    /*     } */
    /* } */




    return r;
}

/** @}*/
