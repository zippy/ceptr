#include "receptor.h"
#include "semtrex.h"
#include <stdarg.h>
/*****************  create and destroy receptors */

/* creates a new receptor allocating all memory needed */
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

/* adds an expectation/action pair to a receptor's aspect */
void _r_add_listener(Receptor *r,Aspect aspect,Symbol carrier,Tnode *expectation,Tnode *action) {
    Tnode *e = _t_newi(0,LISTENER,carrier);
    _t_add(e,expectation);
    _t_add(e,action);
    Tnode *a = __r_get_listeners(r,aspect);
    _t_add(a,e);
}

/* destroys a receptor freeing all memory it uses */
void _r_free(Receptor *r) {
    _t_free(r->root);
    lableTableFree(&r->table);
    free(r);
}

/*****************  receptor symbols and structures */

// we use this for both defining symbols and structures because labels store the full path to the labeled item
int __set_label_for_def(Receptor *r,char *label,Tnode *def) {
    int *path = _t_get_path(def);
    labelSet(&r->table,label,path);
    int i = path[_t_path_depth(path)-1];
    free(path);
    return i;
}

// get the child index for a given label.  This works for retrieving symbols and structures
// because the symbol and structure values is just the child index.
int __get_label_idx(Receptor *r,char *label) {
    int *path = labelGet(&r->table,label);
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
    Tnode *def = _t_child(r->symbols,s);
    Tnode *t = _t_child(def,1);
    return *(Structure *)_t_surface(t);
}

size_t __r_get_structure_size(Receptor *r,Structure s,void *surface) {
    if (s>NULL_STRUCTURE && s <_LAST_SYS_STRUCTURE) {
	switch(s) {
	case NULL_STRUCTURE: return 0;
	    //	case SEMTREX: return
	case INTEGER: return sizeof(int);
	case FLOAT: return sizeof(float);
	case CSTRING: return strlen(surface)+1;
	default: raise_error0("FISH");
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

/*****************  receptor instances and xaddrs */
// TODO: currently this implementation of instances, just stores each instance as tree node
//       of type INSTANCE as a child of the symbol.  This is way inefficient and needs to be
//       replaced.  But this is just here to create the API...
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

/******************  receptor signaling */

// TODO: what to do if match has sibs??
void _r_interpolate_from_match(Tnode *t,Tnode *match_results,Tnode *match_tree) {
    int i,c = _t_children(t);
    if (_t_symbol(t) == INTERPOLATE_SYMBOL) {
	Symbol s = *(Symbol *)_t_surface(t);
	Tnode *m = _t_get_match(match_results,s);
	int *path = (int *)_t_surface(_t_child(m,1));
	int sibs = *(int*)_t_surface(_t_child(m,2));
	Tnode *x = _t_get(match_tree,path);

	if (!x) {
	    raise_error0("expecting to get a value from match!!");
	}
	_t_morph(t,x);
   }
    for (i=1;i<=c;i++) {
	_r_interpolate_from_match(_t_child(t,i),match_results,match_tree);
    }
}

void _r_reduce(Tnode *run_tree) {
    Tnode *code = _t_child(run_tree,1);
    if (!code) {
	raise_error0("expecting code tree as first child of run tree!");
    }
    Symbol s = _t_symbol(code);
    Tnode *params,*match_results,*match_tree;
    Tnode *x;
    switch(s) {
    case RESPOND:
	// for now we just remove the RESPOND instruction and replace it with it's own child
	x = _t_detach(code,1);
	_t_replace(run_tree,1,x);
	_r_reduce(run_tree);
	break;
    case INTERPOLATE_FROM_MATCH:
	params = _t_child(run_tree,2);
	match_results = _t_child(params,1);
	match_tree = _t_child(params,2);
	x = _t_child(code,1);
	_r_interpolate_from_match(x,match_results,match_tree);
	x = _t_detach(code,1);
	_t_replace(run_tree,1,x);
	break;
    default:
	raise_error("unknown instruction: %s",_s_get_symbol_name(0,s));
    }
}

Tnode *_r_make_run_tree(Tnode *code,int num_params,...) {
    va_list params;
    int i;

    Tnode *t = _t_new_root(RUN_TREE);
    Tnode *c = _t_clone(_t_child(code,1));
    _t_add(t,c);
    Tnode *p = _t_newr(t,PARAMS);
    va_start(params,num_params);
    for(i=0;i<num_params;i++) {
	_t_add(p,_t_clone(va_arg(params,Tnode *)));
    }
    va_end(params);
    return t;
}

/* send a signal to a receptor on a given aspect */
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
	    rt = _r_make_run_tree(_t_child(l,2),2,m,signal_contents);
	    _t_free(m);
	    _t_add(s,rt);
	    // for now just reduce the tree in place
	    // TODO: move this to adding the runtree to the thread pool
	    _r_reduce(rt);
	}
    }

    //TODO: results should actually be a what? success/failure of send
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

char *_s_get_symbol_name(Receptor *r,Symbol s) {
    if (s>NULL_SYMBOL && s <_LAST_SYS_SYMBOL )
	return G_sys_symbol_names[s-NULL_SYMBOL];
    if (s>=TEST_SYMBOL && s < _LAST_TEST_SYMBOL)
	return G_test_symbol_names[s-TEST_SYMBOL];
    else if (r) {
	Tnode *def = _t_child(r->symbols,s);
	return (char *)_t_surface(_t_child(def,2));
    }
    return "<unknown symbol>";
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
