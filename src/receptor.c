/**
 * @ingroup receptor
 *
 * @{
 * @file receptor.c
 * @brief receptor implementation
 *
 * @copyright Copyright (C) 2013-2015, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 */

#include "receptor.h"
#include "stream.h"
#include "semtrex.h"
#include "process.h"
#include "accumulator.h"
#include "debug.h"
#include <stdarg.h>
#include <time.h>
#include <unistd.h>

Xaddr G_null_xaddr  = {0,0};
/*****************  create and destroy receptors */


Receptor *__r_new(Symbol s,T *defs,T *aspects) {
    Receptor *r = malloc(sizeof(Receptor));
    r->root = _t_new_root(s);
    _t_add(r->root,defs);
    r->defs.structures = _t_child(defs,1);
    r->defs.symbols = _t_child(defs,2);
    r->defs.processes = _t_child(defs,3);
    r->defs.protocols = _t_child(defs,4);
    r->defs.scapes = _t_child(defs,5);
    _t_add(r->root,aspects);
    r->flux = _t_newr(r->root,FLUX);
    T *a = _t_newi(r->flux,ASPECT,DEFAULT_ASPECT);
    _t_newr(a,LISTENERS);
    _t_newr(a,SIGNALS);
    r->table = NULL;
    r->instances = NULL;
    r->q = _p_newq(&r->defs);
    r->state = Alive;
    return r;
}

/**
 * @brief Creates a new receptor
 *
 * allocates all the memory needed in the heap
 *
 * @param[in] s symbol for this receptor
 * @returns pointer to a newly allocated Receptor
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/receptor_spec.h testReceptorCreate
 */
Receptor *_r_new(Symbol s) {
    T *defs = _t_new_root(DEFINITIONS);
    _t_newr(defs,STRUCTURES);
    _t_newr(defs,SYMBOLS);
    _t_newr(defs,PROCESSES);
    _t_newr(defs,PROTOCOLS);
    _t_newr(defs,SCAPES);
    T *aspects = _t_new_root(ASPECTS);
    return __r_new(s,defs,aspects);
}

// set the labels in the label table for the given def
void __r_set_labels(Receptor *r,T *defs,int sem_type) {
    DO_KIDS(
            defs,
            T *def = _t_child(defs,i);
            T *sl = _t_child(def,1);
            __set_label_for_def(r,_t_surface(sl),def,sem_type);
            );
}

/**
 * @brief Creates a new receptor from a receptor package
 *
 * allocates all the memory needed in the heap, cloning the various parts from the package
 * and binding the new receptor to the provided bindings
 *
 * @param[in] s symbol for this receptor
 * @returns pointer to a newly allocated Receptor
 * @todo implement bindings
 */
Receptor *_r_new_receptor_from_package(Symbol s,T *p,T *bindings) {
    T *defs = _t_clone(_t_child(p,3));
    T *aspects = _t_clone(_t_child(p,4));
    Receptor * r = __r_new(s,defs,aspects);

    //@todo fix this because it relies on SemanticTypes value matching the index order in the definitions.
    DO_KIDS(defs,__r_set_labels(r,_t_child(defs,i),i));

    return r;
}

/**
 * Adds an expectation/action pair to a receptor's aspect.
 */
void _r_add_listener(Receptor *r,Aspect aspect,Symbol carrier,T *expectation,T* params,T *action) {
    T *l = _t_news(0,LISTENER,carrier);
    _t_add(l,expectation);
    _t_add(l,params);
    _t_add(l,action);
    T *a = __r_get_listeners(r,aspect);
    _t_add(a,l);
}

/**
 * Destroys a receptor freeing all memory it uses.
 */
void _r_free(Receptor *r) {
    _t_free(r->root);
    lableTableFree(&r->table);
    _a_free_instances(&r->instances);
    if (r->q) _p_freeq(r->q);

    free(r);
}

/*****************  receptor symbols, structures and processes */

/**
 * we use this for labeling symbols, structures and processes because labels store the full path to the labeled item and we want the labels to be unique across all three
 */
SemanticID __set_label_for_def(Receptor *r,char *label,T *def,int type) {
    int *path = _t_get_path(def);
    labelSet(&r->table,label,path);
    int i = path[_t_path_depth(path)-1];
    free(path);
    SemanticID s = {RECEPTOR_CONTEXT,type,i};
    return s;
}

/**
 * Get the child index for a given label.
 *
 * This works for retrieving symbols, structures & processes because the symbol and structure values is just the child index.
 */
SemanticID  __get_label_idx(Receptor *r,char *label) {
    SemanticID s = {RECEPTOR_CONTEXT,0,0};
    int *path = labelGet(&r->table,label);
    if (path) {
        s.id = path[_t_path_depth(path)-1];
        s.flags = path[1]; // definitions index == semantic type!!
    }
    return s;
}

/**
 * define a new symbol
 *
 * @param[in] r receptor to provide a structural context for symbol declarations
 * @param[in] s the structure type for this symbol
 * @param[in] label a c-string label for this symbol
 * @returns the new Symbol
 *
 */
Symbol _r_declare_symbol(Receptor *r,Structure s,char *label){
    T *def = __d_declare_symbol(r->defs.symbols,s,label);
    return __set_label_for_def(r,label,def,SEM_TYPE_SYMBOL);
}

/**
 * define a new structure
 *
 * @param[in] r receptor to provide a semantic context for new structure definitions
 * @param[in] s the structure type for this symbol
 * @param[in] label a c-string label for this symbol
 * @param[in] num_params number of symbols in the structure
 * @param[in] ... variable list of Symbol type symbols
 * @returns the new Structure
 *
 */
Structure _r_define_structure(Receptor *r,char *label,int num_params,...) {
    va_list params;
    va_start(params,num_params);
    T *def = _dv_define_structure(r->defs.symbols,r->defs.structures,label,num_params,params);
    va_end(params);

    return __set_label_for_def(r,label,def,SEM_TYPE_STRUCTURE);
}

/**
 * add a new process coding to a receptor
 *
 * @param[in] r the receptor
 * @param[in] code the code tree for this process
 * @param[in] name the name of the process
 * @param[in] intention a description of what the process intends to do/transform
 * @param[in] in the input signature for the process
 * @param[in] out the output signature for the process
 * @returns the new Process
 *
 */
Process _r_code_process(Receptor *r,T *code,char *name,char *intention,T *in,T *out) {
    T *def = __d_code_process(r->defs.processes,code,name,intention,in,out);
    return __set_label_for_def(r,name,def,SEM_TYPE_PROCESS);
}

/**
 * find a symbol by its label
 */
Symbol _r_get_symbol_by_label(Receptor *r,char *label) {
    return __get_label_idx(r,label);
}

/**
 * find a structure by its label
 */
Structure _r_get_structure_by_label(Receptor *r,char *label){
    return __get_label_idx(r,label);
}

/**
 * @brief find a symbol's structure
 * @returns structure id
 */
Structure __r_get_symbol_structure(Receptor *r,Symbol s){
    return _d_get_symbol_structure(r->defs.symbols,s);
}

/**
 * get the size of a structure's surface
 * @returns size
 */
size_t __r_get_structure_size(Receptor *r,Structure s,void *surface) {
    return _d_get_structure_size(r->defs.symbols,r->defs.structures,s,surface);
}
/**
 * get the size of a symbol's surface
 * @returns size
 */
size_t __r_get_symbol_size(Receptor *r,Symbol s,void *surface) {
    return _d_get_symbol_size(r->defs.symbols,r->defs.structures,s,surface);
}

/**
 * Walks the definition of a symbol to build a semtrex that would match that definiton
 *
 * @param[in] r the receptor context in which things are defined
 * @param[in] s the symbol to build a semtrex for
 * @returns the completed semtrex
 */
T * _r_build_def_semtrex(Receptor *r,Symbol s) {
    return _d_build_def_semtrex(r->defs,s,0);
}

/**
 * Determine whether a tree matches a symbol definition, both structural and semantic
 *
 * @param[in] r the receptor context in which things are defined
 * @param[in] s the symbol we expect this tree to be
 * @param[in] t the tree to match
 * @returns true or false depending on the match
 *
 * @todo currently this just matches on a semtrex.  It should also look at the surface
 sizes to see if they meet the criteria of the structure definitions.
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/receptor_spec.h testReceptorDefMatch
 */
int _r_def_match(Receptor *r,Symbol s,T *t) {
    T *stx = _r_build_def_semtrex(r,s);
    int result = _t_match(stx,t);
    _t_free(stx);
    return result;
}

/**
 * Setup listeners on an aspect for a given sequence in a protocol
 *
 *
 * @param[in] r the receptor
 * @param[in] idx the index of the protocol in the definition tree
 * @param[in] sequence in the protocol to express (i.e. activate by adding to flux's listeners)
 * @param[in] aspect the aspect on which to install listeners for this protocol
 * @param[in] handler an action to run as the sequence's endpoint (if any)
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/receptor_spec.h testReceptorProtocol
 */
void _r_express_protocol(Receptor *r,int idx,Symbol sequence,Aspect aspect,T* handler) {
    T *p = _t_child(r->defs.protocols,idx);
    T *sequences = _t_child(p,2);
    int j,c = _t_children(sequences);
    for(j=1;j<=c;j++) {
        T *seq = _t_child(sequences,j);
        if (semeq(sequence,_t_symbol(seq))) {
            // found the sequence, now get the first step
            Symbol step1 = *(Symbol *)_t_surface(_t_child(seq,1));
            // and search for the step definition in the list of steps
            T *steps = _t_child(p,1);
            c = _t_children(steps);
            for(j=1;j<=c;j++) {
                T *step = _t_child(steps,j);
                if (semeq(_t_symbol(step),step1)) {
                    T *expect = _t_clone(_t_child(step,1));
                    T *params = _t_clone(_t_child(step,2));
                    T *act = _t_child(step,3);
                    // if there is no action, then assume its the sequence endpoint
                    // and use the handler in its place
                    /// @todo revisit the assumption about handlers and endpoints
                    if (act)
                        act = _t_clone(act);
                    else
                        act = handler;
                    //@todo turns out we don't use the carrier for anything yet, so
                    // we can just set it to a NULL_SYMBOL.  This will have to change
                    // once we actually get carriers figured out
                    _r_add_listener(r,aspect,NULL_SYMBOL,expect,params,act);
                    return;
                }
            }
            raise_error("step not found:%s",_r_get_symbol_name(r,step1));
        }
    }
    raise_error("sequence not found:%s",_r_get_symbol_name(r,sequence));
}

/*****************  receptor instances and xaddrs */

/**
 * Create a new instance of a tree
 *
 * @param[in] r the receptor context in which things are defined
 * @param[in] t the tree to instantiate
 * @returns xaddr of the instance
 *
 * @todo currently stores instances in a hash of hashes, this will later
 * be handled by interacting with the data-engine.
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/receptor_spec.h testReceptorInstanceNew
 */
Xaddr _r_new_instance(Receptor *r,T *t) {
    return _a_new_instance(&r->instances,t);
}

/**
 * retrieve the instance for a given xaddr
 *
 * @param[in] r the receptor context in which things are defined
 * @param[in] x the xaddr of the instance
 * @returns the instance tree
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/receptor_spec.h testReceptorInstanceNew
 */
T * _r_get_instance(Receptor *r,Xaddr x) {
    return _a_get_instance(&r->instances,x);
}

/**
 * get the hash of a tree by Xaddr
 */
TreeHash _r_hash(Receptor *r,Xaddr t) {
    return _t_hash(r->defs.symbols,r->defs.structures,_r_get_instance(r,t));
}

/******************  receptor serialization */

/**
 * Serialize a receptor
 *
 * Allocates a buffer for and serializes a receptor into the buffer
 *
 * @param[in] r Receptor to serialize
 * @param[inout] surfaceP pointer to a void * to hold the resulting serialized data
 * @param[inout] lengthP pointer to a size_t to hold the resulting serialized data length
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/receptor_spec.h testReceptorSerialize
 */
void _r_serialize(Receptor *r,void **surfaceP,size_t *lengthP) {
    size_t buf_size = 10000;
    *surfaceP  = malloc(buf_size);
    *lengthP = __t_serialize(&r->defs,r->root,surfaceP,sizeof(size_t),buf_size,0);
    *(size_t *)(*surfaceP) = *lengthP;
}

/**
 * Unserialize a receptor
 *
 * Given a serialized receptor, return an instantiated receptor tree with label table
 *
 * @param[in] surface serialized receptor data
 * @returns Receptor
 */
Receptor * _r_unserialize(void *surface) {
    size_t length = *(size_t *)surface;
    Receptor *r = _r_new(*(Symbol *)(surface+sizeof(size_t)));
    surface += sizeof(size_t);
    T *t =  _t_unserialize(&r->defs,&surface,&length,0);
    _t_free(r->root);
    r->root = t;
    //@todo fix defs!!
    r->defs.structures = _t_child(t,1);
    r->defs.symbols = _t_child(t,2);
    r->defs.processes = _t_child(t,3);
    r->flux = _t_child(t,2);

    T *defs = _t_child(r->root,1);
    DO_KIDS(defs,__r_set_labels(r,_t_child(defs,i),i));

    return r;
}

/******************  receptor signaling */

/**
 * build a signal
 *
 * @param[in] from source Receptor Xaddr
 * @param[in] to destination Receptor Xaddr
 * @param[in] aspect Aspect over which the message will be sent
 * @param[in] signal_contents the message to be sent, which will be wrapped in a SIGNAL
 * @todo signal should have timestamps and other meta info
 */
T* __r_make_signal(Xaddr from,Xaddr to,Aspect aspect,T *signal_contents) {
    T *s = _t_new_root(SIGNAL);
    T *e = _t_newr(s,ENVELOPE);
    _t_new(e,RECEPTOR_XADDR,&from,sizeof(from));
    _t_new(e,RECEPTOR_XADDR,&to,sizeof(to));
    _t_newi(e,ASPECT,aspect);
    T *b = _t_newt(s,BODY,signal_contents);
    return s;
}

/**
 * low level function for matching expectations on listeners and either adding a new run tree
 * onto the current Q or reawakening the process that's been blocked waiting for the expectation
 * to match
 */
void __r_check_listener(T* processes,T *listener,T *signal,Q *q) {
    T *signal_contents = (T *)_t_surface(_t_child(signal,2));

    T *e,*m;
    e = _t_child(listener,1);
    // if we get a match, create a run tree from the action, using the match and signal as the parameters
    T *stx = _t_news(0,SEMTREX_GROUP,NULL_SYMBOL);
    _t_add(stx,_t_clone(_t_child(e,1)));
    if (_t_matchr(stx,signal_contents,&m)) {
        T *rt=0;
        T *action = _t_child(listener,3);
        if (!action) {
            raise_error("null action in listener!");
        }

        if (semeq(_t_symbol(action),EXPECT_ACT)) {
            // currently if the action is EXPECT_ACT then we assume that
            // this is actually the blocked phase of EXPECT_ACT. This could be a
            // problem if we ever wanted our action to be an EXPECT_ACT process
            // see the implementation of EXPECT_ACT in process.c @fixme
            R *context = *(R**) _t_surface(action);

            // for now we add the params to the contexts run tree
            /// @todo later this should be integrated into some kind of scoping handling
            T *params = _t_clone(_t_child(listener,2));
            _p_interpolate_from_match(params,m,signal_contents);
            _t_add(_t_child(context->run_tree,2),params);
            rt_cur_child(context->node_pointer) = RUN_TREE_NOT_EVAULATED;

            _p_unblock(q,context);
        }
        else {
            Process p = *(Process*) _t_surface(action);
            T *params = _t_clone(_t_child(listener,2));
            _p_interpolate_from_match(params,m,signal_contents);
            rt = __p_make_run_tree(processes,p,params);
            _t_free(params);
            _t_add(signal,rt);
            _p_addrt2q(q,rt);
        }

        _t_free(m);
    }
    _t_free(stx);
}

/**
 * Send a signal to a receptor on a given aspect
 *
 * This function adds the signal to the flux, runs all the listeners on the aspect to see if the
 * signal matches any expectation, and if so, builds action run-trees and adds them to receptor's
 * process queue.
 *
 * @param[in] r destination receptor
 * @param[in] signal signal to be delivered to the receptor
 * @todo for now the signal param is added directly to the flux.  Later it should probably be cloned? or there should be a parameter to choose?
 *
 * @returns Error
 * @todo figure out what kinds of errors could be returned by _r_deliver
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/receptor_spec.h testReceptorAction
 */
Error _r_deliver(Receptor *r, T *signal) {
    T *l;

    T *envelope = _t_child(signal,1);
    Aspect aspect = *(Aspect *)_t_surface(_t_child(envelope,3));

    T *as = __r_get_signals(r,aspect);

    _t_add(as,signal);
    debug(D_SIGNALS,"Delivering: %s\n",_td(r,signal));
    // walk through all the listeners on the aspect and see if any expectations match this incoming signal
    T *ls = __r_get_listeners(r,aspect);

    DO_KIDS(ls,
            l = _t_child(ls,i);
            __r_check_listener(r->defs.processes,l,signal,r->q);
            );

    return noDeliveryErr;
}

/******************  internal utilities */

T *__r_get_aspect(Receptor *r,Aspect aspect) {
    return _t_child(r->flux,aspect);
}
T *__r_get_listeners(Receptor *r,Aspect aspect) {
    return _t_child(__r_get_aspect(r,aspect),1);
}
T *__r_get_signals(Receptor *r,Aspect aspect) {
    return _t_child(__r_get_aspect(r,aspect),2);
}


/**
 * get the Receptor structure from an installed receptor
 */
Receptor * __r_get_receptor(T *installed_receptor) {
    // the receptor itself is the surface of the first child of the INSTALLED_RECEPTOR (bleah)
    if (!semeq(_t_symbol(installed_receptor),INSTALLED_RECEPTOR)) {
        raise_error("expecting an INSTALLED_RECEPTOR!");
    }
    return (Receptor *)_t_surface(_t_child(installed_receptor,1));
}

/*****************  Tree debugging utilities */

char *_r_get_symbol_name(Receptor *r,Symbol s) {
    return _d_get_symbol_name(r?r->defs.symbols:0,s);
}

char *_r_get_structure_name(Receptor *r,Structure s) {
    return _d_get_structure_name(r?r->defs.structures:0,s);
}

char *_r_get_process_name(Receptor *r,Process p) {
    return _d_get_process_name(r?r->defs.structures:0,p);
}

char __t_dump_buf[10000];

char *_td(Receptor *r,T *t) {
    if (!t) sprintf(__t_dump_buf,"<null-tree>");
    else
        __t_dump(&r->defs,t,0,__t_dump_buf);
    return __t_dump_buf;
}

/*****************  Built-in core and edge receptors */

Receptor *_r_makeStreamReaderReceptor(Symbol receptor_symbol,Symbol stream_symbol,Stream *st,Xaddr to) {
    Receptor *r = _r_new(receptor_symbol);
    Symbol line = _r_declare_symbol(r,CSTRING,"LINE"); /// @todo, should be shared symbol from compository, see #29...

    // code is something like:
    // (do (not stream eof) (send to (read_stream stream line)))

    T *t = _t_new_root(RUN_TREE);
    T *p = _t_new_root(REPLICATE);
    T *params = _t_newr(p,PARAMS);
    T *eof = _t_newr(p,STREAM_AVAILABLE);

    _t_new_stream(eof,stream_symbol,st);
    //    _t_newi(p,TEST_INT_SYMBOL,2);  // two repetitions
    T *send = _t_newr(p,SEND);

    _t_new(send,RECEPTOR_XADDR,&to,sizeof(to));

    T *s = _t_new(send,STREAM_READ,0,0);
    _t_new_stream(s,stream_symbol,st);
    _t_new(s,RESULT_SYMBOL,&line,sizeof(Symbol));
    _t_newi(send,BOOLEAN,1); // mark async

    T *c = _t_rclone(p);
    _t_add(t,c);

    _p_addrt2q(r->q,t);
    _t_free(p);
    return r;
}

Receptor *_r_makeStreamWriterReceptor(Symbol receptor_symbol,Symbol stream_symbol,Stream *st) {
    Receptor *r = _r_new(receptor_symbol);

    T *expect = _t_new_root(EXPECTATION);

    Symbol line = _r_declare_symbol(r,CSTRING,"LINE"); /// @todo, should be shared symbol from compository, see #29...
    char *stx = "/<LINE:LINE>";

    // @fixme for some reason parseSemtrex doesn't clean up after itself
    // valgrind reveals that some of the state in the FSA that match the
    // semtrex are left un freed.  So I'm doing this one manually below.
    /* T *t = parseSemtrex(&r->defs,stx); */
    /*  _t_add(expect,t); */

    T *t =_t_news(expect,SEMTREX_GROUP,line);
    T *x =_t_newr(t,SEMTREX_SYMBOL_LITERAL);
    _t_news(x,SEMTREX_SYMBOL,line);

    /* char buf[1000]; */
    /* _dump_semtrex(&r->defs,t,buf); */
    /* puts(buf); */

    x = _t_new_root(STREAM_WRITE);

    _t_new_stream(x,TEST_STREAM_SYMBOL,st);
    int pt1[] = {2,1,TREE_PATH_TERMINATOR};
    _t_new(x,PARAM_REF,pt1,sizeof(int)*4);

    T* params = _t_new_root(PARAMS);
    _t_news(params,INTERPOLATE_SYMBOL,line);

    T *input = _t_new_root(INPUT);
    T *output = _t_new_root(OUTPUT_SIGNATURE);
    Process proc = _r_code_process(r,x,"echo what you said","long desc...",input,output);
    T *act = _t_newp(0,ACTION,proc);

    _r_add_listener(r,DEFAULT_ASPECT,line,expect,params,act);

    return r;
}

Receptor *_r_makeClockReceptor() {
    Receptor *r = _r_new(CLOCK_RECEPTOR);
    return r;
}

/**
    bad implementation of the clock receptor thread (but easy):
   - wake up every second
   - build a TICK symbol based on the current time.
   - send this signal to the CLOCK_RECEPTOR
   - everything else should take care of itself because __r_check_listener which
     runs after _r_deliver will match

  @todo: a better implementation would be to analyze the semtrex expectations that have been planted
  and only wakeup when needed based on those semtrexes

 *
 * @param[in] the clock receptor
 */
void *___clock_thread(void *arg){
    Receptor *r = (Receptor*)arg;
    debug(D_CLOCK,"clock started\n");
    int err =0;
    while (r->state == Alive) {
        T *tick =__r_make_tick();
        debug(D_CLOCK,"%s\n",_td(r,tick));
        Xaddr self = {RECEPTOR_XADDR,0};  // 0 xaddr means SELF
        T *signal = __r_make_signal(self,self,DEFAULT_ASPECT,tick);
        _r_deliver(r,signal);
        sleep(1);
        /// @todo this will skip some seconds over time.... make more sophisticated
        //       with nano-sleep so that we get every second?
    }
    debug(D_CLOCK,"clock stopped\n");
    pthread_exit(&err); //@todo determine if we should use pthread_exit or just return 0
    return 0;
}

T * __r_make_tick() {
    struct tm t;
    time_t clock;
    time(&clock);
    gmtime_r(&clock, &t);
    T *tick = _t_new_root(TICK);
    T *today = _t_newr(tick,TODAY);
    T *now = _t_newr(tick,NOW);
    _t_newi(today,YEAR,t.tm_year+1900);
    _t_newi(today,MONTH,t.tm_mon);
    _t_newi(today,DAY,t.tm_mday);
    _t_newi(now,HOUR,t.tm_hour);
    _t_newi(now,MINUTE,t.tm_min);
    _t_newi(now,SECOND,t.tm_sec);
    return tick;
}

void __r_kill(Receptor *r) {
    r->state = Dead;
    /* pthread_mutex_lock(&shutdownMutex); */
    /* G_shutdown = val; */
    /* pthread_mutex_unlock(&shutdownMutex); */
}
/** @}*/
