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

// helper to build a PROTOCOL_DEF tree
// this just makes it easier to build protocol defs in c.  If you don't
// get the params right when calling it, it will segfault!
T *_o_new(Receptor *r,Symbol protocol,...) {
    va_list params;
    va_start(params,protocol);

    bool in_conv = false;
    T *p = _t_new_root(protocol);
    bool done = false;
    T *c;
    while(!done) {
        Symbol param = va_arg(params,Symbol);
        if (!in_conv) {
            if (semeq(param,ROLE)) {
                Symbol role = va_arg(params,Symbol);
                _t_news(p,ROLE,role);
            }
            else if (semeq(param,CONVERSATION)) {
                in_conv = true;
            }
            else if (semeq(param,NULL_SYMBOL)) {
                done = true;
            }
            else raise_error("expecting ROLE or CONVERSATION or NULL_SYMBOL");
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
                if (semeq(source,ANYBODY))
                    _t_newr(s,ANYBODY);
                else
                    _t_news(s,ROLE,source);
                T *expectation = va_arg(params,T*);
                _t_add(rp,expectation);
            }
        }
        else {
        }
    }
    va_end(params);
    T *ps = r->defs.protocols;
    _t_add(ps,p);
    return p;
}

// search the children of a tree for symbol matching sym
T *_t_find_child(T *t,Symbol sym) {
    T *p;
    DO_KIDS(t,if (semeq(_t_symbol(p=_t_child(t,i)),sym)) return p;);
    return NULL;
}

/* add the appropriate expectations into a receptor as specified by a role in a protocol definiton*/
// @todo binding currently will only be a process to substitute into an action.  This will probably change!
void _o_express_role(Receptor *r,Symbol protocol,Symbol role,Aspect aspect,T *bindings) {
    T *ps = r->defs.protocols;
    T *p = _t_find_child(ps,protocol);
    if (!p) raise_error("protocol not found");
    //@todo convert this search to be repeat Semtex matching
    T *c;
    DO_KIDS(p,if (semeq(_t_symbol(c = _t_child(p,i)),CONVERSATION)) {
            int j;
            int x = _t_children(c);
            for(j=ConversationRoleFirstProcessIdx;j<=x;j++) {
                T *rp = _t_child(c,j);
                T *rpr = _t_child(rp,RoleProcessRoleIdx);
                if (semeq(*(Symbol *)_t_surface(rpr),role)) {
                    T *e = _t_clone(_t_child(rp,RoleProcessExpectation));
                    T *a = _t_child(e,ExpectationActionIdx);
                    if (!bindings && semeq(*(Process *)_t_surface(a),NULL_PROCESS)) {
                        raise_error("binding missing for unspecified protocol action");
                    }
                    if (bindings) {
                        if (!semeq(*(Process *)_t_surface(a),NULL_PROCESS)) {
                            raise_error("binding provided for an already specified protocol action");
                        }
                        *(Process *)_t_surface(a) = *(Process *)_t_surface(bindings);
                    }
                    __r_add_expectation(r,aspect,e);
                }
            }
        };);
}
/** @}*/
