/**
 * @file receptor.c
 * @brief Receptors provide the fundamental coherence contexts for ceptr
 *
 */

#include "receptor.h"
#include "semtrex.h"
#include "process.h"
#include <stdarg.h>
/*****************  create and destroy receptors */

/**
 * Creates a new receptor allocating all memory needed.
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

/**
 * Destroys a receptor freeing all memory it uses.
 */
void _r_free(Receptor *r) {
    _t_free(r->root);
    lableTableFree(&r->table);
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

Symbol _r_def_symbol(Receptor *r,Structure s,char *label){
    Tnode *def = _t_newr(r->symbols,SYMBOL_DEF);
    _t_newi(def,SYMBOL_STRUCTURE,s);
    _t_new(def,SYMBOL_LABEL,label,strlen(label)+1);
    return __set_label_for_def(r,label,def);
}

Structure _r_def_structure(Receptor *r,char *label,int num_params,...) {
    va_list params;
    Tnode *def = _t_new(r->structures,STRUCTURE_DEF,label,strlen(label)+1);
    int i;
    va_start(params,num_params);
    for(i=0;i<num_params;i++) {
	_t_newi(def,STRUCTURE_PART,va_arg(params,Symbol));
    }
    va_end(params);

    return __set_label_for_def(r,label,def);
}

Symbol _r_get_symbol_by_label(Receptor *r,char *label) {
    return __get_label_idx(r,label);
}

Structure _r_get_structure_by_label(Receptor *r,char *label){
    return __get_label_idx(r,label);
}

Structure __r_get_symbol_structure(Receptor *r,Symbol s){
    if (s>=NULL_SYMBOL && s <_LAST_SYS_SYMBOL) {
	switch(s) {
	case SYMBOL_LABEL:
	case STRUCTURE_DEF:
	    return CSTRING;
	case LISTENER:
	case ASPECT:
	case SYMBOL_STRUCTURE:
	case STRUCTURE_PART:
	    return INTEGER;
	case INSTANCE:
	    return SURFACE;
	default: return NULL_STRUCTURE;
	}
    }
    Tnode *def = _t_child(r->symbols,s);
    Tnode *t = _t_child(def,1);
    return *(Structure *)_t_surface(t);
}

size_t __r_get_structure_size(Receptor *r,Structure s,void *surface) {
    if (s>=NULL_STRUCTURE && s <_LAST_SYS_STRUCTURE) {
	switch(s) {
	case NULL_STRUCTURE: return 0;
	    //	case SEMTREX: return
	case INTEGER: return sizeof(int);
	case FLOAT: return sizeof(float);
	case CSTRING: return strlen(surface)+1;
	default: raise_error2("DON'T HAVE A SIZE FOR STRUCTURE '%s' (%d)",_s_get_structure_name(r,s),s);
	}
    }
    else {
	Tnode *structure = _t_child(r->structures,s);
	size_t size = 0;
	int i,c = _t_children(structure);
	for(i=1;i<=c;i++) {
	    Tnode *p = _t_child(structure,i);
	    size += __r_get_symbol_size(r,*(Symbol *)_t_surface(p),surface +size);
	}
	return size;
    }
}

size_t __r_get_symbol_size(Receptor *r,Symbol s,void *surface) {
    Structure st = __r_get_symbol_structure(r,s);
    return __r_get_structure_size(r,st,surface);
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
 * @param[in] the symbol we expect this tree to be
 * @param[in] the tree to match
 * @returns true or false depending on the match
 *
 * @todo currently this just matches on a semtrex.  It should also look at the surface
 sizes to see if they meet the criteria of the structure definitions.
 */
int _r_def_match(Receptor *r,Symbol s,Tnode *t) {
    return _t_match(_r_build_def_semtrex(r,s,0),t);
}

/*****************  receptor instances and xaddrs */

/**
 * Create a new instance of a symbol.
 *
 * @todo currently this implementation of instances, just stores each instance as tree node
 of type INSTANCE as a child of the symbol.  This is way inefficient and needs to be
 replaced.  But this is just here to create the API...
*/
Xaddr _r_new_instance(Receptor *r,Symbol s,void *surface) {
    Xaddr result;
    Tnode *sym = _t_child(r->symbols,s);
    _t_new(sym,INSTANCE,surface,__r_get_symbol_size(r,s,surface));
    result.symbol = s;
    result.addr = _t_children(sym);
    return result;
}

Instance _r_get_instance(Receptor *r,Xaddr x) {
    Tnode *s = _t_child(r->symbols,x.symbol);
    Instance i;
    i.surface = _t_surface(_t_child(s,x.addr));
    return i;
}

/******************  receptor serialization */

/// macro to write data by type into *bufferP and increment offset by the size of the type
#define SWRITE(type,value) type * type##P = (type *)(*bufferP +offset); *type##P=value;offset += sizeof(type);


/**
 * Serialize a tree by recursive descent.
 *
 * @param[in] t tree to be serialized
 * @param[in] r receptor to provide a context for resolving symbols and structures
 * @param[in] bufferP a pointer to a malloced ptr of "current_size" which will be realloced if serialized tree is bigger than initial buffer allocation.
 * @param[in] offset current offset into buffer at which to put serialized data
 * @param[in] current_size size of buffer
 * @returns the length that was added to the buffer
 */
size_t __t_serialize(Receptor *r,Tnode *t,void **bufferP,size_t offset,size_t current_size) {
    size_t cl =0,l = _t_size(t);
    int i, c = _t_children(t);

    //    printf("\ncurrent_size:%ld offset:%ld  size:%ld symbol:%s",current_size,offset,l,_s_get_symbol_name(r,_t_symbol(t)));
    while ((offset+l+sizeof(Symbol)) > current_size) {
	current_size*=2;
	*bufferP = realloc(*bufferP,current_size);
    }
    Symbol s = _t_symbol(t);
    SWRITE(Symbol,s);
    SWRITE(int,c);
    if(s == INSTANCE) {
	SWRITE(size_t,l);
    }
    if (l) {
	memcpy(*bufferP+offset,_t_surface(t),l);
	offset += l;
    }

    for (i=1;i<=c;i++) {
	offset = __t_serialize(r,_t_child(t,i),bufferP,offset,current_size);
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
 */

void _r_serialize(Receptor *r,void **surfaceP,size_t *lengthP) {
    size_t buf_size = 10000;
    *surfaceP  = malloc(buf_size);
    *lengthP = __t_serialize(r,r->root,surfaceP,0,buf_size);
}

/// macro to read typed date from the surface and update length and surface values
#define SREAD(type,var_name) type var_name = *(type *)*surfaceP;*lengthP -= sizeof(type);*surfaceP += sizeof(type);
/// macro to read typed date from the surface and update length and surface values (assumes variable has already been declared)
#define _SREAD(type,var_name) var_name = *(type *)*surfaceP;*lengthP -= sizeof(type);*surfaceP += sizeof(type);

Tnode * _t_unserialize(Receptor *r,void **surfaceP,size_t *lengthP,Tnode *t) {
    size_t size;

    SREAD(Symbol,s);
    //    printf("\nSymbol:%s",_s_get_symbol_name(r,s));

    SREAD(int,c);

    if (s == INSTANCE) {
	_SREAD(size_t,size);
    }
    else size = __r_get_symbol_size(r,s,*surfaceP);
    //printf(" reading: %ld bytes",size);
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

Receptor * _r_unserialize(void *surface,size_t length) {
    Receptor *r = _r_new();
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
char *_s_get_symbol_name(Receptor *r,Symbol s) {
    if (s>NULL_SYMBOL && s <_LAST_SYS_SYMBOL )
	return G_sys_symbol_names[s-NULL_SYMBOL];
    if (s>=TEST_SYMBOL && s < _LAST_TEST_SYMBOL)
	return G_test_symbol_names[s-TEST_SYMBOL];
    else if (r) {
	Tnode *def = _t_child(r->symbols,s);
	return (char *)_t_surface(_t_child(def,2));
    }
    sprintf(__t_extra_buf,"<unknown symbol:%d>",s);
    return __t_extra_buf;
}

char *_s_get_structure_name(Receptor *r,Structure s) {
    if (s>NULL_STRUCTURE && s <_LAST_SYS_STRUCTURE )
	return G_sys_structure_names[s-NULL_STRUCTURE];
    else if (r) {
	Tnode *def = _t_child(r->structures,s);
	return (char *)_t_surface(def);
    }
    return "<unknown structure>";
}

char * __t_dump(Receptor *r,Tnode *t,int level,char *buf) {
    Symbol s = _t_symbol(t);
    char b[255];
    char tbuf[2000];
    int i;
    char *n = _s_get_symbol_name(r,s);
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
	c = _s_get_symbol_name(r,*(int *)_t_surface(t));
	sprintf(buf," (%s on %s",n,c?c:"<unknown>");
	break;
    case STRUCTURE_PART:
    case INTERPOLATE_SYMBOL:
    case SEMTREX_GROUP:
    case SEMTREX_MATCH:
    case SEMTREX_SYMBOL_LITERAL:
	c = _s_get_symbol_name(r,*(int *)_t_surface(t));
	sprintf(buf," (%s:%s",n,c?c:"<unknown>");
	break;
    case SYMBOL_STRUCTURE:
	c = _s_get_structure_name(r,*(int *)_t_surface(t));
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
	sn = _s_get_symbol_name(0,c);
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
	sn = _s_get_symbol_name(0,*(int *)_t_surface(s));
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
