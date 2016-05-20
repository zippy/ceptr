/**
 * @ingroup receptor
 *
 * @{
 * @file process.c
 * @brief implementation of ceptr processing: instructions and run tree reduction
 * @todo implement a way to define sys_processes input and output signatures
 *
 * @copyright Copyright (C) 2013-2016, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 */
#include "process.h"
#include "def.h"
#include "semtrex.h"
#include <stdarg.h>
#include "receptor.h"
#include "../spec/spec_utils.h"
#include "util.h"
#include "debug.h"
#include <errno.h>
#include "accumulator.h"
#include "protocol.h"
void rt_check(Receptor *r,T *t) {
    if (!(t->context.flags & TFLAG_RUN_NODE)) raise_error("Whoa! Not a run node! %s\n",_td(r,t));
}

uint32_t get_rt_cur_child(Receptor *r,T *tP) {
    rt_check(r,tP);
    return (((rT *)tP)->cur_child);
}

void set_rt_cur_child(Receptor *r,T *tP,uint32_t idx) {
    rt_check(r,tP);
    (((rT *)tP)->cur_child) = idx;
}

void processUnblocker(Stream *st) {
    int err;
    // we might need to wait for the process to actually be blocked in the processing thread
    // so if unblock reports that the process isn't blocked, then just wait for a bit.
    // @todo, this is really ugly and could lead to deadlock, so we should fix it somehow!

    // sometimes the unblocker gets called by the stream completion after the actual
    // code finished running for what ever reason,so if the code is already dead, we
    // just ignore that case.
    // @todo, this is gonna bite our ass, and should be fixed
    int i = 1000;
    while ((i-- > 0) && (err = _p_unblock((Q *)st->callback_arg1,st->callback_arg2))) {
        if (err == 2) return; //raise_error("can't unblock completed process!");
        if (err == 3) return; //raise_error("can't unblock non-existent process");
        sleepms(1);
    }
    if (err) raise_error("process never blocked!");
}

// setup the default until condition (only once, and 30 second timeout)
T *defaultRequestUntil() {
    T *until = _t_newr(0,END_CONDITIONS);
    T *ts = __r_make_timestamp(TIMEOUT_AT,30);
    _t_add(until,ts);
    _t_newi(until,COUNT,1);
    return until;
}

/**
 * implements the FILL_FROM_MATCH process
 *
 * replaces the template tree with the matched sub-parts from a semtrex match results tree
 *
 * @param[in] sem current semantic context
 * @param[in] t template tree to be filled
 * @param[in] match_results SEMTREX_MATCH_RESULTS tree
 * @param[in] match_tree original tree that was matched (needed to build SEMANTIC_MAP)
 *
 * @todo what to do if match has sibs??
 */
void _p_fill_from_match(SemTable *sem,T *t,T *match_results,T *match_tree) {
    T *sem_map = _stx_results2sem_map(sem,match_results,match_tree);
    __t_fill_template(t,sem_map,true);
    _t_free(sem_map);
}

Process _p_get_transcoder(SemTable *sem,Symbol src_sym,Symbol to_sym) {

    if (semeq(HTTP_RESPONSE,src_sym) && semeq(LINES,to_sym)) {
        return http_response_2_lines;
    }
    else if (semeq(CONTENT_TYPE,src_sym) && semeq(LINE,to_sym))
        return content_type_2_line;
    else if (semeq(ASCII_CHARS,src_sym) && semeq(HTTP_REQUEST,to_sym))
        return ascii_chars_2_http_req;
    else {
        Structure src_s = _sem_get_symbol_structure(sem,src_sym);
        Structure to_s = _sem_get_symbol_structure(sem,to_sym);
        if (semeq(src_s,DATE) && semeq(to_s,CSTRING)) {
            return date2usshortdate;
        }
	if (semeq(HTTP_RESPONSE_STATUS,src_sym) && semeq(to_s,CSTRING)) {
	    return http_response_status_2_ascii_str;
	}
    }
    return NULL_PROCESS;
}

int _p_transcode(SemTable *sem, T* src,Symbol to_sym, Structure to_s,T **result) {
    debug(D_TRANSCODE,"transcoding a %s to a %s\n",_sem_get_name(sem,_t_symbol(src)),_sem_get_name(sem,to_sym));
    Symbol src_sym = _t_symbol(src);
    T *x;
    bool dofree = true;
    int err = noReductionErr;
    if (semeq(to_sym,src_sym)) {
        x = src;
        dofree = false;
    }
    else {
        Process p = _p_get_transcoder(sem,src_sym,to_sym);
        if (!semeq(p,NULL_PROCESS)) {
            // we found a defined process for trans coding between the symbols
            x = __t_newr(0,p,true);
            _t_add(x,src);
            err=redoReduction;
            dofree = false;
            //                    __t_newi(x,TEST_INT_SYMBOL,3333,true);
        }
        else {
            // built in transcodings for built in structures
            Structure src_s = _sem_get_symbol_structure(sem,src_sym);
            if (semeq(to_s,src_s)) {
                x = src;
                x->contents.symbol = to_sym;
                dofree = false;
            }
            else if (semeq(to_s,INTEGER)) {
                if (semeq(src_s,CSTRING)) {
                    x = __t_newi(0,to_sym,atoi(_t_surface(src)),true);
                }
                else return incompatibleTypeReductionErr;
            }
            else if (semeq(to_sym,ASCII_CHARS)) {
                if (semeq(src_s,CSTRING)) {
                    char *c = (char *)_t_surface(src);
                    int l = _t_size(src);
                    x = __t_newr(0,ASCII_CHARS,true);
                    while (--l) { // ignore the terminating null
                        __t_newc(x,ASCII_CHAR,*c,true);
                        c++;
                    }

                }
                else return incompatibleTypeReductionErr;
            }
            else if (semeq(to_s,CSTRING)) {
                if (semeq(src_s,INTEGER)) {
                    char buf[100];
                    sprintf(buf,"%d",*(int *)_t_surface(src));
                    x = __t_new_str(0,to_sym,buf,true);
                }
                else if (semeq(src_s,FLOAT)) {
                    char buf[100];
                    sprintf(buf,"%f",*(float *)_t_surface(src));
                    x = __t_new_str(0,to_sym,buf,true);
                }
                else if (semeq(src_s,CHAR)) {
                    char buf[2];
                    buf[0] = *(char *)_t_surface(src);
                    buf[1] = 0;
                    x = __t_new_str(0,to_sym,buf,true);
                }
                else {

                    // get the definition of the structure of the src symbol.
                    T *def = _sem_get_def(sem,src_s);
                    Symbol s_def = *(Symbol *)_t_surface(_t_child(def,2));

                    // if it's an optionality structure then we can recurse on transcode
                    // and dissolve the results into the parent
                    if (!semeq(s_def,NULL_SYMBOL)) {
                        if (_t_children(src) == 0) x = __t_new_str(0,to_sym,"",true);
                        else {
                            x = __t_newr(0,DISSOLVE,true);
                            T *xx = __t_newr(x,LINES,true);
                            T *k,*r;
                            int e;
                            while (k = _t_detach_by_idx(src,1)) {
                                e = _p_transcode(sem,k,to_sym,to_s,&r);
                                if (e && e != redoReduction) {
                                    _t_free(src);
                                    return e;
                                }
                                _t_add(xx,r);
                            }
                            err = redoReduction;
                        }
                    }
                    else x = __t_new_str(0,to_sym,_t2s(sem,src),true);
                }
            }
            else {
                debug(D_TRANSCODE,"trying to find structural match\n");
                // This a special case where if the src and dest strutctures
                // of the same form *X then we can transcode the elements
                // @todo generalize
                // get the definition of the structure of the src and dest
                T *sdef = _sem_get_def(sem,src_s);
                sdef = _t_child(sdef,2);
                debug(D_TRANSCODE,"source def: %s\n",t2s(sdef));
                Symbol sdef_sym = _t_symbol(sdef);

                T *tdef = _sem_get_def(sem,to_s);
                tdef = _t_child(tdef,2);
                debug(D_TRANSCODE,"to def: %s\n",t2s(tdef));
                Symbol tdef_sym = _t_symbol(tdef);

                T *k;
                Symbol k_sym;

		bool tsym_isa_list = (semeq(tdef_sym,STRUCTURE_ZERO_OR_MORE)||
				      semeq(tdef_sym,STRUCTURE_ZERO_OR_ONE)||
				      semeq(tdef_sym,STRUCTURE_ONE_OR_MORE));

                if (semeq(tdef_sym,sdef_sym) &&
                    tsym_isa_list &&
                    semeq(k_sym = _t_symbol(k = _t_child(tdef,1)),
                          _t_symbol( _t_child(sdef,1))) &&
                    semeq(k_sym,STRUCTURE_SYMBOL)) {
                    x = __t_newr(0,to_sym,true);
                    T *m,*r;
                    int e;
                    // the to becomes the surface of the STRUCTURE_SYMBOL def
                    Symbol to_sym = *(Symbol *)_t_surface(k);
                    Structure to_s = _sem_get_symbol_structure(sem,to_sym);
                    while (m = _t_detach_by_idx(src,1)) {
                        e = _p_transcode(sem,m,to_sym,to_s,&r);
                        if (e && e != redoReduction) {
                            _t_free(src);
                            return e;
                        }
                        _t_add(x,r);
                    }
                    err = redoReduction;
                }
		else if (tsym_isa_list &&
			 semeq(k_sym = _t_symbol(k = _t_child(tdef,1)),STRUCTURE_SYMBOL) &&
			 semeq(src_sym,*(Symbol *)_t_surface(k))) {
		    // special case of transcoding a singleton into a list of that same type
		    x = __t_newr(0,to_sym,true);
		    _t_add(x,src);
		    dofree=false;
		}
                else {
                    debug(D_TRANSCODE,"unable to match\n");
                    _t_free(src);
                    return incompatibleTypeReductionErr;
		}
            }
        }
    }
    *result = x;
    debug(D_TRANSCODE,"transcode result: %s\n",_t2s(sem,x));
    if (dofree) _t_free(src);
    return err;
}


/**
 * check a group of parameters to see if they match a process input signature
 *
 * @param[in] sem Semantic table in use
 * @param[in] p the Process we are checking against
 * @param[in] params list of parameters
 *
 * @returns Error code
 *
 * @todo add SIGNATURE_SYMBOL for setting up process signatures by Symbol not just Structure
 */
Error __p_check_signature(SemTable *sem,Process p,T *code,T *sem_map) {
    T *processes = _sem_get_defs(sem,p);
    T *def = _d_get_process_code(processes,p);
    T *signature = _t_child(def,ProcessDefSignatureIdx);
    // @todo if there's no signature we should probably fail, but instead we assume everything's ok
    // (sig should always have at least 1 child, the output sig)
    int sigs = _t_children(signature);
    if (!sigs) return 0;
    int input_sigs = 0;
    int i;

    for(i=SignatureOutputSigIdx+1;i<=sigs;i++) { // skip the output signature which is always first
        T *s = _t_child(signature,i);
        Symbol sym = _t_symbol(s);
        if (semeq(sym,INPUT_SIGNATURE)) {
            input_sigs++;
            T *param = _t_child(code,i-1);
            T *sig = _t_child(s,2); // input signatures start at 2
            bool is_optional = _t_child(s,InputSigOptionalIdx) != NULL;
            if (!param && !is_optional)
                //                raise_error("missing non-optional param");
                return tooFewParamsReductionErr;
            if (!param && is_optional) {
                // don't count as required sig
                input_sigs--;
            }
            if (param) {
                if(semeq(_t_symbol(sig),SIGNATURE_STRUCTURE)) {
                    Structure ss = *(Symbol *)_t_surface(sig);
                    if (!semeq(_sem_get_symbol_structure(sem,_t_symbol(param)),ss) && !semeq(ss,TREE))
                        return signatureMismatchReductionErr;
                }
                else if(semeq(_t_symbol(sig),SIGNATURE_SYMBOL)) {
                    Symbol ss = *(Symbol *)_t_surface(sig);
                    if (!semeq(ss,_t_symbol(param)))
                        raise_error("signatureMismatchReductionErr expected:%s got:%s\n",_sem_get_name(sem,ss),_t2s(sem,param));
                    //                    return signatureMismatchReductionErr;
                }
                else if(semeq(_t_symbol(sig),SIGNATURE_ANY)) {
                }
                else if(semeq(_t_symbol(sig),SIGNATURE_PROCESS)) {
                    Symbol expected = *(Symbol *)_t_surface(sig);
                    Symbol actual = _t_symbol(param);
                    if (!semeq(expected,actual)) {
                        raise_error("expecting process to reduce to %s, got: %s\n",_sem_get_name(sem,expected),_t2s(sem,param));
                    }
                }
                else {
                    raise_error("unknown signature checking symbol: %s",_sem_get_name(sem,_t_symbol(sig)));
                }
            }
        }
        else if (semeq(sym,TEMPLATE_SIGNATURE)) {
            if (!sem_map)
                return missingSemanticMapReductionErr;
            int c = _t_children(s);
            int map_children = _t_children(sem_map);
            if (map_children < c ) return mismatchSemanticMapReductionErr;

            // build up hashes of all the semantic references in our map
            // @todo cache this someplace so we don't need to do it every time
            TreeHash mapped[map_children];
            int j;
            for(j=1;j<=map_children;j++) {
                T *t = _t_child(_t_child(sem_map,j),SemanticMapSemanticRefIdx);
                mapped[j-1] = _t_hash(sem,t);
            }
            // now scan through the signature and see if all it's expected slots are actually mapped
            // @todo convert this to a true hash lookup algorithm
            for(j=1;j<=c;j++) {
                T *t = _t_child(_t_child(s,j),1);
                TreeHash h = _t_hash(sem,t);
                int k;
                for (k=0;k<map_children;k++) {
                    if (mapped[k] == h) {
                        break;
                    }
                }
                // not found so return a mismatch error
                if (k == map_children) return mismatchSemanticMapReductionErr;
            }
        }
    }
    int param_count = _t_children(code);
    if (param_count > input_sigs) return tooManyParamsReductionErr;
    if (param_count < input_sigs) return tooFewParamsReductionErr;

    return 0;
}

/* low level function to unwind a run-tree to a specific point*/
void __p_unwind_to_point(R *context,T *code_point,T *with) {
    T *p = _t_parent(code_point);
    _t_replace(p,_t_node_index(code_point), with);
    context->parent = p;
    context->node_pointer = with;
}

/**
 * reduce system level processes in a run tree.  Assumes that the children have already been
 * reduced and all parameters have been filled in
 *
 * these system level processes are the equivalent of the instruction set of the ceptr virtual machine
 */
Error __p_reduce_sys_proc(R *context,Symbol s,T *code,Q *q) {
    int b,c;
    char *str;
    Symbol sy;
    T *x,*t,*match_results,*match_tree;
    Error err = noReductionErr;
    SemTable *sem = q ? q->r->sem : G_sem;

    debug(D_REDUCE,"Reducing sys proc: %s\n",_sem_get_name(sem,s));
    debug(D_STEP,"Reducing %s\n",_t2s(sem,code));

    bool dissolve = false;
    switch(s.id) {
    case NOOP_ID:
        // noop simply replaces itself with it's own child
        /// @todo what happens if it has more than one child! validity check?
        x = _t_detach_by_idx(code,1);
        break;
    case GET_ID:
    case DEL_ID:
        {
            T *t = _t_detach_by_idx(code,1);
            Xaddr xa = *(Xaddr *)_t_surface(t);
            T *v = _r_get_instance(q->r,xa);
            if (!v) raise_error("Invalid xaddr in GET");
            x = _t_rclone(v);
            _t_free(t);
            if (s.id == DEL_ID) {
                _r_delete_instance(q->r,xa);
            }
        }
        break;
    case DEF_SYMBOL_ID:
        {
            T *def = _t_detach_by_idx(code,1);
            //@todo some kind of validation of the def??
            SemanticID ns = _d_define(sem,def, SEM_TYPE_SYMBOL,q->r->context);
            x = __t_news(0,RESULT_SYMBOL,ns,true);
        }
        break;
    case DEF_STRUCTURE_ID:
        {
            T *def = _t_detach_by_idx(code,1);
            //@todo some kind of validation of the def??
            SemanticID ns = _d_define(sem,def, SEM_TYPE_STRUCTURE,q->r->context);
            x = __t_news(0,RESULT_STRUCTURE,ns,true);
        }
        break;
    case DEF_PROCESS_ID:
        {
            T *def = _t_detach_by_idx(code,1);
            //@todo some kind of validation of the def??
            SemanticID ns = _d_define(sem,def, SEM_TYPE_PROCESS,q->r->context);
            x = __t_news(0,RESULT_PROCESS,ns,true);
        }
        break;
    case DEF_RECEPTOR_ID:
        {
            T *def = _t_detach_by_idx(code,1);
            //@todo some kind of validation of the def??
            SemanticID ns = __d_define_receptor(sem,def,q->r->context);
            x = __t_news(0,RESULT_RECEPTOR,ns,true);
        }
        break;
    case DEF_PROTOCOL_ID:
        {
            T *def = _t_detach_by_idx(code,1);
            //@todo some kind of validation of the def??
            SemanticID ns = _d_define(sem,def,SEM_TYPE_PROTOCOL,q->r->context);
            x = __t_news(0,RESULT_PROTOCOL,ns,true);
        }
        break;
    case NEW_ID:
        {
            T *t = _t_detach_by_idx(code,1);
            Symbol s = *(Symbol *)_t_surface(t);
            _t_free(t);
            t = _t_detach_by_idx(code,1);
            Structure struc_new = _sem_get_symbol_structure(sem,s);
            Structure struc_val = _sem_get_symbol_structure(sem,_t_symbol(t));
            if (!semeq(struc_new,struc_val)) {
                return structureMismatchReductionErr;
            }
            else {
                t->contents.symbol = s;
                Xaddr xa = _r_new_instance(q->r,t);
                x = __t_new(0,WHICH_XADDR,&xa,sizeof(Xaddr),1);
            }
        }
        break;
    case DO_ID:
    case CONVERSE_ID:
        {
            // all of the scope's children should have been reduced
            // so all we need to do is return the last one.
            T *scope = _t_detach_by_idx(code,1);
            int p = _t_children(scope);
            x = _t_detach_by_idx(scope,p);
            _t_free(scope);
            if (s.id == CONVERSE_ID) {
                //pop off the last conversation reference
                if (context->conversation)
                    context->conversation = context->conversation->next;
            }
            T *t;
            while (t = _t_detach_by_idx(code,1)) {
                if (semeq(_t_symbol(t),BOOLEAN)) {
                    if (*(int *)_t_surface(t)) {
                        err = Block;
                    }
                }
                _t_free(t);
            }
        }
        break;
    case IF_ID:
        t = _t_child(code,1);
        b = (*(int *)_t_surface(t)) ? 2 : 3;
        x = _t_detach_by_idx(code,b);
        break;
    case COND_ID:
        // COND is a special case, we have to check the phase to see what to do
        // after the children have been evaluated.
        {
            CondState *state = *(CondState **)_t_surface(code);
            // get the condition or else results into x
            x = _t_detach_by_idx(code,1);
            if (state->phase == EvalCondCondtions) {
                // if the condition was true, then we have to load the body for evaluation
                T *cond_pair = _t_detach_by_idx(state->conditions,1);
                if (*(int *)_t_surface(x)) {
                    _t_add(code,_t_detach_by_idx(cond_pair,1));
                    state->phase = EvalCondResult;
                } else  {
                    // otherwise we have to move on to the next condition or the Else
                    T *c = _t_child(state->conditions,1);
                    _t_add(code,_t_detach_by_idx(c,1));
                    if (semeq(_t_symbol(c),COND_ELSE)) {
                        state->phase = EvalCondResult;
                    }
                }
                _t_free(cond_pair);
                _t_free(x);
                return Eval;
            }
            else {
                // cleanup the state before returning.
                _t_free(state->conditions);
                free(state);
                code->contents.size = 0;
            }
        }
        break;
    case EQ_SYM_ID:
        x = __t_newi(0,BOOLEAN,
                     semeq(
                           *(Symbol *)_t_surface(_t_child(code,1)),
                           *(Symbol *)_t_surface(_t_child(code,2))),
                           true);
        break;
    case ADD_INT_ID:
        x = _t_detach_by_idx(code,1);
        c = *(int *)_t_surface(_t_child(code,1));
        *((int *)&x->contents.surface) = c+*((int *)&x->contents.surface);
        break;
    case SUB_INT_ID:
        x = _t_detach_by_idx(code,1);
        c = *(int *)_t_surface(_t_child(code,1));
        *((int *)&x->contents.surface) = *((int *)&x->contents.surface)-c;
        break;
    case MULT_INT_ID:
        x = _t_detach_by_idx(code,1);
        c = *(int *)_t_surface(_t_child(code,1));
        *((int *)&x->contents.surface) = *((int *)&x->contents.surface)*c;
        break;
    case DIV_INT_ID:
        x = _t_detach_by_idx(code,1);
        c = *(int *)_t_surface(_t_child(code,1));
        if (!c) {
            _t_free(x);
            return divideByZeroReductionErr;
        }
        *((int *)&x->contents.surface) = *((int *)&x->contents.surface)/c;
        break;
    case MOD_INT_ID:
        x = _t_detach_by_idx(code,1);
        c = *(int *)_t_surface(_t_child(code,1));
        if (!c) {
            _t_free(x);
            return divideByZeroReductionErr;
        }
        *((int *)&x->contents.surface) = *((int *)&x->contents.surface)%c;
        break;
    case EQ_INT_ID:
        x = _t_detach_by_idx(code,1);
        c = *(int *)_t_surface(_t_child(code,1));
        *((int *)&x->contents.surface) = *((int *)&x->contents.surface)==c;
        x->contents.symbol = BOOLEAN;
        break;
    case LT_INT_ID:
        x = _t_detach_by_idx(code,1);
        c = *(int *)_t_surface(_t_child(code,1));
        *((int *)&x->contents.surface) = *((int *)&x->contents.surface)<c;
        x->contents.symbol = BOOLEAN;
        break;
    case GT_INT_ID:
        x = _t_detach_by_idx(code,1);
        c = *(int *)_t_surface(_t_child(code,1));
        *((int *)&x->contents.surface) = *((int *)&x->contents.surface)>c;
        x->contents.symbol = BOOLEAN;
        break;
    case LTE_INT_ID:
        x = _t_detach_by_idx(code,1);
        c = *(int *)_t_surface(_t_child(code,1));
        *((int *)&x->contents.surface) = *((int *)&x->contents.surface)<=c;
        x->contents.symbol = BOOLEAN;
        break;
    case GTE_INT_ID:
        x = _t_detach_by_idx(code,1);
        c = *(int *)_t_surface(_t_child(code,1));
        *((int *)&x->contents.surface) = *((int *)&x->contents.surface)>=c;
        x->contents.symbol = BOOLEAN;
        break;
    case POP_PATH_ID:
        {
            x = _t_detach_by_idx(code,1);
            T *as = _t_child(code,1);
            T *count = _t_child(code,2);
            x->contents.symbol = *(Symbol *)_t_surface(as);
            int i = count ? *(int *)_t_surface(count) : 1;
            int *path = (int *)_t_surface(x);
            int d = _t_path_depth(path);
            if (i>d) path[0] = TREE_PATH_TERMINATOR;
            else path[d-i] = TREE_PATH_TERMINATOR;
        }
        break;
    case CONTRACT_STR_ID:
    case CONCAT_STR_ID:
        // if the first parameter is a RESULT SYMBOL then we use that as the symbol type for the result tree.
        x = _t_detach_by_idx(code,1);
        if (!x) return tooFewParamsReductionErr;
        sy = _t_symbol(x);
        if (semeq(RESULT_SYMBOL,sy)) {
            sy = *(Symbol *)_t_surface(x);
            _t_free(x);
            // confirm that the result structure is a CSTRING
            Structure struc = _sem_get_symbol_structure(sem,sy);
            if (!semeq(struc,CSTRING))
                return signatureMismatchReductionErr;
            x = _t_detach_by_idx(code,1);
        } else if (s.id == CONTRACT_STR_ID) {// CONTRACT requires first param to be RESULT_SYMBOL
            _t_free(x);
            return signatureMismatchReductionErr;
        }
        if (!x) {
            return tooFewParamsReductionErr;
        }
        c = _t_children(code);

        // make sure the surface was allocated and if not, converted to an alloced surface
        if (c > 0) {
            if (!(x->context.flags & TFLAG_ALLOCATED)) {
                str = malloc(x->contents.size);
                memcpy(str,&x->contents.surface,x->contents.size);
                x->contents.surface = str;
                x->context.flags = TFLAG_ALLOCATED+TFLAG_RUN_NODE;
            }
        }
        // check type the first node
        Structure struc = _sem_get_symbol_structure(sem,_t_symbol(x));
        if (semeq(struc,CHAR)) {
            x->contents.surface = realloc(x->contents.surface,++x->contents.size);
            ((char *)x->contents.surface)[1] = 0;
        }
        else if (!semeq(struc,CSTRING)) {
            _t_free(x);
            return incompatibleTypeReductionErr;
        }
        /// @todo this would probably be faster with just one total realloc for all children
        for(b=1;b<=c;b++) {
            T *t = _t_child(code,b);
            struc = _sem_get_symbol_structure(sem,_t_symbol(t));
            str = (char *)_t_surface(t);
            int size;
            if (semeq(struc,CSTRING)) size = strlen(str);
            else if (semeq(struc,CHAR)) size = 1;
            else {
                _t_free(x);
                return incompatibleTypeReductionErr;
            }
            x->contents.surface = realloc(x->contents.surface,x->contents.size+size);
            memcpy(x->contents.surface+x->contents.size-1,str,size);
            x->contents.size+=size;
            *( (char *)x->contents.surface + x->contents.size -1) = 0;
        }
        x->contents.symbol = sy;
        break;
    case EXPAND_STR_ID:
        {
            T *t = _t_detach_by_idx(code,1);
            x = makeASCIITree((char *)_t_surface(t));
            _t_free(t);
        }
        break;
    case RESPOND_ID:
        {
            T *signal = _t_parent(context->run_tree);
            if (!signal || !semeq(_t_symbol(signal),SIGNAL))
                return notInSignalContextReductionError;
            T *t = _t_detach_by_idx(code,1);
            if (!semeq(CARRIER,_t_symbol(t))) raise_error("expected CARRIER got %s",_t2s(sem,t));
            Symbol carrier = *(Symbol*)_t_surface(t);
            _t_free(t);
            T *response_contents = _t_detach_by_idx(code,1);
            T *head = _t_getv(signal,SignalMessageIdx,MessageHeadIdx,TREE_PATH_TERMINATOR);

            ReceptorAddress to = __r_get_addr(_t_child(head,HeadFromIdx)); // from and to reverse in response
            ReceptorAddress from = __r_get_addr(_t_child(head,HeadToIdx));
            Aspect a = *(Aspect *)_t_surface(_t_child(head,HeadAspectIdx));
            T *su = _t_getv(signal,SignalEnvelopeIdx,EnvelopeSignalUUIDIdx,TREE_PATH_TERMINATOR);
            UUIDt uuid = *(UUIDt *)_t_surface(su);

            T *response = __r_make_signal(from,to,a,carrier,response_contents,&uuid,0,context->conversation ? context->conversation->cid : NULL);
            x = _r_send(q->r,response);
        }
        break;
    case QUOTE_ID:
        /// @todo what happens if it has more than one child! validity check?
        // Note that QUOTE seems to be the same as NOOP?
        x = _t_detach_by_idx(code,1);
        break;
    case REQUEST_ID:
    case SAY_ID:
        {

            T *t = _t_detach_by_idx(code,1);
            ReceptorAddress to = __r_get_addr(t);
            _t_free(t);

            t = _t_detach_by_idx(code,1);
            if (!semeq(_t_symbol(t),ASPECT_IDENT)) {
                raise_error("expected ASPECT_IDENT!");
            }
            Aspect aspect = *(Aspect *)_t_surface(t);
            _t_free(t);

            t = _t_detach_by_idx(code,1);
            Symbol carrier = *(Symbol*)_t_surface(t);
            _t_free(t);

            T* signal_contents = _t_detach_by_idx(code,1);

            ReceptorAddress from = __r_get_self_address(q->r);
            T *signal;

            if (s.id == SAY_ID) {
                signal = __r_make_signal(from,to,aspect,carrier,signal_contents,0,0,context->conversation ? context->conversation->cid : NULL);
                x = _r_send(q->r,signal);
            }
            else if (s.id == REQUEST_ID) {
                T *response_point = NULL;

                t = _t_detach_by_idx(code,1);
                Symbol response_carrier = *(Symbol*)_t_surface(t);
                _t_free(t);

                int kids = _t_children(code);
                T *until = NULL;
                if (kids > 2) {
                    return(tooManyParamsReductionErr);
                }
                T *callback = NULL;
                while(kids--) {
                    t = _t_detach_by_idx(code,1);
                    if (semeq(_t_symbol(t),END_CONDITIONS)) {
                        until = t;
                    }
                    else callback = t;
                }
                if (!until) until = defaultRequestUntil();
                if (!callback) {
                    err = Block;
                    debug(D_SIGNALS,"blocking at %s\n",_td(q->r,code));
                    response_point = code;
                }
                else {
                    raise_error("request callback not implemented for %s",t2s(callback));
                }
                T *cid = context->conversation ? context->conversation->cid : NULL;
                signal = __r_make_signal(from,to,aspect,carrier,signal_contents,0,until,cid);

                x = _r_request(q->r,signal,response_carrier,response_point,context->id,cid);
            }
        }
        break;
    case THIS_SCOPE_ID:
        // @todo make this return an error that would invoke the error handler
        if (!context->conversation)
            raise_error("whoa THIS_SCOPE executed outside CONVERSE!");
        x = _t_rclone(context->conversation->cid);
        break;
    case CONTINUE_ID:
        {
            T *at = _t_detach_by_idx(code,1);
            T *with = _t_detach_by_idx(_t_child(code,1),1);
            int *path = (int *)_t_surface(at);
            // @todo validate that the path is on ok path to unwind to...
            T *c = _t_get(context->run_tree,path);
            if (!c) raise_error("continue at point invalid");
            __p_unwind_to_point(context,c,with);
            return(noErr);
        }
        break;
    case COMPLETE_ID:
        {
            T *with = _t_detach_by_idx(code,1);
            T *cid = _t_detach_by_idx(code,1);

            // @todo think about if this would cause other arbitrary weirdness, like what about any pending wakeups (from requests) that point to spots in the code that's getting unwound.
            if (!cid) {
                // in the case no conversation id parameter then we need to look in
                // we get it from the context
                // @todo make this return an error that would invoke the error handler
                if (!context->conversation)
                    raise_error("COMPLETE invoked without conversation id outside of CONVERSE");
                cid = context->conversation->cid;
                UUIDt *cuuid = __cid_getUUID(cid);
                T *w = __r_cleanup_conversation(q->r,cuuid);
                if (w) _t_free(w);

                // now move the execution point up to the CONVERSE root.
                // @todo maybe only use the wakeup_ref (like below) and don't store converse_pointer?
                T *c = context->conversation->converse_pointer;
                __p_unwind_to_point(context,c,with);
                return(noErr);
            }
            else {
                // if the conversation param was specified we need to get it from

                UUIDt *cuuid = __cid_getUUID(cid);
                T *w = __r_cleanup_conversation(q->r,cuuid);
                // restart the CONVERSE instruction that spawned this conversation
                if (w) {
                    int *code_path = (int *)_t_surface(_t_child(w,WakeupReferenceCodePathIdx));
                    int process_id = *(int *)_t_surface(_t_child(w,WakeupReferenceProcessIdentIdx));
                    debug(D_LOCK,"complete LOCK\n");
                    pthread_mutex_lock(&q->mutex);
                    Qe *e = __p_find_context(q->blocked,process_id);
                    if (e) {
                        if (with) {
                            T *c = _t_get(e->context->run_tree,code_path);
                            if (!c) raise_error("failed to find code path when completing converse!");
                            __p_unwind_to_point(e->context,c,with);
                        }
                        debug(D_SIGNALS,"unblocking CONVERSE\n");
                        __p_unblock(q,e,noReductionErr);
                    }
                    else if (with) _t_free(with);
                    pthread_mutex_unlock(&q->mutex);
                    debug(D_LOCK,"complete UNLOCK\n");
                }
                x = cid;
            }
        }
        break;
    case TRANSCODE_ID:
        {
            T *params = _t_detach_by_idx(code,1);
            if (!params) return signatureMismatchReductionErr;
            T *to = _t_detach_by_idx(params,1);
            if (!to) return signatureMismatchReductionErr;
            _t_free(params);

            Symbol to_sym = *(Symbol *)_t_surface(to);
            _t_free(to);
            Structure to_s = _sem_get_symbol_structure(sem,to_sym);

            T *items = _t_child(code,1);
            if (!items) return signatureMismatchReductionErr;
            T *t = __t_newr(0,PARAMS,true);  //holder for the transcoding children
            T *src;
            while (src = _t_detach_by_idx(items,1)) {
                int e = _p_transcode(sem,src,to_sym,to_s,&x);
                if (e != noReductionErr) {
                    if (e != redoReduction) return e;
                    else err = e;
                }
                _t_add(t,x);
            }
            if (_t_children(t) == 1) {
                x = _t_detach_by_idx(t,1);
                _t_free(t);
            }
            else {
                x = __t_newr(0,DISSOLVE,true);
                _t_add(x,t);
                err = redoReduction;
            }
        }
        break;
    case DISSOLVE_ID:
        // dissolve can't be the root process!
        if (!_t_parent(code)) return structureMismatchReductionErr;

        // if the param has children, then they are to be inserted into the
        // parent's children at this instruction's spot.
        x = _t_detach_by_idx(code,1);
        dissolve = _t_children(x) > 0;
        break;
    case MATCH_ID:
        {
            T *pattern = _t_detach_by_idx(code,1);
            T *t = _t_detach_by_idx(code,1);
            bool matchr = false;
            if (_t_children(code)) {
                T *t = _t_detach_by_idx(code,1);
                matchr = *(int*)_t_surface(t);
                _t_free(t);
            }
            T *results;
            bool match;
            if (matchr) {
                match =_t_matchr(pattern,t,&results);
                if (match) {
                    x = _t_rclone(results);
                    _t_free(results);
                }

                else x = __t_newi(0,BOOLEAN,0,true);
            }
            else {
                match = _t_match(pattern,t);
                x = __t_newi(0,BOOLEAN,match,true);
            }
            _t_free(pattern);
            _t_free(t);
        }
        break;
    case FILL_ID:
        {
            x = _t_detach_by_idx(code,1);
            T *sem_map = _t_detach_by_idx(code,1);
            _t_fill_template(x,sem_map);
            _t_free(sem_map);
        }
        break;
    case FILL_FROM_MATCH_ID:
        match_results = _t_child(code,2);
        match_tree = _t_child(code,3);
        x = _t_detach_by_idx(code,1);
        /// @todo interpolation errors?
        _p_fill_from_match(sem,x,match_results,match_tree);
        break;
    case RAISE_ID:
        return raiseReductionErr;
        break;
    case STREAM_READ_ID:
        {
            // get the stream param
            T *s = _t_child(code,1);
            Stream *st = _t_surface(s);

            //@todo possible another parameter to specify if we should read lines, or specific number of bytes
            st->callback = 0;
            if (st->flags & StreamHasData) {
                _t_detach_by_idx(code,1);
                _t_free(s);
                if (st->err) return(st->err);
                // get the result type to use as the symbol type for the ascii data
                s = _t_detach_by_idx(code,1);
                sy = _t_symbol(s);
                if (semeq(RESULT_SYMBOL,sy)) {
                    sy = *(Symbol *)_t_surface(s);
                    _t_free(s);
		    size_t l = _st_data_size(st);
		    char *c = _st_data(st);

		    Structure to_s = _sem_get_symbol_structure(sem,sy);
		    if (semeq(to_s,CSTRING)) {
			debug(D_STREAM,"creating CSTRING: %s '%.*s'\n",_sem_get_name(sem,sy),(int)l,c);
			// @todo fix this to be a flag instruction to __t_new
			// currently it only works because that value is the newline in the
			// read buffer.
			_st_data(st)[l] = 0;
			x = __t_new(0,sy,c,l+1,1);
		    }
		    else {
			debug(D_STREAM,"non CSTRING RESULT_SYMBOL so converting to ASCII_CHARS and transcoding to %s \n",_sem_get_name(sem,sy));
                        T *src = __t_newr(0,ASCII_CHARS,true);
                        while (l--) {
                            __t_newc(src,ASCII_CHAR,*c,true);
                            c++;
                        }
			int e = _p_transcode(sem,src,sy,to_s,&x);
			if (e != noReductionErr) {
			    if (e != redoReduction) return e;
			    else err = e;
			}
		    }
                    _st_data_consumed(st);
                }
                else {raise_error("expecting RESULT_SYMBOL");}
            }
            else if (_st_is_alive(st)) {
                st->callback = processUnblocker;
                st->callback_arg1 = q;
                st->callback_arg2 = q->active->id;

                // start up the thread to read the data,
                _st_start_read(st);

                // and block this context
                return(Block);
            }
            else {
                return deadStreamReadReductionErr;
            }
        }
        break;
    case STREAM_WRITE_ID:
        {
            // get the stream param
            T *s = _t_detach_by_idx(code,1);
            Stream *st = _t_surface(s);
            _t_free(s);
            // get the data to write as string
            while(s = _t_detach_by_idx(code,1)) {
                int err = _t_write(sem,s,st);
                _t_free(s);
                if (err == 0) return unixErrnoReductionErr;
            }
            /// @todo what should this really return?
            x = __t_news(0,REDUCTION_ERROR_SYMBOL,NULL_SYMBOL,1);
        }
        break;
    case STREAM_ALIVE_ID:
        {
            // get the stream param
            T *s = _t_detach_by_idx(code,1);
            Stream *st = _t_surface(s);
            x = __t_newi(0,BOOLEAN,_st_is_alive(st),true);
            _t_free(s);
        }
        break;
    case STREAM_CLOSE_ID:
        {
            // get the stream param
            T *s = _t_detach_by_idx(code,1);
            Stream *st = _t_surface(s);
            _st_kill(st);
            void *status;
            int rc;

            rc = pthread_join(st->pthread, &status);
            if (rc) {
                raise_error("ERROR; return code from pthread_join() is %d\n", rc);
            }
            _st_free(st);
            x = __t_newi(0,BOOLEAN,1,true);
            _t_free(s);
        }
        break;
    case ITERATE_ID:
        // iterate is a special case, we have to check the phase to see what to do
        // after the children have been evaluated.
        {
            IterationState *state = *(IterationState **)_t_surface(code);
            bool done = false;
            int next_phase;
            // get the condition or body results into x
            x = _t_detach_by_idx(code,2);
            switch(state->phase) {
            case EvalCondition: {
                // if this is the first time evaluating the cond, figure out what type
                // of iteration we are doing based on the semantics
                if (state->type == IterateTypeUnknown) {
                    Symbol c = _t_symbol(x);
                    if (semeq(c,BOOLEAN)) {
                        state->type = IterateTypeCond;
                        state->count = 0;
                    }
                    else if (semeq(c,ITERATE_ON_SYMBOL)) {
                        state->type = IterateTypeOnSymbol;
                        T *params = _t_child(code,1);
                        T *list = _t_newr(params,ITERATION_DATA);
                        _a_get_instances(&q->r->instances,*(Symbol *)_t_surface(x),list);
                        // if the list has no children the we are already done
                        done = !_t_children(list);
                    }
                    else {
                        Structure s = _sem_get_symbol_structure(sem,c);
                        if (semeq(s,INTEGER)) {
                            state->type = IterateTypeCount;
                            state->count = *(int *)_t_surface(x);
                        }
                        else {
                            raise_error("unable to determine iteration type! symbol was:%s",_sem_get_name(sem,s));
                        }
                    }
                }
                // evaluate condition based on iteration type
                switch(state->type) {
                case IterateTypeCond:
                    done = !*(int *)_t_surface(x);
                    break;
                case IterateTypeCount:
                    done = (--state->count < 0);
                    break;
                // IterateTypeOnSymbol handled above
                }
                next_phase = EvalBody;
                break;
            }
            case EvalBody:
                // if this is cond iteration we have to alternate between phases because
                // the cond has to be re-cloned and evaluated.  For other iteration types
                // the conditionality not internal so we evaluate it directly
                // @todo we should refactor this code because the done checking also appear
                // above in the first IterateTypeUnknown case.
                next_phase = (state->type == IterateTypeCond) ? EvalCondition :  EvalBody;
                if (state->type == IterateTypeCount){
                    if (--state->count < 0) done = true;
                }
                else {
                    // we aren't doing count iteration, use the count var just
                    // to keep track of how many times we've gone through the loop
                    state->count++;
                    if (state->type == IterateTypeOnSymbol) {
                        T *params = _t_child(code,1);
                        int p = _t_children(params);
                        T *list = _t_child(params,p);  // iterate list should be last child
                        T *t = _t_detach_by_idx(list,1);
                        _t_free(t);
                        if (!_t_children(list)) done = true;
                    }
                    //if (state->count > 9) done = true;  // temporary infinite loop breaker
                }
            }
            if (done) {
                // we are done so free up the iteration state info
                /// @todo the value returned from the iteration will be what??(what's in x)
                _t_free(state->code);
                free(state);
                code->contents.size = 0;
            }
            else {
                _t_free(x);
                // add a copy of the body/condition on as the last child
                _t_add(code,_t_rclone(_t_child(state->code,next_phase == EvalBody ? 3 : 2)));
                // and reset the current child count so it gets evaluated.
                set_rt_cur_child(q->r,code,1); // reset the current child count on the code
                state->phase = next_phase;
                return Eval;
            }

        }

        break;
    case LISTEN_ID:
        {
            t = _t_detach_by_idx(code,1);
            if (!t || !semeq(_t_symbol(t),ASPECT_IDENT)) {
                raise_error("expected ASPECT_IDENT!");
            }
            Aspect aspect = *(Aspect *)_t_surface(t);
            _t_free(t);

            T *on = _t_detach_by_idx(code,1);
            Symbol carrier = *(Symbol *)_t_surface(on);
            _t_free(on);
            T *match = _t_detach_by_idx(code,1);
            T *with = NULL;
            T *until = NULL;
            T *act = NULL;
            int kids = _t_children(code);
            if (kids > 3) {
                return(tooManyParamsReductionErr);
            }
            while(kids--) {
                T *t = _t_detach_by_idx(code,1);
                Symbol sym = _t_symbol(t);
                if (semeq(sym,END_CONDITIONS)) {
                    until = t;
                }
                else if (semeq(sym,ACTION)) {
                    act = t;
                }
                else if (semeq(sym,PARAMS)) {
                    with = t;
                }
            }
            if (!with) {
                with = _t_new_root(PARAMS);
                T *s = _t_newr(with,SLOT);
                _t_news(s,USAGE,NULL_SYMBOL);
            }

            T *cid = context && context->conversation ? _t_clone(context->conversation->cid) : NULL;
            // @todo add SEMANTIC_MAP into LISTEN
            if (act) {
                _r_add_expectation(q->r,aspect,carrier,match,act,with,until,NULL,cid);
                x = __t_news(0,REDUCTION_ERROR_SYMBOL,NULL_SYMBOL,1);
                debug(D_LISTEN,"adding expectation\n");
            }
            else {
                act = __p_build_wakeup_info(code,context->id);
                if (!until) {
                    until = _t_new_root(END_CONDITIONS);
                    _t_newi(until,COUNT,1);
                }
                _r_add_expectation(q->r,aspect,carrier,match,act,with,until,NULL,cid);
                debug(D_LISTEN,"adding expectation and blocking at %d,%s\n",context->id,_td(q->r,code));
                return Block;
            }
        }
        break;
    case INITIATE_PROTOCOL_ID:
        {
            T *protocol = _t_detach_by_idx(code,1);
            T *interaction = _t_detach_by_idx(code,1);
            T *bindings = _t_detach_by_idx(code,1);
	    T *sem_map;
	    x = __o_initiate(q->r,*(SemanticID *)_t_surface(protocol),*(SemanticID *)_t_surface(interaction),bindings,&sem_map);
            _t_free(protocol);
            _t_free(interaction);
	    err = redoReduction;
        }
        break;
    case SELF_ADDR_ID:
        {
            x = _t_detach_by_idx(code,1);
            Symbol sym = *(Symbol *)_t_surface(x);
            _t_free(x);
            ReceptorAddress addr =  __r_get_self_address(q->r);
            x = ___r_make_addr(0,sym,addr,true);
        }
        break;
    case GET_LABEL_ID:
        {
            x = _t_detach_by_idx(code,1);
            Symbol sym = *(Symbol *)_t_surface(x);
            _t_free(x);
            x = _t_detach_by_idx(code,1);
            Symbol type = *(Symbol *)_t_surface(x);
            _t_free(x);
            x = _t_detach_by_idx(code,1);
            Symbol as;
            if (x) {
                as = *(Symbol *)_t_surface(x);
                _t_free(x);
            }
            else as = type;
            T *l = _sem_get_label(sem,sym,type);
            if (!l) raise_error("label not found for symbol");
            x = __t_new_str(0,as,_t_surface(l),true);
        }
        break;
    case MAGIC_ID:
        {switch(*(int *)_t_surface(code)) {
            case MagicReceptors:
                if (G_vm) {
                    char *s = malloc(10000);
                    int i;
                    int l = 0;
                    for (i=0;i<G_vm->receptor_count;i++) {
                        Receptor *r = G_vm->routing_table[i].r;
                        if (r) {
                            char *n = _sem_get_name(r->sem,G_vm->routing_table[i].s);
                            if (!n) n= "??";
                            int nl = strlen(n);
                            memcpy(&s[l],n,nl);
                            l+= nl;
                            sprintf(&s[l],":%d ",r->addr.addr);
                            l += strlen(&s[l]);
                        }
                    }
                    s[l]=0;
                    x = __t_new_str(0,LINE,s,1);
                    free(s);
                }
                break;
            case MagicDebug:
                if (!debugging(D_SIGNALS)) {
                    debug_enable(D_SIGNALS);
                    x = __t_new_str(0,LINE,"debugging enabled",1);
                }
                else {
                    debug_disable(D_SIGNALS);
                    x = __t_new_str(0,LINE,"debugging disabled",1);
                }
                break;
            case MagicQuit:
                if (G_vm) {
                    __r_kill(G_vm->r);
                }
            default:
                x = __t_new_str(0,TEST_STR_SYMBOL,"blorp!",1);
            }
        }
        break;
    default:
        raise_error("unknown sys-process id: %d",s.id);
    }

    if (debugging(D_STEP)) {
        if (err==redoReduction) debug(D_STEP,"   redoing reduction\n");
    }

    if (!dissolve) {
        // in the normal case we just replace code with the value of x, so:
        // any remaining children of 'code' are the parameters which have all now been "used up"
        // so we can call the low-level __t_free the clean them up and then replace the contents of
        // the 'code' node with the contents of the 'x' node that was either detached or produced
        // by the the process that just ran
        __t_free(code);
        code->structure.child_count = x->structure.child_count;
        code->structure.children = x->structure.children;
        code->contents = x->contents;
        code->context = x->context;
        // we do have to fixe the parent value of all the children
        DO_KIDS(code,_t_child(code,i)->structure.parent = code);
        free(x);
        debug(D_STEP,"  to  %s\n",_t2s(sem,code));
    }
    else {
        // in the dissolve case we have to insert x's children into the spot where code was, so:
        // first build a path
        int path[2] = {0,TREE_PATH_TERMINATOR};
        path[0] = _t_node_index(code);
        T *parent = _t_parent(code);
        // then free the code node
        _t_detach_by_ptr(parent,code);
        _t_free(code);
        // then loop through x's children inserting them in the parent
        T *c;
        while (c = _t_detach_by_idx(x,1)) {
            _t_insert_at(parent,path,c);
            path[0]++;
        }
        _t_free(x);  // and free the decapitated root!
        debug(D_STEP,"  dissolving to  %s\n",_t2s(sem,parent));

        //@todo, I think this might cause those children to be evaluated twice which may be a mistake...
    }
    return err;
}

/**
 * create a run-tree execution context.
 */
R *__p_make_context(T *run_tree,R *caller,int process_id,T *sem_map) {
    R *context = malloc(sizeof(R));
    context->id = process_id;
    context->state = Eval;
    context->err = 0;
    context->run_tree = run_tree;
    // start with the node_pointer at the first child of the run_tree
    context->node_pointer = _t_child(run_tree,1);
    context->parent = run_tree;
    context->idx = 1;
    context->caller = caller;
    context->sem_map = sem_map;
    // copy in the callers conversation context too.
    context->conversation = caller ? caller->conversation : NULL;
    if (caller) caller->callee = context;
    return context;
}

#ifdef CEPTR_DEBUG
void pq(Qe *qe) {
    while(qe) {
        printf("%p(<-%p)  -> ",qe,qe->prev);
        qe = qe->next;
    }
    printf("NULL\n");
}
#endif

// unlink the queue element from the list rejoining the
// previous with the next
#define __p_dequeue(list,qe)                    \
    if (qe->next) {qe->next->prev = qe->prev;}                          \
    if (!qe->prev) {                            \
        list = qe->next;                        \
    }                                           \
    else {                                      \
        qe->prev->next = qe->next;              \
    }


// add the queue element onto the head of the list
#define __p_enqueue(list,qe) {                  \
        Qe *d = list;                           \
        qe->next = d;                           \
        if (d) d->prev = qe;                    \
        list = qe;                              \
    }
// add the queue element onto the tail of the list
#define __p_append(list,qe) {                   \
        qe->next = NULL;                        \
        if (!list) {list=qe;}                   \
        else {                                  \
            Qe *d = list;                       \
            while(d->next) {                    \
                d = d->next;                    \
            }                                   \
            qe->prev = d;                       \
            d->next = qe;                       \
        }                                       \
    }

void _p_enqueue(Qe **listP,Qe *e) {
    __p_enqueue(*listP,e);
}

Qe *__p_find_context(Qe *e,int process_id) {
    while (e && e->id != process_id) e = e->next;
    return e;
}

// low level unblock. Should be called only when q mutex is locked
void __p_unblock(Q *q,Qe *e,Error err) {
    __p_dequeue(q->blocked,e);
    __p_enqueue(q->active,e);
    q->contexts_count++;
    e->context->state = err ? err : Eval;
}

/**
 * search for the context in the q and unblock it
 */
Error _p_unblock(Q *q,int id) {
    // find the context in the queue
    int err = 0;
    debug(D_LOCK,"unblock LOCK\n");
    pthread_mutex_lock(&q->mutex);
    Qe *e = __p_find_context(q->blocked,id);
    if (e) {
        __p_unblock(q,e,noReductionErr);
    }
    else {
        // if the process has been completed then return err 2 otherwise 1
        e = __p_find_context(q->completed,id);
        if (e) err = 2;
        else {
            e = __p_find_context(q->active,id);
            err = e ? 1 : 3;
        }
    }
    pthread_mutex_unlock(&q->mutex);
    debug(D_LOCK,"unblock UNLOCK\n");
    return err;
}


/**
 * wakeup a process that's been paused
 *
 * @param[in] q the processing q in which to search for the process
 * @param[in] wakeup the WAKEUP_REF tree that identifies what needs waking up
 * @param[in] with a value to replace the current node_pointer with (i.e. a result)
 * @param[in] err an error value if the wakeup should trigger the error handler instead of resuming processing
 */
void _p_wakeup(Q *q,T *wakeup, T *with,Error err) {
    int process_id = *(int *)_t_surface(_t_child(wakeup,WakeupReferenceProcessIdentIdx));
    //int *code_path = (int *)_t_surface(_t_child(wakeup,WakeupReferenceCodePathIdx));

    debug(D_LOCK,"wakeup LOCK\n");
    pthread_mutex_lock(&q->mutex);
    Qe *e = __p_find_context(q->blocked,process_id);
    if (e) {
        // code_path is something I thought I needed to restart execution at the right place
        // I currently think that was a mistake, because a blocked process should really only
        // be blocked at ONE place, wherever the node_pointer is.
        /* T *t = _t_get(e->context->run_tree,code_path); */
        /* if (!t) raise_error("failed to find code path when waking up expectation!"); */
        /* if (t != e->context->node_pointer) */
        /*     raise_error("wakeup ref and node_pointer don't match"); */
        if (with) {
            if (!(with->context.flags & TFLAG_RUN_NODE)) {
                T *w = _t_rclone(with);
                _t_free(with);
                with = w;
            }
            T *t = e->context->node_pointer;
            T *p = _t_parent(t);
            _t_replace(p,_t_node_index(t), with);
            e->context->node_pointer = with;
        }
        __p_unblock(q,e,err);
    }
    else { if (with) _t_free(with);}
    pthread_mutex_unlock(&q->mutex);
    debug(D_LOCK,"wakeup UNLOCK\n");
}

/**
 * reduce a run tree by executing the instructions in it and replacing the tree values in place
 *
 * a run_tree is expected to have a code tree as the first child, parameters as the second,
 * and optionally an error handling routine as the third child.  This is a simplified
 * reducer for testing purposes only, as real reduction happens in the context of a processing Q.
 * This function makes a fake processing Q and Receptor.
 *
 * @param[in] processes context of defined processes
 * @param[in] run_tree the run tree being reduced
 * @returns Error status of the reduction
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/process_spec.h testProcessErrorTrickleUp
 */
Error _p_reduce(SemTable *sem,T *rt) {
    T *run_tree = rt;
    R *context = __p_make_context(run_tree,0,0,NULL);
    Error e;

    // build a fake Receptor and Q on the stack so _p_step will work
    Receptor r;
    Q q;
    r.root = NULL;
    r.sem = sem;
    r.q = &q;
    q.r = &r;

    while(_p_step(&q, &context) != Done);
    e = context->err;
    free(context);
    return e;
}

T * __p_buildErr(R *context) {
    Symbol se;
    T *extra = NULL;
    //@todo: fix this so we don't actually use an error value that
    // then has to be translated into a symbol, but rather so that we
    // can programatically calculate the symbol.
    // or, perhaps, the Error type should actually be the REDUCTION_ERROR
    // symbol that we generate when we want to return an error during the
    // reduction process (rather than here).  That way we actually have the
    // symbol whether or not a we have an error handler for context.

    switch(context->state) {
    case tooFewParamsReductionErr: se=TOO_FEW_PARAMS_ERR;break;
    case tooManyParamsReductionErr: se=TOO_MANY_PARAMS_ERR;break;
    case signatureMismatchReductionErr: se=SIGNATURE_MISMATCH_ERR;break;
    case notProcessReductionError: se=NOT_A_PROCESS_ERR;break;
    case notInSignalContextReductionError: se=NOT_IN_SIGNAL_CONTEXT_ERR;
    case divideByZeroReductionErr: se=ZERO_DIVIDE_ERR;break;
    case incompatibleTypeReductionErr: se=INCOMPATIBLE_TYPE_ERR;break;
    case deadStreamReadReductionErr: se=DEAD_STREAM_READ_ERR;break;
    case missingSemanticMapReductionErr: se=MISSING_SEMANTIC_MAP_ERR;break;
    case mismatchSemanticMapReductionErr: se=MISMATCH_SEMANTIC_MAP_ERR;break;
    case structureMismatchReductionErr: se=STRUCTURE_MISMATCH_ERR;break;
        //    case conversatonCompletedReductionErr: se=CONVERSATION_COMPLETED_ERR;break;
    case unixErrnoReductionErr:
        se=UNIX_ERRNO_ERR;
        extra = _t_new_str(0,TEST_STR_SYMBOL,strerror(errno));
        break;
    case raiseReductionErr:
        se = *(Symbol *)_t_surface(_t_child(context->node_pointer,1));
        break;
    default: raise_error("unknown reduction error: %d",context->err);
    }
    T *err = __t_newr(0,se,true);
    int *path = _t_get_path(context->node_pointer);
   _t_new(err,ERROR_LOCATION,path,sizeof(int)*(_t_path_depth(path)+1));
    free(path);
    if (extra) {
        _t_add(err,extra);
    }
    return err;
}

/**
 * take one step in the execution state machine given a run-tree context
 *
 * a run_tree is expected to have a code tree as the first child, parameters as the second,
 * and optionally an error handling routine as the third child.
 *
 * @param[in] Processing Q in which this step is taking place
 * @param[in] pointer to context pointer
 * @returns the next state that will be called for the context
 */
Error _p_step(Q *q, R **contextP) {
    R *context = *contextP;
    SemTable *sem = q->r->sem;

    switch(context->state) {
    case noReductionErr:
    case Block:
        raise_error("whoa, virtual states can't be executed!"); // shouldn't be calling step if Done or noErr or Block
        break;
    case Pop:
        // if this was the successful reduction by an error handler
        // move the value to the 1st child
        if (context->err) {
            T *t = _t_detach_by_idx(context->run_tree,RunTreeErrorCodeIdx);
            if (t) {
                _t_replace(context->run_tree,1,t);

                context->err = noReductionErr;
            }
        }

        // if this is top caller on the stack then we are completely done
        if (!context->caller) {
            if (debugging(D_REDUCE) && (context->err)) {
                //                T *err =  __p_buildErr(context);
                debug(D_REDUCE,"finishing reduction with unhandled error: %d\n",context->err);
                //                _t_free(err);
            }
            context->state = Done;
            break;
        }
        else {
            // otherwise pop the context
            R *ctx = context;
            context = context->caller;  // set the new context

            if (!ctx->err) {
                // get results of the run_tree
                T *np = _t_detach_by_idx(ctx->run_tree,1);
                _t_replace(context->parent,context->idx,np); // replace the process call node with the result
                set_rt_cur_child(q->r,np,RUN_TREE_EVALUATED);
                context->node_pointer = np;
                context->state = Eval;  // or possible ascend??
            }
            else context->state = ctx->err;
            // cleanup
            _t_free(ctx->run_tree);
            free(ctx);
            context->callee = 0;
            *contextP = context;
        }

        break;
    case Eval:
        {
            T *np = context->node_pointer;
            if (!np) {
                raise_error("Whoa! Null node pointer");
            }
            Process s = _t_symbol(np);

            if (semeq(s,PARAMETER)) {
                T *ref = _t_child(_t_child(np,ParameterReferenceIdx),1);
                Symbol ref_sym = _t_symbol(ref);
                T *param;
                if (semeq(ref_sym,PARAM_PATH)) {
                    param = _t_get(context->run_tree,(int *)_t_surface(ref));
                }
                else if (semeq(ref_sym,PARAM_LABEL)) {
                    raise_error("reference by PARAM_LABEL not implemented");
                }
                else raise_error("unknown reference type %s",_sem_get_name(sem,ref_sym));

                T *result = _t_child(_t_child(np,ParameterResultIdx),1);
                Symbol result_sym = _t_symbol(result);
                if (semeq(result_sym,RESULT_VALUE)) {
                    np = _t_rclone(param);
                }
                else if (semeq(result_sym,RESULT_SYMBOL)) {
                    np = __t_news(0,*(Symbol *)_t_surface(result),_t_symbol(param),true);
                }
                else if (semeq(result_sym,RESULT_LABEL)) {
                    Symbol label_sym = *(Symbol *)_t_surface(result);
                    Symbol param_sym = _t_symbol(param);
                    T *l = _sem_get_label(sem,param_sym,label_sym);
                    if (!l) raise_error("label not found for param symbol");
                    np = _t_rclone(l);
                }
                else raise_error("unknown result type %s",_sem_get_name(sem,result_sym));

                context->node_pointer = np;
                _t_replace(context->parent, context->idx,np);
                s = _t_symbol(np);
            }
            else if (semeq(s,PARAM_REF)) {
                int *path = (int *)_t_surface(np);
                T *param = _t_get(context->run_tree,path);
                if (!param) {
                    char buf[255];
                    _t_sprint_path(path,buf);
                    raise_error("request for non-existent param %s in %s",buf,_t2s(sem,context->run_tree));
                }
                context->node_pointer = np = _t_rclone(param);
                _t_replace(context->parent, context->idx,np);
                s = _t_symbol(np);
            }
            else if (semeq(s,SIGNAL_REF)) {
                T *sig = _t_parent(context->run_tree);
                if (!sig) {
                    raise_error("not in signal context!");
                    //                    return context->state = context->err = notInSignalContextReductionError;
                }
                T *param = _t_get(sig,(int *)_t_surface(np));
                if (!param) {
                    char buf[1000];

                    raise_error("request for non-existent signal portion.  signal was: %s \n path was:%s\n",t2s(sig),_t_sprint_path((int *)_t_surface(np),buf));
                }
                context->node_pointer = np = _t_rclone(param);
                _t_replace(context->parent, context->idx,np);
                s = _t_symbol(np);
            }
            /// @todo what if the replaced parameter is itself a PARAM_REF tree ??

            int count = _t_children(np);
            if (!is_process(s)) {

                // if this node is not a process, i.e. it's data, then either we
                // are done descending and the current items will be the result so ascend
                // or if we are doing deep param_ref searching, then search the entire tree
                // @todo increase efficiency by adding some instruction to allow the coder choose, see #39
#ifndef RUN_TREE_SHALLOW_PARAM_REF_SEARCH
                int node_cur_child = get_rt_cur_child(q->r,np);
                if ((node_cur_child != RUN_TREE_EVALUATED) && count && (count != node_cur_child))
                    context->state = Descend;
                else
#endif
                    context->state = Ascend;
            } else {
                if (semeq(s,ITERATE)) {
                    // if first time we are hitting this iteration
                    // then we need to set up the state data to track the iteration
                    if (_t_size(np) == 0) {
                        // sanity check
                        if (_t_children(np) != 3) {raise_error("ITERATE must have 3 params");}
                        // create a copy of the code and stick it in the iteration state struct
                        IterationState *state = malloc(sizeof(IterationState));
                        state->phase = EvalCondition;
                        state->code = _t_rclone(np);
                        state->type = IterateTypeUnknown;
                        *((IterationState **)&np->contents.surface) = state;
                        np->contents.size = sizeof(IterationState *);

                        // we start in condition phase so throw away the code copy
                        T *x = _t_detach_by_idx(np,3);
                        _t_free(x);
                    }
                }
                else if (semeq(s,COND)) {
                    // if first time we are hitting the cond
                    // the we need to set up the state data to track flow control
                    if (_t_size(np) == 0) {
                        CondState *state = malloc(sizeof(CondState));
                        // remove the conditions and store them in state
                        T *c = state->conditions = _t_detach_by_idx(np,1);
                        c = _t_child(c,1);
                        // we add the first child of the COND_PAIR or the COND_ELSE
                        // to the code for reduction and set the phase appropriately
                        _t_add(np,_t_detach_by_idx(c,1));
                        if (semeq(_t_symbol(c),COND_PAIR)) {
                            state->phase = EvalCondCondtions;
                        }
                        else {
                            state->phase = EvalCondResult;
                        }
                        *((CondState **)&np->contents.surface) = state;
                        np->contents.size = sizeof(CondState *);
                    }
                }
                else if (semeq(s,CONVERSE)) {
                    // if first time we are hitting the CONVERSE instruction
                    // in the tree (i.e. on the way down) we need to register
                    // the conversation IDs and make the tree
                    if (_t_size(np) == 0) {
                        UUIDt cuuid = __uuid_gen();
                        T *until,*wait = NULL;
                        //@todo get these value semantically i.e _t_get_siganture_child(np,"until");
                        until =_t_child(np,2);
                        if (until) {
                            if (semeq(_t_symbol(until),BOOLEAN)) {
                                wait = until;
                                until = NULL;
                            }
                            else wait = _t_child(np,3);
                        }

                        UUIDt *parent_u;
                        if (context->conversation) {
                            parent_u = __cid_getUUID(context->conversation->cid);
                        }
                        else {
                            parent_u = NULL;
                        }

                        T *c = _r_add_conversation(q->r,parent_u,&cuuid,until?_t_clone(until):NULL,
                                                   __p_build_wakeup_info(np,context->id)
                                                   );

                        ConversationState *state = malloc(sizeof(ConversationState));
                        state->converse_pointer = np;  // save the node pointer for later COMPLETEs
                        state->cid = _t_child(c,ConversationIdentIdx);
                        *((ConversationState **)&np->contents.surface) = state;
                        np->contents.size = sizeof(ConversationState *);
                        np->context.flags |= TFLAG_ALLOCATED;

                        // register the conversation with the context linking an existing conversation
                        // to the new one if it exists
                        state->next = context->conversation;
                        context->conversation = state;
                    }
                }
                if (count == get_rt_cur_child(q->r,np) || semeq(s,QUOTE)) {
                    // if the current child == the child count this means
                    // all the children have been processed, so we can evaluate this process
                    // if the process is QUOTE that's a special case and we evaluate it
                    // immediately without descending.
                    if (!is_sys_process(s)) {
                        debug(D_STEP,"Stepping into %s\n",_sem_get_name(sem,s));
                        // if it's user defined process then we check the signature and then make
                        // a new run-tree run that process

                        Error e = __p_check_signature(sem,s,np,context->sem_map);
                        if (e) {
                            context->state = e;
                        }
                        else {
                            T *run_tree = _p_make_run_tree(sem,s,np,context->sem_map);
                            context->state = Pushed;
                            // @todo for now we just are just passing the semantic map from one
                            // context to the next, but I'm pretty sure we're going to need a way
                            // for folks to modify this on the fly as processes are called
                            *contextP = __p_make_context(run_tree,context,context->id,context->sem_map);
                            debug(D_REDUCE,"New context for %s: %s\n\n",_sem_get_name(sem,s),_t2s(sem,run_tree));
                        }
                    }
                    else {
                        // if it's a sys process we can just reduce it in and then ascend
                        // or move to the error handling state

                        //Error e = __p_check_signature(sem,s,np,context->sem_map);
                        //if (e) raise_error("SIG FAILURE on %s\n",_t2s(sem,np));

                        Error e = __p_reduce_sys_proc(context,s,np,q);
                        if (e == redoReduction) {

			    // reset the node_pointer
                            np = context->node_pointer = _t_child(context->parent,context->idx);
			    // there are two reasons to redoReduction, one because the call to reduce_sys_proc
			    // added more code to the runtree that just still needs to be reduced
			    // or because it added a new run-tree, which needs to be treated as a function
			    // call and thus adding a new context
			    if (semeq(RUN_TREE,_t_symbol(np))) {
				context->state = Pushed;
				// swap out the RUN_TREE for a dummy proc
				// @todo really the error returned by reduce_sys_proc should be a struct
				// with the RUN_TREE in it so we don't have store it in the actual tree
				int i = _t_node_index(np);
				T *p = _t_parent(np);
				T *dummy = __t_newr(0,NOOP,true);
				p->structure.children[i-1] = dummy;
				dummy->structure.parent = p;
				np->structure.parent = NULL;
				*contextP = __p_make_context(np,context,context->id,context->sem_map);
				debug(D_REDUCE,"Redoing with a new context for: %s\n\n",_t2s(sem,np));
			    }
			    else {
				context->state = Eval;
				set_rt_cur_child(q->r,np,RUN_TREE_NOT_EVAULATED); // reset the current child count on the code
			    }
                        }
                        else context->state = e ? e : Ascend;
                    }

                }
                else if(count) {
                    //descend and increment the current child we're working on!
                    context->state = Descend;
                }
                else {
                    raise_error("whoa! brain fart! on %d,%s",count,_t2s(sem,np));
                }
            }
        }
        break;
    case Ascend:
        set_rt_cur_child(q->r,context->node_pointer,RUN_TREE_EVALUATED);
        context->node_pointer = context->parent;
        context->parent = _t_parent(context->node_pointer);
        if (!context->parent || context->parent == context->run_tree || (context->node_pointer == context->run_tree)) {
            context->idx = 1;
        }
        else {
            context->idx = get_rt_cur_child(q->r,context->parent);
        }
        if (context->node_pointer == context->run_tree)
            context->state = Pop;
        else
            context->state = Eval;
        break;
    case Descend:
        context->parent = context->node_pointer;
        rt_check(q->r,context->node_pointer);
        context->idx = ++rt_cur_child(context->node_pointer);
        context->node_pointer = _t_child(context->node_pointer,context->idx);
        context->state = Eval;
        break;
    default:
        context->err = context->state;
        if (_t_children(context->run_tree) <= 2) {
            // no error handler so just return the error
            if (debugging(D_STEP)) {
                T *err =  __p_buildErr(context);
                debug(D_STEP,"Reduction Err (no handler): %s\n",_t2s(sem,err));
                debug(D_STEP,"   node_pointer @ err: %s\n",_t2s(sem,context->node_pointer));
                _t_free(err);
            }
            context->state = Pop;
        }
        else {
            // the first parameter to the error code is always a reduction error
            // which gets added on as the 4th child of the run tree when the
            // error happens.
            T *ps = _t_newr(context->run_tree,PARAMS);

            T *err = __p_buildErr(context);
            debug(D_STEP,"In Error Handler with %s\n",_t2s(sem,err));
            debug(D_STEP,"   node_pointer @ err: %s\n",_t2s(sem,context->node_pointer));
            _t_add(ps,err);

            // switch the node_pointer to the top of the error handling routine
            context->node_pointer = _t_child(context->run_tree,RunTreeErrorCodeIdx);
            context->idx = RunTreeErrorCodeIdx;
            context->parent = context->run_tree;

            context->state = Eval;
        }
    }
    return context->state;
}


/**
 * low level function to build a WAKEUP_REFERENCE symbol used for unblocking a process
 */
T* __p_build_wakeup_info(T *code_point,int process_id) {
    T *wakeup = _t_new_root(WAKEUP_REFERENCE);
    _t_newi(wakeup,PROCESS_IDENT,process_id);
    int *path = _t_get_path(code_point);
    _t_new(wakeup,CODE_PATH,path,sizeof(int)*(_t_path_depth(path)+1));
    free(path);
    return wakeup;
}

/**
 * low level functon to build a run tree from a code tree and a variable list of params
 */
T *__p_build_run_tree_va(T* code,int num_params,va_list params) {
    T *t = _t_new_root(RUN_TREE);
    T *c = _t_rclone(code);
    _t_add(t,c);
    T *ps = _t_newr(t,PARAMS);
    int i;
    for(i=1;i<=num_params;i++) {
        _t_add(ps,_t_clone(va_arg(params,T *)));
    }
    return t;
}

T *__p_build_run_tree(T* code,int num_params,...) {
    va_list params;
    va_start(params,num_params);
    T *t = __p_build_run_tree_va(code,num_params,params);
    va_end(params);
    return t;
}

/**
 * Build a run tree from a code tree and params
 *
 * @param[in] sem current semantic context
 * @param[in] p process to be cloned into a run tree
 * @param[in] params parameters to be added to the run-tree
 *
 * @note the params get added, not cloned to the tree
 *
 * @returns T RUN_TREE tree
 */
T *_p_make_run_tree(SemTable *sem,Process p,T *params,T *sem_map) {
    if (!is_process(p)) {
        raise_error("not a Process!");
    }
    T *processes = _sem_get_defs(sem,p);
    T *code_def = _d_get_process_code(processes,p);
    T *t = _t_new_root(RUN_TREE);
    T *ps;

    T *code = _t_child(code_def,ProcessDefCodeIdx);

    // if this is a system process the code will be NULL_PROCESS so
    // we'll just add the params right onto the process node
    // and leave the run tree params empty
    if (semeq(_t_symbol(code),NULL_PROCESS)) {
        ps = __t_new(t,p,0,0,true);
        _t_newr(t,PARAMS);
    }
    else {
        // otherwise we clone the code of the process
        T *c = _t_rclone(code);
        _t_add(t,c);
        ps = _t_newr(t,PARAMS);
    }
    int i,num_params = _t_children(params);
    for(i=1;i<=num_params;i++) {
        _t_add(ps,_t_detach_by_idx(params,1));
    }

    if (sem_map) {
        __t_fill_template(t,sem_map,true);
    }
    return t;
}

/**
 * create a new processing queue
 *
 * @param[in] r receptor in which to create the Queue
 * @returns Q the processing queue
 */
Q *_p_newq(Receptor *r) {
    Q *q = malloc(sizeof(Q));
    q->r = r;
    q->contexts_count = 0;
    q->active = NULL;
    q->completed = NULL;
    q->blocked = NULL;
    pthread_mutex_init(&(q->mutex), NULL);
    return q;
}

// clean up a context including its run-trees
void _p_free_context(R *c) {
    while(c) {
        // free any run_trees that are roots, i.e. assume
        // that a tree in a context that's part of another tree
        // will get freed elsewhere.
        if (!_t_parent(c->run_tree))
            _t_free(c->run_tree);
        R *n = c->caller;
        free(c);
        c = n;
    }
}

// clean up a queue element
void _p_free_elements(Qe *e) {
    while(e) {
        _p_free_context(e->context);
        Qe *n = e->next;
        free(e);
        e = n;
    }
}

/**
 * free the resources in a queue, including any run-trees
 *
 * @param[in] q the queue to be freed
 */
void _p_freeq(Q *q) {
    Qe *e = q->active;
    _p_free_elements(q->active);
    _p_free_elements(q->completed);
    _p_free_elements(q->blocked);
    free(q);
}

int G_next_process_id = 0;
/**
 * add a run tree into a processing queue
 *
 * @todo make thread safe.  currently you shouldn't call this if the Q is being actively reduced
 */
Qe *__p_addrt2q(Q *q,T *run_tree,T *sem_map) {
    Qe *n = malloc(sizeof(Qe));
    n->id = ++G_next_process_id;
    n->prev = NULL;
    n->context = __p_make_context(run_tree,0,n->id,sem_map);
    n->accounts.elapsed_time = 0;
    debug(D_LOCK,"addrt2q LOCK\n");
    pthread_mutex_lock(&q->mutex);
    __p_append(q->active,n);
    q->contexts_count++;
    pthread_mutex_unlock(&q->mutex);
    debug(D_LOCK,"addrt2q UNLOCK\n");

    return n;
}

/**
 * reduce all the processes in a queue, and terminate thread when completed
 *
 * @param[in] q the queue to be processed
 */
void *_p_reduceq_thread(void *arg){
    Q *q = (Q*)arg;

    int err;
    err = _p_reduceq((Q *)arg);
    pthread_exit(NULL);
}

#ifdef CEPTR_DEBUG
void debug_np(int type,T *np) {
    int *path = _t_get_path(np);
    char pp[255];
    _t_sprint_path(path,pp);
    debug(type,"Node Pointer:%s\n",pp);
    free(path);
}
#endif


#ifdef CEPTR_DEBUG
char *sn[]={"Done","Ascend","Descend","Pushed","Pop","Eval","Block"};
char *__debug_state_str(R *context) {
    int s = context->state;

    char *result;
    if (s <= 0) result = sn[-s];
    else {
        T *e = __p_buildErr(context);
        result = t2s(e);
        _t_free(e);
    }
    return result;
}
#endif
/**
 * reduce all the processes in a queue
 *
 * @param[in] q the queue to be processed
 */
Error _p_reduceq(Q *q) {
    debug(D_REDUCE+D_REDUCEV,"Starting reduce:\n");

    Qe *qe = q->active;
    Error next_state;
    struct timespec start, end;

    while (q->contexts_count) {
#ifdef CEPTR_DEBUG
        if (debugging(D_REDUCEV)) {
            R *context = qe->context;
            char *s = __debug_state_str(context);
            debug(D_REDUCEV,"ID:%d -- State %s(%d)\n",qe->id,s,context->state);
            debug(D_REDUCEV,"  idx:%d\n",context->idx);
            debug(D_REDUCEV,"%s\n",_t2s(q->r->sem,context->run_tree));
            if (context) {
                if (context->node_pointer == 0) {
                    debug(D_REDUCEV,"Node Pointer: NULL!\n");
                }
                else {
                    debug(D_REDUCEV,"rt_cur_child:%d\n",rt_cur_child(context->node_pointer));
                    debug_np(D_REDUCEV,context->node_pointer);
                }
            }
        }
        int prev_state;
        if (debugging(D_REDUCEV+D_REDUCE)) {
            prev_state = qe->context->state;
        }
#endif

        clock_gettime(CLOCK_MONOTONIC, &start);
        next_state = _p_step(q, &qe->context); // next state is set in directly in the context
        clock_gettime(CLOCK_MONOTONIC, &end);
        qe->accounts.elapsed_time +=  diff_micro(&start, &end);

#ifdef CEPTR_DEBUG
        debug(D_REDUCEV,"result state:%s\n\n",__debug_state_str(qe->context));
        if (debugging(D_REDUCE) && prev_state == Eval) {
            debug_np(D_REDUCE,qe->context->node_pointer);
            debug(D_REDUCE,"Eval: %s\n\n",_t2s(q->r->sem,qe->context->run_tree));
        }
#endif
        debug(D_LOCK,"reduce LOCK\n");
        pthread_mutex_lock(&q->mutex);
        Qe *next = qe->next;
        if (next_state == Done) {
            // remove from the round-robin
            __p_dequeue(q->active,qe);

            debug(D_REDUCEV,"Just completed:%d\n",qe->id);

            // add to the completed list
            __p_enqueue(q->completed,qe);
            q->contexts_count--;
        }
        else if (next_state == Block) {
            // remove from the round-robin
            __p_dequeue(q->active,qe);

            // add to the blocked list
            __p_enqueue(q->blocked,qe);
            q->contexts_count--;
        }
        qe = next ? next : q->active;  // next in round robin or wrap back to first
        pthread_mutex_unlock(&q->mutex);
        debug(D_LOCK,"reduce UNLOCK\n");
    };

    /// @todo figure out what error we should be sending back here, i.e. what if
    // one process ended ok, but one did not.  What's the error?  Probably
    // the errors here would be at a different level, and the caller would be
    // expected to inspect the errors of the reduced processes.
    debug(D_REDUCE+D_REDUCEV,"Ending reduce\n");
    return 0;
}

/**
 * cleanup any completed process from the queue, updating the receptor state data as necessary
 *
 * @param[in] q the queue to be cleaned up
 */
void _p_cleanup(Q *q) {
    debug(D_LOCK,"cleanup LOCK\n");
    pthread_mutex_lock(&q->mutex);
    Qe *e = q->completed;
    while (e) {
        T *ett = _t_child(_t_child(q->r->root,ReceptorInstanceStateIdx),ReceptorElapsedTimeIdx);
        int *et = (int *)_t_surface(ett);
        (*et) += e->accounts.elapsed_time;
        e = e->next;
    }
    _p_free_elements(q->completed);
    q->completed = NULL;
    pthread_mutex_unlock(&q->mutex);
    debug(D_LOCK,"cleanup UNLOCK\n");
}

/**
 * utility function to build process form structures
 *
 * @param[in] symbol the type of process form to build, i.e. PROCESS_SIGNATURE
 * @param[in] output_label
 * @param[in] output_symbol
 * @param[in] var_args triplets of label, symbol type, and symbol value for the signature
 * @param[out] form tree
 *
 * @todo add more sensible handling of output signature.  We still aren't quite sure what to do about the output label, and how to handle output signature pass through from the results, probably need a special symbol for that
 */
T *__p_make_form(Symbol sym,char *output_label,Symbol output_type,SemanticID output_sem,...){
    va_list params;
    va_start(params,output_sem);
    char *label;
    Symbol type,value;
    int optional;
    T *signature = _t_new_root(sym);
    T *o = _t_newr(signature,OUTPUT_SIGNATURE);
    T *l = _t_newr(o,SIGNATURE_LABEL);
    _t_new_str(l,ENGLISH_LABEL,output_label);
    if (semeq(output_type,SIGNATURE_PASSTHRU)) {
        _t_newr(o,output_type);
    }
    else {
        _t_news(o,output_type,output_sem);
    }
    while (label = va_arg(params,char*)) {
        type = va_arg(params,Symbol);
        if (semeq(type,SIGNATURE_OPTIONAL)) {
            optional = 1;
            type = va_arg(params,Symbol);
        }
        else {
            optional = 0;
        }
        value = va_arg(params,Symbol);
        T *i = _t_newr(signature,INPUT_SIGNATURE);
        l = _t_newr(i,SIGNATURE_LABEL);
        _t_new_str(l,ENGLISH_LABEL,label);
        _t_news(i,type,value);
        if (optional) _t_newr(i,SIGNATURE_OPTIONAL);
    }
    va_end(params);
    return signature;
}

/** @}*/
