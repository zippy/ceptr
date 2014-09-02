/**
 * @ingroup receptor
 *
 * @{
 * @file receptor.c
 * @brief receptor implementation
 *
 * @copyright Copyright (C) 2013-2014, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 */

#include "receptor.h"
#include "semtrex.h"
#include "process.h"
#include <stdarg.h>
/*****************  create and destroy receptors */

/**
 * @brief Creates a new receptor
 *
 * allocates all the memory needed in the heap
 *
 * @returns pointer to a newly allocated Receptor

 * <b>Examples (from test suite):</b>
 * @snippet spec/receptor_spec.h testReceptorCreate
 */
Receptor *_r_new() {
    Receptor *r = malloc(sizeof(Receptor));
    r->root = _t_new_root(RECEPTOR);
    r->structures = _t_newr(r->root,STRUCTURES);
    r->symbols = _t_newr(r->root,SYMBOLS);
    r->flux = _t_newr(r->root,FLUX);
    Tnode *a = _t_newi(r->flux,ASPECT,DEFAULT_ASPECT);
    _t_newr(a,LISTENERS);
    _t_newr(a,SIGNALS);
    r->table = NULL;
    r->instances = NULL;
    return r;
}

/**
 * Adds an expectation/action pair to a receptor's aspect.
 */
void _r_add_listener(Receptor *r,Aspect aspect,Symbol carrier,Tnode *expectation,Tnode *action) {
    Tnode *e = _t_newi(0,LISTENER,carrier);
    _t_add(e,expectation);
    _t_add(e,action);
    Tnode *a = __r_get_listeners(r,aspect);
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

/*****************  receptor symbols and structures */

/**
 * we use this for both defining symbols and structures because labels store the full path to the labeled item
 */
int __set_label_for_def(Receptor *r,char *label,Tnode *def) {
    int *path = _t_get_path(def);
    labelSet(&r->table,label,path);
    int i = path[_t_path_depth(path)-1];
    free(path);
    return i;
}

/**
 * Get the child index for a given label.
 *
 * This works for retrieving symbols and structures because the symbol and structure values is just the child index.
 */
int __get_label_idx(Receptor *r,char *label) {
    int *path = labelGet(&r->table,label);
    if (!path) return 0;
    return path[_t_path_depth(path)-1];
}

/**
 * define a new symbol
 *
 * @param[in] symbol_defs a symbol def tree containing symbol definitions
 * @param[in] s the structure type for this symbol
 * @param[in] label a c-string label for this symbol
 * @returns the new Symbol
 *
 */
Symbol _r_def_symbol(Receptor *r,Structure s,char *label){
    Tnode *def = _d_def_symbol(r->symbols,s,label);
    return __set_label_for_def(r,label,def);
}

/**
 * define a new structure
 *
 * @param[in] symbol_defs a symbol def tree containing symbol definitions
 * @param[in] s the structure type for this symbol
 * @param[in] label a c-string label for this symbol
 * @param[in] num_params number of symbols in the structure
 * @param[in] ... variable list of Symbol type symbols
 * @returns the new Structure
 *
 */
Structure _r_def_structure(Receptor *r,char *label,int num_params,...) {
    va_list params;
    va_start(params,num_params);
    Tnode *def = _dv_def_structure(r->structures,label,num_params,params);
    va_end(params);

    return __set_label_for_def(r,label,def);
}

/**
 * find a symbol give its label
 */
Symbol _r_get_symbol_by_label(Receptor *r,char *label) {
    return __get_label_idx(r,label);
}

/**
 * find a symbol give its label
 */
Structure _r_get_structure_by_label(Receptor *r,char *label){
    return __get_label_idx(r,label);
}

/**
 * @brief find a symbol's structure
 * @returns structure id
 */
Structure __r_get_symbol_structure(Receptor *r,Symbol s){
    return _d_get_symbol_structure(r->symbols,s);
}

/**
 * get the size of a structure's surface
 * @returns size
 */
size_t __r_get_structure_size(Receptor *r,Structure s,void *surface) {
    return _d_get_structure_size(r->symbols,r->structures,s,surface);
}
/**
 * get the size of a symbol's surface
 * @returns size
 */
size_t __r_get_symbol_size(Receptor *r,Symbol s,void *surface) {
    return _d_get_symbol_size(r->symbols,r->structures,s,surface);
}

/**
 * Walks the definition of a symbol to build a semtrex that would match that definiton
 *
 * @param[in] r the receptor context in which things are defined
 * @param[in] s the symbol to build a semtrex for
 * @param[in] parent the parent semtrex node because this function calls itself recursively.  Pass in 0 to start.
 * @returns the completed semtrex
 * @todo currently this won't detect an incorrect strcture with extra children.
 This is because we don't haven't yet implemented the equivalent of "$" for semtrex.
 */
Tnode * _r_build_def_semtrex(Receptor *r,Symbol s,Tnode *parent) {
    Tnode *stx = _t_newi(parent,SEMTREX_SYMBOL_LITERAL,s);

    Structure st = __r_get_symbol_structure(r,s);
    if (st > 0) {
	Tnode *structure = _t_child(r->structures,st);
	int i,c = _t_children(structure);
	if (c > 0) {
	    Tnode *seq = _t_newr(stx,SEMTREX_SEQUENCE);
	    for(i=1;i<=c;i++) {
		Tnode *p = _t_child(structure,i);
		_r_build_def_semtrex(r,*(Symbol *)_t_surface(p),seq);
	    }
	}
    }
    return stx;
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
 */
int _r_def_match(Receptor *r,Symbol s,Tnode *t) {
    Tnode *stx = _r_build_def_semtrex(r,s,0);
    int result = _t_match(stx,t);
    _t_free(stx);
    return result;
}

/*****************  receptor instances and xaddrs */

/**
 * Create a new instance of a tree
 *
 * @param[in] r the receptor context in which things are defined
 * @param[in] t the tree to instantiate
 * @returns xaddr of the instance
 *
 * @todo currently stores instances as extra-children on the def tree, this will actually
 * be handled by interacting with the data-engine!
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/receptor_spec.h testReceptorInstanceNew
 */
Xaddr _r_new_instance(Receptor *r,Tnode *t) {
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
Tnode * _r_get_instance(Receptor *r,Xaddr x) {
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

/******************  receptor serialization */

/// macro to write data by type into *bufferP and increment offset by the size of the type
#define SWRITE(type,value) type * type##P = (type *)(*bufferP +offset); *type##P=value;offset += sizeof(type);

/**
 * Serialize a tree.
 *
 * @param[in] t tree to be serialized
 * @param[inout] surfaceP a pointer to a buffer that will be malloced
 * @param[inout] lengthP the serialized length of the tree
 */
void _t_serialize(Tnode *t,void **surfaceP, size_t *lengthP) {
    size_t buf_size = 1000;
    *surfaceP = malloc(buf_size);
    *lengthP = __t_serialize(t,surfaceP,0,buf_size,1);
    *surfaceP = realloc(*surfaceP,*lengthP);
}

/**
 * Serialize a tree by recursive descent.
 *
 * @param[in] t tree to be serialized
 * @param[in] bufferP a pointer to a malloced ptr of "current_size" which will be realloced if serialized tree is bigger than initial buffer allocation.
 * @param[in] offset current offset into buffer at which to put serialized data
 * @param[in] current_size size of buffer
 * @param[in] compact boolean to indicate whether to add in extra information
 * @returns the length that was added to the buffer
 *
 * @todo compact is really a shorthand for whether this is a fixed size tree or not
 * this should actually be determined on the fly by looking at the structure types.
 */
size_t __t_serialize(Tnode *t,void **bufferP,size_t offset,size_t current_size,int compact) {
    size_t cl =0,l = _t_size(t);
    int i, c = _t_children(t);

    //    printf("\ncurrent_size:%ld offset:%ld  size:%ld symbol:%s",current_size,offset,l,_r_get_symbol_name(r,_t_symbol(t)));
    while ((offset+l+sizeof(Symbol)) > current_size) {
	current_size*=2;
	*bufferP = realloc(*bufferP,current_size);
    }
    if (!compact) {
	Symbol s = _t_symbol(t);
	SWRITE(Symbol,s);
	SWRITE(int,c);
	if(s == INSTANCE) {
	    SWRITE(size_t,l);
	}
    }
    if (l) {
	memcpy(*bufferP+offset,_t_surface(t),l);
	offset += l;
    }

    for (i=1;i<=c;i++) {
	offset = __t_serialize(_t_child(t,i),bufferP,offset,current_size,compact);
    }
    return offset;
}

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
    *lengthP = __t_serialize(r->root,surfaceP,sizeof(size_t),buf_size,0);
    *(size_t *)(*surfaceP) = *lengthP;
}

/// macro to read typed date from the surface and update length and surface values
#define SREAD(type,var_name) type var_name = *(type *)*surfaceP;*lengthP -= sizeof(type);*surfaceP += sizeof(type);
/// macro to read typed date from the surface and update length and surface values (assumes variable has already been declared)
#define _SREAD(type,var_name) var_name = *(type *)*surfaceP;*lengthP -= sizeof(type);*surfaceP += sizeof(type);

Tnode * _t_unserialize(Receptor *r,void **surfaceP,size_t *lengthP,Tnode *t) {
    size_t size;

    SREAD(Symbol,s);
    //        printf("\nSymbol:%s",_r_get_symbol_name(r,s));

    SREAD(int,c);

    if (s == INSTANCE) {
	_SREAD(size_t,size);
    }
    else size = __r_get_symbol_size(r,s,*surfaceP);
    //printf(" reading: %ld bytes\n",size);
    Structure st = __r_get_symbol_structure(r,s);
    if (st == INTEGER)
	t = _t_newi(t,s,*(int *)*surfaceP);
    else
	t = _t_new(t,s,*surfaceP,size);
    *lengthP -= size;
    *surfaceP += size;
    int i;
    for(i=1;i<=c;i++) {
	_t_unserialize(r,surfaceP,lengthP,t);
    }
    return t;
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
    Receptor *r = _r_new();
    surface += sizeof(size_t);
    Tnode *t =  _t_unserialize(r,&surface,&length,0);
    _t_free(r->root);
    r->root = t;
    r->structures = _t_child(t,1);
    r->symbols = _t_child(t,2);
    r->flux = _t_child(t,2);
    int c = _t_children(r->symbols);
    int i;
    for(i=1;i<=c;i++){
	Tnode *def = _t_child(r->symbols,i);
	Tnode *sl = _t_child(def,2);
	__set_label_for_def(r,_t_surface(sl),def);
    }
    c = _t_children(r->structures);
    for(i=1;i<=c;i++){
	Tnode *def = _t_child(r->structures,i);
	__set_label_for_def(r,_t_surface(def),def);
    }
    return r;
}

/******************  receptor signaling */

/**
 * Send a signal to a receptor on a given aspect
 */
Tnode * _r_send(Receptor *r,Receptor *from,Aspect aspect, Tnode *signal_contents) {
    Tnode *m,*e,*l,*rt=0;

    Tnode *as = __r_get_signals(r,aspect);
    Tnode *s = _t_newt(as,SIGNAL,signal_contents);

    // walk through all the listeners on the aspect and see if any expectations match this incoming signal
    Tnode *ls = __r_get_listeners(r,aspect);
    int i,c = _t_children(ls);
    for(i=1;i<=c;i++) {
	l = _t_child(ls,i);
	e = _t_child(l,1);
	// if we get a match, create a run tree from the action, using the match and signal as the parameters
	if (_t_matchr(_t_child(e,1),signal_contents,&m)) {
	    rt = _p_make_run_tree(_t_child(l,2),2,m,signal_contents);
	    _t_free(m);
	    _t_add(s,rt);
	    // for now just reduce the tree in place
	    /// @todo move this to adding the runtree to the thread pool
	    _p_reduce(rt);
	}
    }

    /// @todo  results should actually be a what? success/failure of send
    if (rt == 0) return 0;
    else return s;
}

/******************  internal utilities */

Tnode *__r_get_aspect(Receptor *r,Aspect aspect) {
    return _t_child(r->flux,aspect);
}
Tnode *__r_get_listeners(Receptor *r,Aspect aspect) {
    return _t_child(__r_get_aspect(r,aspect),1);
}
Tnode *__r_get_signals(Receptor *r,Aspect aspect) {
    return _t_child(__r_get_aspect(r,aspect),2);
}

/*****************  Tree debugging utilities */

char __t_dump_buf[10000];
char __t_extra_buf[50];
char *_r_get_symbol_name(Receptor *r,Symbol s) {
    return _d_get_symbol_name(r?r->symbols:0,s);
}

char *_r_get_structure_name(Receptor *r,Structure s) {
    return _d_get_structure_name(r?r->structures:0,s);
}

char * __t_dump(Receptor *r,Tnode *t,int level,char *buf) {
    if (!t) return "";
    Symbol s = _t_symbol(t);
    char b[255];
    char tbuf[2000];
    int i;
    char *n = _r_get_symbol_name(r,s);
    char *c;
    switch(s) {
    case TEST_STR_SYMBOL:
    case TEST_FIRST_NAME_SYMBOL:
    case STRUCTURE_DEF:
    case SYMBOL_LABEL:
	sprintf(buf," (%s:%s",n,(char *)_t_surface(t));
	break;
    case TEST_TREE_SYMBOL:
	c = __t_dump(r,(Tnode *)_t_surface(t),0,tbuf);
	sprintf(buf," (%s:{%s}",n,c);
	//	sprintf(buf," (%s:%s",n,);
	break;
    case TREE_PATH:
	sprintf(buf," (%s:%s",n,_t_sprint_path((int *)_t_surface(t),b));
	break;
    case TEST_SYMBOL:
    case SEMTREX_MATCH_SIBLINGS_COUNT:
    case ASPECT:
	sprintf(buf," (%s:%d",n,*(int *)_t_surface(t));
	break;
    case LISTENER:
	c = _r_get_symbol_name(r,*(int *)_t_surface(t));
	sprintf(buf," (%s on %s",n,c?c:"<unknown>");
	break;
    case STRUCTURE_PART:
    case INTERPOLATE_SYMBOL:
    case SEMTREX_GROUP:
    case SEMTREX_MATCH:
    case SEMTREX_SYMBOL_LITERAL:
	c = _r_get_symbol_name(r,*(int *)_t_surface(t));
	sprintf(buf," (%s:%s",n,c?c:"<unknown>");
	break;
    case SYMBOL_STRUCTURE:
	c = _r_get_structure_name(r,*(int *)_t_surface(t));
	sprintf(buf," (%s:%s",n,c?c:"<unknown>");
	break;
    default:
	if (n == 0)
	    sprintf(buf," (<unknown:%d>",s);
	else
	    sprintf(buf," (%s",n);
    }
    for(i=1;i<=_t_children(t);i++) __t_dump(r,_t_child(t,i),level+1,buf+strlen(buf));
    sprintf(buf+strlen(buf),")");
    return buf;
}

char *_td(Receptor *r,Tnode *t) {
    if (!t) sprintf(__t_dump_buf,"<null-tree>");
    else
	__t_dump(r,t,0,__t_dump_buf);
    return __t_dump_buf;
}


// semtrex dumping code
char * __dump_semtrex(Tnode *s,char *buf);

void __stxd_multi(char *x,Tnode *s,char *buf) {
    char b[1000];
    sprintf(buf,_t_children(s)>1 ? "(%s)%s" : "%s%s",__dump_semtrex(_t_child(s,1),b),x);
}
void __stxd_descend(Tnode *s,char *v,char *buf) {
    if(_t_children(s)>0) {
	char b[1000];
	Tnode *sub = _t_child(s,1);
	sprintf(buf,_t_children(sub)>0?"(%s/%s)":"(%s/%s)",v,__dump_semtrex(sub,b));
    }
    else sprintf(buf,"%s",v);
}

char * __dump_semtrex(Tnode *s,char *buf) {
    Symbol sym = _t_symbol(s);
    char b[1000];
    char *sn;
    int i,c;
    switch(sym) {
    case SEMTREX_VALUE_LITERAL:
    case SEMTREX_SYMBOL_LITERAL:
	c = *(int *)_t_surface(s);
	sn = _d_get_symbol_name(0,c);
	// ignore "<unknown symbol"
	if (*sn=='<')
	    sprintf(b,"%d",c);
	else
	    sprintf(b,"%s",sn);
	if (sym == SEMTREX_VALUE_LITERAL) {
	    sprintf(b+strlen(b),"=???");
	}
	__stxd_descend(s,b,buf);
	break;
    case SEMTREX_SYMBOL_ANY:
	sprintf(b,".");
	__stxd_descend(s,b,buf);
	break;
    case SEMTREX_SEQUENCE:
	c = _t_children(s);
	sn = buf;
	for(i=1;i<=c;i++) {
	    sprintf(sn,i<c ? "%s,":"%s",__dump_semtrex(_t_child(s,i),b));
	    sn += strlen(sn);
	}
	break;
    case SEMTREX_OR:
	sprintf(buf,"(%s)|(%s)",__dump_semtrex(_t_child(s,1),b),__dump_semtrex(_t_child(s,2),b));
	break;
    case SEMTREX_ZERO_OR_MORE:
	__stxd_multi("*",s,buf);
	break;
    case SEMTREX_ONE_OR_MORE:
	__stxd_multi("+",s,buf);
	break;
    case SEMTREX_ZERO_OR_ONE:
	__stxd_multi("?",s,buf);
	break;
    case SEMTREX_GROUP:
	sn = _d_get_symbol_name(0,*(int *)_t_surface(s));
	// ignore "<unknown symbol"
	if  (*sn=='<')
	    sprintf(buf, "{%s}",__dump_semtrex(_t_child(s,1),b));
	else
	    sprintf(buf, "{%s: %s}",sn,__dump_semtrex(_t_child(s,1),b));
	break;
    }
    return buf;
}

char * _dump_semtrex(Tnode *s,char *buf) {
    buf[0] = '/';
    __dump_semtrex(s,buf+1);
    return buf;
}
/** @}*/
