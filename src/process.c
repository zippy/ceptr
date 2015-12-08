/**
 * @ingroup receptor
 *
 * @{
 * @file process.c
 * @brief implementation of ceptr processing: instructions and run tree reduction
 * @todo implement a way to define sys_processes input and output signatures
 *
 * @copyright Copyright (C) 2013-2015, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
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
    while (err = _p_unblock((Q *)st->callback_arg1,st->callback_arg2)) {
        sleepms(1);
    }
    //    if (err) raise_error("couldn't unblock!");
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
 * implements the INTERPOLATE_FROM_MATCH process
 *
 * replaces the interpolation tree with the matched sub-parts from a semtrex match results tree
 *
 * @param[in] t interpolation tree to be scanned for INTERPOLATE_SYMBOL nodes
 * @param[in] match_results SEMTREX_MATCH_RESULTS tree
 * @param[in] match_tree original tree that was matched (needed to grab the data to interpolate)
 * @todo what to do if match has sibs??
 */
void _p_interpolate_from_match(T *t,T *match_results,T *match_tree) {
    if (semeq(_t_symbol(t),INTERPOLATE_SYMBOL)) {
        Symbol s = *(Symbol *)_t_surface(t);
        int sibs;
        T *x = _stx_get_matched_node(s,match_results,match_tree,&sibs);
        if (sibs > 1) raise_error("not implemented for sibs > 1");

        //@todo most contexts where we use interpolation are in run trees
        // hence the rclone.  Perhaps there should be an option to this function
        // to allow regular cloning?
        x = _t_rclone(x);
        _t_replace_node(t,x);

        t = x;
        // set the symbol to the interpolate type, not the matched item type
        // because this allows for type conversion on semtrex matching using
        // the semtrex group name to as the indicator of which the new type will be
        if (!semeq(s,NULL_SYMBOL)) // if match was on NULL_SYMBOL it's a full results match so use what we found
            t->contents.symbol = s;
    }
    DO_KIDS(t,_p_interpolate_from_match(_t_child(t,i),match_results,match_tree));
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
Error __p_check_signature(SemTable *sem,Process p,T *params) {
    T *processes = _sem_get_defs(sem,p);
    T *def = _d_get_process_code(processes,p);
    T *signature = _t_child(def,ProcessDefSignatureIdx);
    int i = _t_children(signature) - 1; // there's always one output signature
    int c = _t_children(params);
    if (i > c) return tooFewParamsReductionErr;
    if (i < c) return tooManyParamsReductionErr;
    for(i=2;i<=c;i++) {
        T *sig = _t_child(_t_child(signature,i),2); // input signatures start at 2
        if(semeq(_t_symbol(sig),SIGNATURE_STRUCTURE)) {
            Structure ss = *(Symbol *)_t_surface(sig);
            if (!semeq(_sem_get_symbol_structure(sem,_t_symbol(_t_child(params,i-1))),ss) && !semeq(ss,TREE))
                return signatureMismatchReductionErr;
        }
        else if(semeq(_t_symbol(sig),SIGNATURE_SYMBOL)) {
            raise_error("not implemented");
        }
        else if(semeq(_t_symbol(sig),SIGNATURE_ANY)) {
        }
        else {
            raise_error("unknown signature checking symbol: %s",_sem_get_name(sem,_t_symbol(sig)));
        }
    }
    return 0;
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
    switch(s.id) {
    case NOOP_ID:
        // noop simply replaces itself with it's own child
        /// @todo what happens if it has more than one child! validity check?
        x = _t_detach_by_idx(code,1);
        break;
    case GET_ID:
        {
            T *t = _t_detach_by_idx(code,1);
            Xaddr xa = *(Xaddr *)_t_surface(t);
            T *v = _r_get_instance(q->r,xa);
            if (!v) raise_error("Invalid xaddr in GET");
            x = _t_rclone(v);
            _t_free(t);
        }
        break;
    case IF_ID:
        t = _t_child(code,1);
        b = (*(int *)_t_surface(t)) ? 2 : 3;
        x = _t_detach_by_idx(code,b);
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
    case CONCAT_STR_ID:
        // if the first parameter is a RESULT SYMBOL then we use that as the symbol type for the result tree.
        x = _t_detach_by_idx(code,1);
        sy = _t_symbol(x);
        if (semeq(RESULT_SYMBOL,sy)) {
            sy = *(Symbol *)_t_surface(x);
            _t_free(x);
            x = _t_detach_by_idx(code,1);
        }
        //@todo, add a bunch of sanity checking here to make sure the
        // parameters are all CSTRINGS
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
        /// @todo this would probably be faster with just one total realloc for all children
        for(b=1;b<=c;b++) {
            str = (char *)_t_surface(_t_child(code,b));
            int size = strlen(str);
            x->contents.surface = realloc(x->contents.surface,x->contents.size+size);
            memcpy(x->contents.surface+x->contents.size-1,str,size);
            x->contents.size+=size;
            *( (char *)x->contents.surface + x->contents.size -1) = 0;
        }
        x->contents.symbol = sy;
        break;
    case RESPOND_ID:
        {
            T *signal = _t_parent(context->run_tree);
            if (!signal || !semeq(_t_symbol(signal),SIGNAL))
                return notInSignalContextReductionError;
            T *t = _t_detach_by_idx(code,1);
            Symbol carrier = *(Symbol*)_t_surface(t);
            _t_free(t);
            T *response_contents = _t_detach_by_idx(code,1);
            T *envelope = _t_child(signal,SignalEnvelopeIdx);
            ReceptorAddress to = __r_get_addr(_t_child(envelope,EnvelopeFromIdx)); // from and to reverse in response
            ReceptorAddress from = __r_get_addr(_t_child(envelope,EnvelopeToIdx));
            Aspect a = *(Aspect *)_t_surface(_t_child(envelope,EnvelopeAspectIdx));
            UUIDt uuid = *(UUIDt *)_t_surface(_t_child(envelope,EnvelopeUUIDIdx));

            T *response = __r_make_signal(from,to,a,carrier,response_contents,&uuid,0);
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
                signal = __r_make_signal(from,to,aspect,carrier,signal_contents,0,0);
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
                    raise_error("request callback not implemented");
                }
                signal = __r_make_signal(from,to,aspect,carrier,signal_contents,0,until);

                x = _r_request(q->r,signal,response_carrier,response_point,context->id);
            }
        }
        break;
    case INTERPOLATE_FROM_MATCH_ID:
        match_results = _t_child(code,2);
        match_tree = _t_child(code,3);
        x = _t_detach_by_idx(code,1);
        /// @todo interpolation errors?
        _p_interpolate_from_match(x,match_results,match_tree);
        break;
    case RAISE_ID:
        return raiseReductionErr;
        break;
    case STREAM_READ_ID:
        {
            // get the stream param
            T *s = _t_child(code,1);
            Stream *st = _t_surface(s);
            if (st->type != UnixStream) raise_error("unknown stream type:%d\n",st->type);
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
                    x = __t_new(0,sy,_st_data(st),_st_data_size(st),1);
                    _st_data_read(st);
                }
                else {raise_error("expecting RESULT_SYMBOL");}
            }
            else if (st->flags & StreamAlive) {
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
            if (st->type != UnixStream) raise_error("unknown stream type:%d\n",st->type);
            FILE *stream = st->data.unix_stream;
            _t_free(s);
            // get the data to write as string
            while(s = _t_detach_by_idx(code,1)) {
                /// @todo check the structure type to make sure it's compatible as a string (i.e. it's null terminated)
                /// @todo other integrity checks, i.e. length etc?
                char *str;
                Symbol sym = _t_symbol(s);
                Structure struc = _sem_get_symbol_structure(q->r->sem,sym);
                int add_nl = 1;
                if (semeq(struc,CSTRING)) {
                    str = _t_surface(s);
                    debug(D_STREAM,"found a cstring to write: %s\n",str);
                    if (!semeq(sym,LINE)) add_nl = 0;
                }
                else {
                    str = _t2s(q->r->sem,s);
                }
                //str = t2s(s);
                int err = fputs(str,stream);
                if (err < 0) {
                    debug(D_STREAM,"unix error %d on attempting to write: %s\n",errno,str);
                    _t_free(s);
                    return unixErrnoReductionErr;
                }
                else {
                    debug(D_STREAM,"just wrote: %s\n",str);
                }
                _t_free(s);
                if (add_nl) {
                    err = fputs("\n",stream);
                    if (err < 0) return unixErrnoReductionErr;
                }
            }
            fflush(stream);
            /// @todo what should this really return?
            x = __t_news(0,REDUCTION_ERROR_SYMBOL,NULL_SYMBOL,1);
        }
        break;
    case STREAM_ALIVE_ID:
        {
            // get the stream param
            T *s = _t_detach_by_idx(code,1);
            Stream *st = _t_surface(s);
            if (st->type != UnixStream) raise_error("unknown stream type:%d\n",st->type);
            FILE *stream = st->data.unix_stream;
            if (st->flags & StreamAlive)
                if (feof(stream)) st->flags &= ~StreamAlive;
            debug(D_STREAM,"checking if StreamAlive: %s\n",st->flags & StreamAlive ? "yes":"no");
            x = __t_newi(0,BOOLEAN, (st->flags&StreamAlive)?1:0,1);
            _t_free(s);
        }
        break;
    case ITERATE_ID:
        // replicate is a special case, we have to check the phase to see what to do
        // after the children have been evaluated.
        {
            ReplicationState *state = *(ReplicationState **)_t_surface(code);
            int done = 0;
            int next_phase;
            // get the condition or body results into x
            x = _t_detach_by_idx(code,2);
            switch(state->phase) {
            case EvalCondition: {
                // if this is the first time evaluating the cond, figure out what type
                // of replication we are doing based on the semantics
                if (state->type == ReplicateTypeUnknown) {
                    Symbol c = _t_symbol(x);
                    if (semeq(c,BOOLEAN)) {
                        state->type = ReplicateTypeCond;
                        state->count = 0;
                    }
                    else {
                        Structure s = _sem_get_symbol_structure(q->r->sem,c);
                        if (semeq(s,INTEGER)) {
                            state->type = ReplicateTypeCount;
                            state->count = *(int *)_t_surface(x);
                        }
                        else {
                            raise_error("unable to determine replication type!");
                        }
                    }
                }
                // evaluate condition based on replication type
                switch(state->type) {
                case ReplicateTypeCond:
                    done = !*(int *)_t_surface(x);
                    break;
                case ReplicateTypeCount:
                    done = (--state->count < 0);
                    break;
                }
                next_phase = EvalBody;
                break;
            }
            case EvalBody:
                // if this is count replication, just check the counter for done
                next_phase = (state->type == ReplicateTypeCount) ? EvalBody : EvalCondition;
                if (state->type == ReplicateTypeCount){
                    if (--state->count < 0) done = 1;
                }
                else {
                    // if we aren't doing count replication, use the count var just
                    // to keep track of how many times we've gone through the loop
                    state->count++;
                    if (state->count > 9) done = 1;  // temporary infinite loop breaker
                }
            }
            if (done) {
                // we are done so free up the replication state info
                /// @todo the value returned from the loop will be what??(what's in x)
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
                _t_news(with,INTERPOLATE_SYMBOL,NULL_SYMBOL);
            }
            if (act) {
                _r_add_expectation(q->r,aspect,carrier,match,act,with,until);
                x = __t_news(0,REDUCTION_ERROR_SYMBOL,NULL_SYMBOL,1);
                debug(D_LISTEN,"adding expectation\n");
            }
            else {
                act = __r_build_wakeup_info(code,context->id);
                if (!until) {
                    until = _t_new_root(END_CONDITIONS);
                    _t_newi(until,COUNT,1);
                }
                _r_add_expectation(q->r,aspect,carrier,match,act,with,until);
                debug(D_LISTEN,"adding expectation and blocking at %d,%s\n",context->id,_td(q->r,code));
                return Block;
            }
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
                        Receptor *r = G_vm->routing_table[i];
                        if (r) {
                            SemanticID sid ={r->parent,SEM_TYPE_RECEPTOR,r->context};
                            char *n = _sem_get_name(r->sem,sid);
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

    // any remaining children of 'code' are the parameters which have all now been "used up"
    // so we can call the low-level __t_free the clean them up and then replace the contents of
    // the 'code' node with the contents of the 'x' node that was either detached or produced
    // by the the process that just ran
    __t_free(code);
    code->structure.child_count = x->structure.child_count;
    code->structure.children = x->structure.children;
    code->contents = x->contents;
    code->context = x->context;
    free(x);
    return err;
}

/**
 * create a run-tree execution context.
 */
R *__p_make_context(T *run_tree,R *caller,int process_id) {
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
void __p_unblock(Q *q,Qe *e) {
    __p_dequeue(q->blocked,e);
    __p_enqueue(q->active,e);
    q->contexts_count++;
    e->context->state = Eval;
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
        __p_unblock(q,e);
    }
    else {
        err = 1;
    }
    pthread_mutex_unlock(&q->mutex);
    debug(D_LOCK,"unblock UNLOCK\n");
    return err;
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
    R *context = __p_make_context(run_tree,0,0);
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

    switch(context->state) {
    case noReductionErr:
    case Block:
        raise_error("whoa, virtual states can't be executed!"); // shouldn't be calling step if Done or noErr or Block
        break;
    case Pop:
        // if this was the successful reduction by an error handler
        // move the value to the 1st child
        if (context->err) {
            T *t = _t_detach_by_idx(context->run_tree,3);
            if (t) {
                _t_replace(context->run_tree,1,t);

                context->err = noReductionErr;
            }
        }

        // if this is top caller on the stack then we are completely done
        if (!context->caller) {
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

            if (semeq(s,PARAM_REF)) {
                T *param = _t_get(context->run_tree,(int *)_t_surface(np));
                if (!param) {
                    raise_error("request for non-existent param");
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
                    raise_error("request for non-existent signal portion");
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
                if (count && (count != get_rt_cur_child(q->r,np)))
                    context->state = Descend;
                else
#endif
                    context->state = Ascend;
            }
            else
                {
                if (semeq(s,ITERATE)) {
                    // if first time we are hitting this replication
                    // then we need to set it up
                    //                    raise(SIGINT);
                    if (_t_size(np) == 0) {
                        // sanity check
                        if (_t_children(np) != 3) {raise_error("ITERATE must have 3 params");}
                        // create a copy of the code and stick it in the replication state struct
                        ReplicationState *state = malloc(sizeof(ReplicationState));
                        state->phase = EvalCondition;
                        state->code = _t_rclone(np);
                        state->type = ReplicateTypeUnknown;
                        *((ReplicationState **)&np->contents.surface) = state;
                        np->contents.size = sizeof(ReplicationState *);
                        // we start in condition phase so throw away the code copy
                        T *x = _t_detach_by_idx(np,3);
                        _t_free(x);
                    }
                }
                if (count == get_rt_cur_child(q->r,np) || semeq(s,QUOTE)) {
                    // if the current child == the child count this means
                    // all the children have been processed, so we can evaluate this process
                    // if the process is QUOTE that's a special case and we evaluate it
                    // immediately without descending.
                    if (!is_sys_process(s)) {
                        // if it's user defined process then we check the signature and then make
                        // a new run-tree run that process
                        Error e = __p_check_signature(q->r->sem,s,np);
                        if (e) context->state = e;
                        else {
                            T *processes = _sem_get_defs(q->r->sem,s);
                            T *run_tree = __p_make_run_tree(processes,s,np);
                            context->state = Pushed;
                            *contextP = __p_make_context(run_tree,context,context->id);
                        }
                    }
                    else {
                        // if it's a sys process we can just reduce it in and then ascend
                        // or move to the error handling state
                        Error e = __p_reduce_sys_proc(context,s,np,q);
                        context->state = e ? e : Ascend;
                    }
                }
                else if(count) {
                    //descend and increment the current child we're working on!
                    context->state = Descend;
                }
                else {
                    raise_error("whoa! brain fart! on %d,%s",count,_t2s(q->r->sem,np));
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
            context->state = Pop;
        }
        else {
            // the first parameter to the error code is always a reduction error
            // which gets added on as the 4th child of the run tree when the
            // error happens.
            T *ps = _t_newr(context->run_tree,PARAMS);
            T *extra = NULL;
            //@todo: fix this so we don't actually use an error value that
            // then has to be translated into a symbol, but rather so that we
            // can programatically calculate the symbol.
            // or, perhaps, the Error type should actually be the REDUCTION_ERROR
            // symbol that we generate when we want to return an error during the
            // reduction process (rather than here).  That way we actually have the
            // symbol whether or not a we have an error handler for context.
            Symbol se;
            switch(context->state) {
            case tooFewParamsReductionErr: se=TOO_FEW_PARAMS_ERR;break;
            case tooManyParamsReductionErr: se=TOO_MANY_PARAMS_ERR;break;
            case signatureMismatchReductionErr: se=SIGNATURE_MISMATCH_ERR;break;
            case notProcessReductionError: se=NOT_A_PROCESS_ERR;break;
            case notInSignalContextReductionError: se=NOT_IN_SIGNAL_CONTEXT_ERR;
            case divideByZeroReductionErr: se=ZERO_DIVIDE_ERR;break;
            case incompatibleTypeReductionErr: se=INCOMPATIBLE_TYPE_ERR;break;
            case deadStreamReadReductionErr: se=DEAD_STREAM_READ_ERR;break;
            case unixErrnoReductionErr:
                /// @todo make a better error symbol here... :-P
                extra = _t_new_str(0,TEST_STR_SYMBOL,strerror(errno));
                break;
            case raiseReductionErr:
                se = *(Symbol *)_t_surface(_t_child(context->node_pointer,1));
                break;
            default: raise_error("unknown reduction error: %d",context->state);
            }
            T *err = __t_new(ps,se,0,0,1);
            int *path = _t_get_path(context->node_pointer);
            _t_new(err,ERROR_LOCATION,path,sizeof(int)*(_t_path_depth(path)+1));
            free(path);
            if (extra) {
                _t_add(err,extra);
            }
            // switch the node_pointer to the top of the error handling routine
            context->node_pointer = _t_child(context->run_tree,RunTreeErrorCodeIdx);
            context->idx = RunTreeErrorCodeIdx;
            context->parent = context->run_tree;

            context->state = Eval;
        }
    }
    return context->state;
}

/*
  special runtree builder that uses the actual params tree node.  This is used
  in the process of reduction because we don't have to clone the param values, we
  can use the actual tree nodes that are being reduced as they are already rT nodes
  and they are only used once, i.e. in this reduction
*/
T *__p_make_run_tree(T *processes,Process p,T *params) {
    T *t = _t_new_root(RUN_TREE);
    T *ps;
    // if this is a system process, we'll just add the params right onto the process node
    // and leave the run tree params empty
    if (is_sys_process(p)) {
        ps = __t_new(t,p,0,0,1);
        _t_newr(t,PARAMS);
    }
    else {
        // otherwise we get the code of the process
        T *code_def = _d_get_process_code(processes,p);
        T *code = _t_child(code_def,ProcessDefCodeIdx);
        T *c = _t_rclone(code);
        _t_add(t,c);
        ps = _t_newr(t,PARAMS);
    }
    int i,num_params = _t_children(params);
    for(i=1;i<=num_params;i++) {
        _t_add(ps,_t_detach_by_idx(params,1));
    }
    return t;
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
 * @param[in] processes processes trees
 * @param[in] process Process tree node to be turned into run tree
 * @param[in] num_params the number of parameters to add to the parameters child
 * @param[in] ... T params
 * @returns T RUN_TREE tree
 */
T *_p_make_run_tree(T *processes,T *process,int num_params,...) {

    T *t = NULL;
    va_list params;

    Process p = *(Process *)_t_surface(process);
    if (!is_process(p)) {
        raise_error("not a Process!");
    }
    if (is_sys_process(p)) {
        t =  _t_new_root(RUN_TREE);
        // if it's a sys process we add the parameters directly as children to the process
        // because no sys-processes refer to PARAMS by path
        // this also means we need rclone them instead of clone them because they
        // will actually need to have space for the status marks by the processing code
        T *c = __t_new(t,p,0,0,1);

        va_start(params,num_params);
        int i;
        for(i=0;i<num_params;i++) {
            _t_add(c,_t_rclone(va_arg(params,T *)));
        }
        va_end(params);
    }
    else {
        T *code_def = _d_get_process_code(processes,p);
        T *code = _t_child(code_def,3);

        va_list params;
        va_start(params,num_params);
        t = __p_build_run_tree_va(code,num_params,params);
        va_end(params);
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

// clean up a queue element
_p_free_elements(Qe *e) {
    while(e) {
        _p_free_context(e->context);
        Qe *n = e->next;
        free(e);
        e = n;
    }
}

// clean up a context including its run-trees
_p_free_context(R *c) {
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
Qe *_p_addrt2q(Q *q,T *run_tree) {
    Qe *n = malloc(sizeof(Qe));
    n->id = ++G_next_process_id;
    n->prev = NULL;
    n->context = __p_make_context(run_tree,0,n->id);
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
        char *sn[]={"Done","Ascend","Descend","Pushed","Pop","Eval","Block"};
#define __debug_state_str(s) (s <= 0 ? sn[-s] : "Error")
        if (debugging(D_REDUCEV)) {
            R *context = qe->context;
            char *s = __debug_state_str(context->state);
            debug(D_REDUCEV,"ID:%p -- State %s(%d)\n",qe,s,context->state);
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
        debug(D_REDUCEV,"result state:%s\n\n",__debug_state_str(qe->context->state));
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

            debug(D_REDUCEV,"Just completed:\n");

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
    _t_new_str(o,SIGNATURE_LABEL,output_label);
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
        _t_new_str(i,SIGNATURE_LABEL,label);
        _t_news(i,type,value);
        if (optional) _t_newr(i,SIGNATURE_OPTIONAL);
    }
    va_end(params);
    return signature;
}

/** @}*/
