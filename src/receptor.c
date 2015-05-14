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
#include "semtrex.h"
#include "process.h"
#include <stdarg.h>

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
void _r_add_listener(Receptor *r,Aspect aspect,Symbol carrier,T *expectation,T *action) {
    T *e = _t_news(0,LISTENER,carrier);
    _t_add(e,expectation);
    _t_add(e,action);
    T *a = __r_get_listeners(r,aspect);
    _t_add(a,e);
}

void instanceFree(Instance *i) {
    instance_elem *cur,*tmp;
    HASH_ITER(hh, *i, cur, tmp) {
	_t_free((*i)->instance);
	HASH_DEL(*i,cur);  /* delete; cur advances to next */
	free(cur);
    }
}

void instancesFree(Instances *i) {
    instances_elem *cur,*tmp;
    HASH_ITER(hh, *i, cur, tmp) {
	instanceFree(&(*i)->instances);
	HASH_DEL(*i,cur);  /* delete; cur advances to next */
	free(cur);
    }
}

/**
 * Destroys a receptor freeing all memory it uses.
 */
void _r_free(Receptor *r) {
    _t_free(r->root);
    lableTableFree(&r->table);
    instancesFree(&r->instances);
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
    T *def = _dv_define_structure(r->defs.structures,label,num_params,params);
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
 * Install listeners on an aspect for a given role in a protocol
 *
 * @param[in] r the receptor
 * @param[in] idx the index of the protocol in the definition tree
 * @param[in] role the name of the role to install
 * @param[in] aspect the aspect on which to install listeners for this protocol
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/receptor_spec.h testReceptorProtocol
 */
void _r_install_protocol(Receptor *r,int idx,char *role,Aspect aspect) {
    T *p = _t_child(r->defs.protocols,idx);
    //@todo check that role exists
    T *aspects = _t_child(r->root,2);
    T *a = _t_child(aspects,aspect);

    // get the aspects input carrier
    Symbol a_ic = *(Symbol *)_t_surface(_t_child(a,2));

    T *interactions = _t_child(p,2);
    int j,c = _t_children(interactions);
    for(j=1;j<=c;j++) {

	T *i = _t_child(interactions,j);
	//	raise(SIGINT);
	// the TO_ROLE indicates the expectation actions we must install
	if (!strcmp(role,(char *)_t_surface(_t_child(i,3)))) {
	    // get the protocols input_carrier
	    Symbol ic = *(Symbol *)_t_surface(_t_child(i,4));
	    if (!semeq(a_ic,ic)) {
		//		raise_error2("input carriers don't match: aspect=%s protocol=%s",_r_get_symbol_name(r,a_ic),_r_get_symbol_name(r,ic));
raise_error2("input carriers don't match: aspect=%d protocol=%d",a_ic.id,ic.id);
	    }
	    T *expect = _t_clone(_t_child(i,6));
	    T *act = _t_clone(_t_child(i,7));
	    _r_add_listener(r,aspect,ic,expect,act);
	}
    }
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
    Symbol s = _t_symbol(t);
    Instances *instances = &r->instances;
    instances_elem *e;

    HASH_FIND_INT( *instances, &s, e );
    if (!e) {
	e = malloc(sizeof(struct instances_elem));
	e->instances = NULL;
	e->s = s;
	e->last_id = 0;
	HASH_ADD_INT(*instances,s,e);
    }
    e->last_id++;
    instance_elem *i;
    i = malloc(sizeof(struct instance_elem));
    i->instance = t;
    i->addr = e->last_id;
    Instance *iP = &e->instances;
    HASH_ADD_INT(*iP,addr,i);

    Xaddr result;
    result.symbol = s;
    result.addr = e->last_id;

    return result;
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
    Instances *instances = &r->instances;
    instances_elem *e = 0;
    HASH_FIND_INT( *instances, &x.symbol, e );
    if (e) {
	instance_elem *i = 0;
	Instance *iP = &e->instances;
	HASH_FIND_INT( *iP, &x.addr, i );
	if (i) {
	    return i->instance;
	}
   }
    return 0;
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
 * Send a signal to a receptor on a given aspect
 *
 * @param[in] r destination receptor
 * @param[in] signal signal to be delivered to the receptor
 *
 * @returns signals that were caused to be sent by processing the signal (reponses or side-effects)
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/receptor_spec.h testReceptorAction
 */
T * _r_deliver(Receptor *r, T *signal) {
    T *m,*e,*l,*rt=0;
    T *signal_contents = (T *)_t_surface(_t_child(signal,2));
    T *envelope = _t_child(signal,1);
    Aspect aspect = *(Aspect *)_t_surface(_t_child(envelope,3));
    Xaddr from = *(Xaddr *)_t_surface(_t_child(envelope,1));
    Xaddr to = *(Xaddr *)_t_surface(_t_child(envelope,2));

    T *as = __r_get_signals(r,aspect);

    _t_add(as,signal);

    // walk through all the listeners on the aspect and see if any expectations match this incoming signal
    T *ls = __r_get_listeners(r,aspect);
    T *signals = _t_new_root(SIGNALS);

    DO_KIDS(ls,
	l = _t_child(ls,i);
	e = _t_child(l,1);
	// if we get a match, create a run tree from the action, using the match and signal as the parameters
	T *stx = _t_news(0,SEMTREX_GROUP,NULL_SYMBOL);
	_t_add(stx,_t_clone(_t_child(e,1)));
	if (_t_matchr(stx,signal_contents,&m)) {
	    T *action = _t_child(l,2);
	    rt = _p_make_run_tree(r->defs.processes,action,2,m,signal_contents);
	    _t_free(m);
	    _t_add(signal,rt);
	    // for now just reduce the tree in place
	    /// @todo move this to adding the runtree to the thread pool
	    int e = _p_reduce(r->defs,rt);
	    /// @todo runtime error handing!!!
	    if (e) {
		raise_error("got reduction error: %d",e);
	    }

	    /// @todo we shouldn't assume that all reductions are response signals...
	    /// but this will take coordination with the _p_reduce, to tell us which signals result
	    _t_add(signals,__r_make_signal(to,from,aspect,_t_clone(_t_child(rt,1))));

	}
	    _t_free(stx);
	    );

    //    printf("\n    signals after:"); puts(_td(r,signals));


    //    else return _t_child(_t_child(signal,3),1);
    return signals;
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

/** @}*/
