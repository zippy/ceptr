/**
 * @ingroup receptor
 *
 * @{
 * @file protocol.c
 * @brief implementation of protocol helpers
 *
 * @copyright Copyright (C) 2013-2016, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 */

#include "protocol.h"
#include "vmhost.h"
#include "semtrex.h"
#include "debug.h"

extern VMHost *G_vm;

void *_o_add_label(char *label,T *def) {
    T *l = _t_new_root(PROTOCOL_LABEL);
    _t_new_str(l,ENGLISH_LABEL,label);
    int path[2] = {1,TREE_PATH_TERMINATOR};
    _t_insert_at(def,path,l);
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
 * @returns the new protocol def
 *
 */
T *_o_make_protocol_def(SemTable *sem,Context c,char *label,...) {
    va_list params;
    va_start(params,label);

    Symbol state = PROTOCOL_SEMANTICS;
    bool in_conv = false;
    T *p = _t_new_root(PROTOCOL_DEFINITION);
    T *l = _t_newr(p,PROTOCOL_LABEL);
    _t_new_str(l,ENGLISH_LABEL,label);
    T *t = _t_newr(p,PROTOCOL_SEMANTICS);
    bool done = false;
    Symbol param;
    bool pop = false;
    while(!done) {
        if (pop) {
            t = _t_parent(t);
            p = _t_parent(t);
            state = _t_symbol(t);
            if (semeq(_sem_get_symbol_structure(sem,state),INTERACTION)) state = INTERACTION;

            pop = false;
            debug(D_PROTOCOL,"popping to state %s with param %s\n",_sem_get_name(sem,state),_sem_get_name(sem,param));
        }
        else {
            param = va_arg(params,Symbol);
            debug(D_PROTOCOL,"reading %s in state %s\n",_sem_get_name(sem,param),_sem_get_name(sem,state));
        }
        if (semeq(state,PROTOCOL_DEFINITION)) {
            if (semeq(param,INTERACTION)) {
                state = INTERACTION;
            }
            else if (semeq(param,INCLUSION)) {
                p = t;
                state = INCLUSION;
            }
            else if (semeq(param,NULL_SYMBOL)) {
                done = true;
            }
            else raise_error("expecting ROLE,GOAL,USAGE,INCLUSION,INTERACTION or NULL_SYMBOL got %s",_sem_get_name(sem,param));
        }
        if (semeq(state,PROTOCOL_SEMANTICS)) {
            if (semeq(param,ROLE)) {
                Symbol role = va_arg(params,Symbol);
                _t_news(t,ROLE,role);
            }
            else if (semeq(param,GOAL)) {
                Symbol role = va_arg(params,Symbol);
                _t_news(t,GOAL,role);
            }
            else if (semeq(param,USAGE)) {
                Symbol role = va_arg(params,Symbol);
                _t_news(t,USAGE,role);
            }
            else pop = true;
        }
        if (semeq(state,INTERACTION)) {
            if (semeq(param,INTERACTION)) {
                Symbol interaction =  va_arg(params,Symbol);
                t = _t_newr(t,interaction);
            }
            else if (semeq(param,EXPECT) || semeq(param,INITIATE)) {
                p = t;
                state = param;
            }
            else pop = true;
        }
        if (semeq(state,EXPECT)) {
            if (semeq(param,EXPECT)) {
                t = _t_newr(p,EXPECT);
                Symbol role = va_arg(params,Symbol);
                _t_news(t,ROLE,role);
                T *s = _t_newr(t,SOURCE);
                Symbol source = va_arg(params,Symbol);
                _t_news(s,ROLE,source);
                T *pattern = va_arg(params,T*);
                _t_add(t,pattern);
                T *action = va_arg(params,T*);
                _t_add(t,action);
            }
            else pop = true;
        }
        if (semeq(state,INITIATE)) {
            if (semeq(param,INITIATE)) {
                t = _t_newr(p,INITIATE);
                Symbol role = va_arg(params,Symbol);
                _t_news(t,ROLE,role);
                T *s = _t_newr(t,DESTINATION);
                Symbol source = va_arg(params,Symbol);
                _t_news(s,ROLE,source);
                T *action = va_arg(params,T*);
                _t_add(t,action);
            }
            else pop = true;
        }
        if (semeq(state,INCLUSION)) {
            if (semeq(param,INCLUSION)) {
                t = _t_newr(p,INCLUSION);
                Symbol pname = va_arg(params,Symbol);
                _t_news(t,PNAME,pname);
            }
            else if (semeq(param,WHICH_SYMBOL)) {
                T *l = _t_newr(t,RESOLUTION);
                T *w = _t_newr(l,WHICH_SYMBOL);
                _t_news(w,USAGE,va_arg(params,Symbol));
                _t_news(w,ACTUAL_SYMBOL,va_arg(params,Symbol));
            }
            else if (semeq(param,WHICH_PROCESS)) {
                T *l = _t_newr(t,RESOLUTION);
                T *w = _t_newr(l,WHICH_PROCESS);
                _t_news(w,GOAL,va_arg(params,Process));
                _t_news(w,ACTUAL_PROCESS,va_arg(params,Process));
            }
            else if (semeq(param,WHICH_RECEPTOR)) {
                T *l = _t_newr(t,RESOLUTION);
                T *w = _t_newr(l,WHICH_RECEPTOR);
                _t_news(w,ROLE,va_arg(params,Symbol));
                _t_news(w,ACTUAL_RECEPTOR,va_arg(params,Symbol));
            }
            else if (semeq(param,WHICH_USAGE)) {
                T *l = _t_newr(t,LINKAGE);
                T *w = _t_newr(l,WHICH_USAGE);
                _t_news(w,USAGE,va_arg(params,Symbol));
                _t_news(w,USAGE,va_arg(params,Symbol));
            }
            else if (semeq(param,WHICH_GOAL)) {
                T *l = _t_newr(t,LINKAGE);
                T *w = _t_newr(l,WHICH_GOAL);
                _t_news(w,GOAL,va_arg(params,Process));
                _t_news(w,GOAL,va_arg(params,Process));
            }
            else if (semeq(param,WHICH_ROLE)) {
                T *l = _t_newr(t,LINKAGE);
                T *w = _t_newr(l,WHICH_ROLE);
                _t_news(w,ROLE,va_arg(params,Symbol));
                _t_news(w,ROLE,va_arg(params,Symbol));
            }
            else pop = true;
        }
    }
    va_end(params);
    return t;
}


// hmmm this is kind of like template replace but just looks
// for the semantic references, not for SLOTs
void _t_replace_sem_refs(T *t, T *sem_map) {
    if (!t) return;

    Symbol sym = _t_symbol(t);
    if (semeq(sym,GOAL)||semeq(sym,ROLE)||semeq(sym,USAGE)) {
        Symbol valsym = *(Symbol *)_t_surface(t);

        // scan all the sem_map for semantic refs that match this slot.
        // @todo convert this to a hashtable based implementation, probably on the treehash of the semantic ref
        int i,c = _t_children(sem_map);
        for(i=1;i<=c;i++) {
            T *m = _t_child(sem_map,i);
            T *ref = _t_child(m,SemanticMapSemanticRefIdx);
            if (semeq(sym,_t_symbol(ref)) && semeq(valsym,*(Symbol *)_t_surface(ref))) {
                T *r;
                T *replacement_value = _t_child(_t_child(m,SemanticMapReplacementValIdx),1);
                r = _t_clone(replacement_value);
                _t_replace_node(t,r);
                break;
            }
        }
    }
    else {
        DO_KIDS(t,_t_replace_sem_refs(_t_child(t,i),sem_map));
    }
}

// convert PROTOCOL_BINDINGS to SEMANTIC_MAP
T *_o_bindings2sem_map(T *bindings, T *sem_map) {
    if (!sem_map)
        sem_map = _t_new_root(SEMANTIC_MAP);
    DO_KIDS(bindings,
            T *res = _t_child(bindings,i);
            T *w = _t_child(res,ResolutionWhichIdx);
            T *t = _t_newr(sem_map,SEMANTIC_LINK);
            _t_add(t,_t_clone(_t_child(w,1)));
            T *r = _t_newr(t,REPLACEMENT_VALUE);
            _t_add(r,_t_clone(_t_child(w,2)));
            );
    debug(D_PROTOCOL,"converting bindings %s\n",t2sp(bindings));
    debug(D_PROTOCOL,"to sem map %s\n",t2sp(sem_map));
    return sem_map;
}

/**
 * recursively expand wrapped protocol defs
 *
 * @param[in] sem SemTable of contexts
 * @param[in] def protocol definition tree
 * @param[in,out] sem_map semantic map of the unwrapped protocols
 * @returns unwrapped protocol definition tree
 */
T * _o_unwrap(SemTable *sem,T *def,T *sem_map) {
    T *d = _t_clone(def);
    int i;
    for (i=1;i<=_t_children(d);i++) {
        T *t = _t_child(d,i);
        if (semeq(_t_symbol(t),INCLUSION)) {
            Protocol p = *(Protocol *)_t_surface(_t_child(t,InclusionPnameIdx));
            T *ps = _sem_get_defs(sem,p);
            T *p_def = _o_unwrap(sem,_t_child(ps,p.id),sem_map);  // do the recursive unwrapping
            int j,c = _t_children(t);
            T *bindings = NULL;
            for(j=InclusionPnameIdx+1;j<=c;j++) {
                T *x = _t_child(t,j); // get the connection or resolution
                if (semeq(_t_symbol(x),LINKAGE)) {
                    T *w = _t_child(x,ConnectionWhichIdx);  // get the which
                    T *t = _t_newr(sem_map,SEMANTIC_LINK);
                    _t_add(t,_t_clone(_t_child(w,1)));
                    T *r = _t_newr(t,REPLACEMENT_VALUE);
                    _t_add(r,_t_clone(_t_child(w,2)));
                }
                else if (semeq(_t_symbol(x),RESOLUTION)) {
                    if (!bindings) bindings = _t_new_root(PROTOCOL_BINDINGS);
                    _t_add(bindings,_t_clone(x));
                }
                else raise_error("expecting LINKAGE or RESOLUTION");
            }

            if (sem_map) {
                debug(D_PROTOCOL,"filling template %s\n",__t2s(sem,p_def,INDENT));
                debug(D_PROTOCOL,"with %s\n",__t2s(sem,sem_map,INDENT));
                _t_replace_sem_refs(p_def,sem_map);
                debug(D_PROTOCOL,"result %s\n",__t2s(sem,p_def,INDENT));
            }

            T *unbound_semantics;
            if (bindings) {
                _o_bindings2sem_map(bindings,sem_map);
                unbound_semantics = _o_resolve(sem,p_def,sem_map);
                _t_free(bindings);
                _t_free(_t_detach_by_idx(p_def,ProtocolDefSemanticsIdx));
            }
            else {
                unbound_semantics = _t_detach_by_idx(p_def,ProtocolDefSemanticsIdx);
            }

            // after doing the semantics mapping from the LINKAGEs and RESOLUTIONs
            // we need to add into the parent semantics and sem_map that weren't resolved
            // or connected for later binding

            T *parent_semantics = _t_child(d,ProtocolDefSemanticsIdx);
            T *x;
            while(x = _t_detach_by_idx(unbound_semantics,1)) {
                Symbol sym = _t_symbol(x);
                c = _t_children(parent_semantics);
                for (j=1;j<=c;j++) {
                    if (semeq(sym,_t_symbol(_t_child(parent_semantics,j)))) break;
                }
                // not found so we can add it
                if (j>c) {
                    _t_add(parent_semantics,x);
                    debug(D_PROTOCOL,"adding %s to semantics\n",_sem_get_name(sem,sym));
                }
                else _t_free(x);
            }
            _t_detach_by_ptr(d,t);  // remove the INCLUSION specs

            // add in the unwrapped interactions
            while(x = _t_detach_by_idx(p_def,ProtocolDefSemanticsIdx)) {
                _t_add(d,x);
            }
            _t_free(unbound_semantics);
            _t_free(p_def);
        }
    }
    debug(D_PROTOCOL,"unwrapped to: %s\n",__t2s(sem,d,INDENT));

    return d;
}


/**
 * convert a, possibly composed, protocol definition into an expressable definition
 *
 * convert a PROTOCOL_DEF to a concretized version according to the
 * RESOLUTIONs and LINKAGEs in the the def and the bindings
 *
 * @param[in] sem SemTable contexts
 * @param[in,out] def protocol definition to resolve
 * @param[in] role the role that needs resolving
 * @param[in] sem_map used to resolve the bindings
 * @returns a PROTOCOL_SEMANTICS tree of semantics that remain unbound
 */
T * _o_resolve(SemTable *sem,T *def, T *sem_map){

    // calculate the list of still unbound semantics
    T *semantics = _t_clone(_t_child(def,ProtocolDefSemanticsIdx));

    if (sem_map) {
        _t_fill_template(def,sem_map);
        int i,c = _t_children(sem_map);
        for(i=1;i<=c;i++) {
            T *x = _t_child(_t_child(sem_map,i),SemanticMapSemanticRefIdx);
            Symbol symx = _t_symbol(x);
            Symbol symxs = *(Symbol *) _t_surface(x);
            int j,b = _t_children(semantics);
            // search the semantics for matching item and remove it.
            for(j=1;j<=b;j++) {
                T *y = _t_child(semantics,j);
                if (semeq(symx,_t_symbol(y)) && semeq(symxs,*(Symbol *) _t_surface(y))) {
                    _t_free(_t_detach_by_idx(semantics,j));
                    break;
                }
            }
        }
    }

    return semantics;
}

/**
 * setup a receptor to participate as some role in a protocol
 *
 * @param[in] receptor in which to express the protocol role
 * @param[in] protocol which protocol
 * @param[in] role symbol of role to express
 * @param[in] aspect on which aspect to install the expectations
 * @param[in] bindings PROTOCOL_BINDINGs tree to concretize the unknowns in the protocol def
 *
 */
void _o_express_role(Receptor *r,Protocol protocol,Symbol role,Aspect aspect,T *bindings) {
    T *p = _sem_get_def(r->sem,protocol);
    if (!p) raise_error("protocol %s not found",_sem_get_name(r->sem,protocol));
    T *sem_map = _t_new_root(SEMANTIC_MAP);
    p = _o_unwrap(r->sem,p,sem_map);  // creates a cloned, uwrapped protocol def.
    if (bindings) {
        _o_bindings2sem_map(bindings,sem_map);
    }
    T *unbound = _o_resolve(r->sem,p,sem_map);
    debug(D_PROTOCOL,"express %s yelids unbound: %s\n",_sem_get_name(r->sem,role),_t2s(r->sem,unbound));
    debug(D_PROTOCOL,"express %s yelids sem_map: %s\n",_sem_get_name(r->sem,role),_t2s(r->sem,sem_map));
    if (!_t_children(sem_map)) {
        _t_free(sem_map);
        sem_map = NULL;
    }
    //@todo convert this search to be repeat Semtex matching on INTERACTION structure...
    T *t;
    int i,c=_t_children(p);
    for(i=1;i<=c;i++) {
        t = _t_child(p,i);
        Symbol interaction = _t_symbol(t);
        if (semeq(_sem_get_symbol_structure(r->sem,interaction),INTERACTION)) {
            int j;
            int b = _t_children(t);
            for(j=1;j<=b;j++) {
                T *x = _t_child(t,j);
                if (semeq(_t_symbol(x),EXPECT)) {
                    T *rl = _t_child(x,ExpectRoleIdx);
                    if (semeq(*(Symbol *)_t_surface(rl),role)) {
                        T *pattern = _t_clone(_t_child(x,ExpectPatternIdx));
                        // @todo check pattern for unbound USAGEs

                        T *a = _t_clone(_t_child(x,ExpectActionIdx));
                        if (!bindings && semeq(_t_symbol(a),GOAL)) {
                            raise_error("binding missing for GOAL:%s in %s",_sem_get_name(r->sem,*(SemanticID *)_t_surface(a)),_t2s(r->sem,x));
                        }
                        T *sm = sem_map ? _t_clone(sem_map) : NULL;
                        T *params = _t_child(x,ExpectParamsIdx);
                        if (params) params = _t_clone(params);
                        T *e = __r_build_expectation(interaction,pattern,a,params,0,sm,NULL);
                        debug(D_PROTOCOL,"express %s adds expectation: %s\n",_sem_get_name(r->sem,role),_t2s(r->sem,e));
                        __r_add_expectation(r,aspect,e);
                    }
                }
            }
        }
    }
    _t_free(p);
    if (unbound) _t_free(unbound);
    if (sem_map) _t_free(sem_map);
}

/**
 * build a run-tree and semantic map that will initiate a protocol interaction
 *
 * @param[in] receptor initiating the protocol interaction
 * @param[in] protocol which protocol
 * @param[in] interaction which interaction in the protocol
 * @param[in] bindings tree of PROTOCOL_BINDINGS
 * @param[out] sem_map semantic map built by the protocol
 *
 * @returns run-tree
 */

T * __o_initiate(Receptor *r,SemanticID protocol,SemanticID interaction,T *bindings,T **sem_mapP) {
    T *p = _sem_get_def(r->sem,protocol);
    if (!p) raise_error("protocol %s not found",_sem_get_name(r->sem,protocol));
    T *sem_map = _t_new_root(SEMANTIC_MAP);
    p = _o_unwrap(r->sem,p,sem_map);  // creates a cloned, uwrapped protocol def.
    if (bindings) {
        _o_bindings2sem_map(bindings,sem_map);
    }
    T *ia = _t_find(p,interaction);
    if (!ia) raise_error("interaction '%s' not found in protocol definition",_sem_get_name(r->sem,interaction));
    T *initiate = _t_find(ia,INITIATE);
    if (!initiate) raise_error("no INITIATE found in %s interaction",_sem_get_name(r->sem,interaction));
    //@todo somehow check that we've expressed the role in INITIATE

    T *action = _t_child(initiate,InitiateActionIdx);
    Process proc = *(Process*) _t_surface(action);
    T *params = _t_new_root(PARAMS);  //@todo fix this, would should probably get params from the INITIATE?
    T *rt = _p_make_run_tree(r->sem,proc,params,sem_map);
    _t_free(params);
    _t_free(p);

    *sem_mapP = sem_map;
    return rt;
}

/**
 * initiate the first signal in a protocol interaction
 *
 * @param[in] receptor initiating the protocol interaction
 * @param[in] protocol which protocol
 * @param[in] interaction which interaction in the protocol
 * @param[in] bindings tree of PROTOCOL_BINDINGS
 */
void _o_initiate(Receptor *r,SemanticID protocol,SemanticID interaction,T *bindings) {
    T *sem_map;
    T *rt = __o_initiate(r,protocol,interaction,bindings,&sem_map);
    __p_addrt2q(r->q,rt,sem_map);
}


/** @}*/
