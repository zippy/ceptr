/**
 * @ingroup receptor
 *
 * @{
 * @file protocol.c
 * @brief implementation of protocol helpers
 *
 * @copyright Copyright (C) 2013-2015, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 */

#include "protocol.h"

#include "vmhost.h"
extern VMHost *G_vm;
// search the children of a tree for symbol matching sym
T *_t_find_child(T *t,Symbol sym) {
    T *p;
    DO_KIDS(t,
            puts(_sem_get_name(G_vm->r->sem,_t_symbol(_t_child(t,i))));
            if (semeq(_t_symbol(p=_t_child(t,i)),sym)) return p;

            );
    return NULL;
}

/**
 * helper to build a PROTOCOL_DEFINITION
 *
 * NOTE: this just makes it easier to build protocol defs in c.  If you don't
 * get the params right when calling it, it will segfault!
 *
 * @param[inout] protocols a protocols def tree containing protocols which will be added to
 * @param[in] label the name of the protocol
 * @param[in] ... params in correct order
 *
 */
T *_o_make_protocol_def(char *label,...) {
    va_list params;
    va_start(params,label);

    bool in_conv = false;
    T *p = _t_new_root(PROTOCOL_DEFINITION);
    _t_new_str(p,PROTOCOL_LABEL,label);
    bool done = false;
    T *c;
    while(!done) {
        Symbol param = va_arg(params,Symbol);
        if (!in_conv) {
            if (semeq(param,ROLE)) {
                Symbol role = va_arg(params,Symbol);
                _t_news(p,ROLE,role);
            }
            else if (semeq(param,GOAL)) {
                Symbol role = va_arg(params,Symbol);
                _t_news(p,GOAL,role);
            }
            else if (semeq(param,USAGE)) {
                Symbol role = va_arg(params,Symbol);
                _t_news(p,USAGE,role);
            }
            else if (semeq(param,CONVERSATION)) {
                in_conv = true;
            }
            else if (semeq(param,NULL_SYMBOL)) {
                done = true;
            }
            else raise_error("expecting ROLE,GOAL,USAGE or CONVERSATION or NULL_SYMBOL");
        }
        if (in_conv) {
            if (semeq(param,CONVERSATION)) {
                c = _t_newr(p,CONVERSATION);
                char *l = va_arg(params,char*);
                _t_new_str(c,CONVERSATION_LABEL,l);
                param = va_arg(params,Symbol);
            }
            else if (semeq(param,NULL_SYMBOL)) {
                done = true;
            }
            // we are adding in an role_process
            if (!done) {
                T *rp = _t_newr(c,ROLE_PROCESS);
                _t_news(rp,ROLE,param);
                T *s = _t_newr(rp,SOURCE);
                Symbol source = va_arg(params,Symbol);
                _t_news(s,ROLE,source);
                T *pattern = va_arg(params,T*);
                _t_add(rp,pattern);
                T *action = va_arg(params,T*);
                _t_add(rp,action);
            }
        }
        else {
        }
    }
    va_end(params);
    return p;
}

/* add the appropriate expectations into a receptor as specified by a role in a protocol definiton*/
// @todo binding currently will only be a process to substitute into an action.  This will probably change!
void _o_express_role(Receptor *r,Protocol protocol,Symbol role,Aspect aspect,T *bindings) {
    T *ps = _sem_get_defs(r->sem,protocol);
    T *p = _t_child(ps,protocol.id);
    if (!p) raise_error("protocol not found");
    //@todo convert this search to be repeat Semtex matching on CONVERSATION
    T *c;
    DO_KIDS(p,if (semeq(_t_symbol(c = _t_child(p,i)),CONVERSATION)) {
            int j;
            int x = _t_children(c);
            for(j=ConversationRoleFirstProcessIdx;j<=x;j++) {
                T *rp = _t_child(c,j);
                T *rpr = _t_child(rp,RoleProcessRoleIdx);
                if (semeq(*(Symbol *)_t_surface(rpr),role)) {
                    T *pattern = _t_clone(_t_child(rp,RoleProcessPatternIdx));
                    T *a = _t_clone(_t_child(rp,RoleProcessActionIdx));
                    if (!bindings && semeq(_t_symbol(a),GOAL)) {
                        raise_error("binding missing for unspecified protocol role");
                    }
                    if (bindings) {
                        if (!semeq(_t_symbol(a),GOAL)) {
                            raise_error("binding provided for an already specified protocol action");
                        }

                        // morph the surface directly instead of calling __t_morph
                        // this works because a GOAL's surface is a SYMBOL which is the
                        // same size as an ACTION's surface (a PROCESS) as they are both
                        // just SemanticIDs
                        *(Process *)_t_surface(a) = *(Process *)_t_surface(bindings);
                        a->contents.symbol = ACTION;
                    }
                    T *e = __r_build_expectation(protocol,pattern,a,0,0);
                    __r_add_expectation(r,aspect,e);
                }
            }
        };);
}
/** @}*/
