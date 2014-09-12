/**
 * @ingroup def
 *
 * @{
 * @file def.c
 * @brief implementation file for symbol and structure definition functions
 * @todo refactor symbol/structure/process definition mechanisms into a general definitional form
 *
 * @copyright Copyright (C) 2013-2014, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 */

#include "tree.h"
#include "def.h"
char __d_extra_buf[10];

/**
 * get symbol's label
 *
 * @param[in] symbols a symbol def tree containing symbol definitions
 * @param[in] s the Symbol to return the name for
 * @returns char * pointing to label
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/def_spec.h testSymbolGetName
 */
char *_d_get_symbol_name(Tnode *symbols,Symbol s) {
    if (s>NULL_SYMBOL && s <_LAST_SYS_SYMBOL )
	return G_sys_symbol_names[s-NULL_SYMBOL];
    if (s>=TEST_INT_SYMBOL && s < _LAST_TEST_SYMBOL)
	return G_test_symbol_names[s-TEST_INT_SYMBOL];
    else if (symbols) {
	Tnode *def = _t_child(symbols,s);
	Tnode *l = _t_child(def,1);
	return (char *)_t_surface(_t_child(def,2));
    }
    sprintf(__d_extra_buf,"<unknown symbol:%d>",s);
    return __d_extra_buf;
}

/**
 * get structure's label
 *
 * @param[in] structures a structre def tree containing structure definitions
 * @param[in] s the Structure to return the name for
 * @returns char * pointing to label
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/def_spec.h testStructureGetName
 */
char *_d_get_structure_name(Tnode *structures,Structure s) {
    if (s>NULL_STRUCTURE && s <_LAST_SYS_STRUCTURE )
	return G_sys_structure_names[s-NULL_STRUCTURE];
    else if (structures) {
	Tnode *def = _t_child(structures,s);
	Tnode *l = _t_child(def,1);
	return (char *)_t_surface(l);
    }
    sprintf(__d_extra_buf,"<unknown structure:%d>",s);
    return __d_extra_buf;
}

/**
 * get processes's label
 *
 * @param[in] processes a process def tree containing structure definitions
 * @param[in] p the Process to return the name for
 * @returns char * pointing to label
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/def_spec.h testProcessGetName
 */
char *_d_get_process_name(Tnode *processes,Process p) {
    if (p>NULL_PROCESS && p <_LAST_SYS_PROCESS )
	return G_sys_process_names[p-NULL_PROCESS];
    else if (processes) {
	Tnode *def = _t_child(processes,p);
	Tnode *l = _t_child(def,1);
	return (char *)_t_surface(l);
    }
    sprintf(__d_extra_buf,"<unknown process:%d>",p);
    return __d_extra_buf;
}


/**
 * add a symbol definition to a symbol defs tree
 *
 * @param[in] symbols a symbol def tree containing symbol definitions
 * @param[in] s the structure type for this symbol
 * @param[in] label a c-string label for this symbol
 * @returns the new symbol def
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/def_spec.h testDefSymbol
 */
Tnode *__d_declare_symbol(Tnode *symbols,Structure s,char *label){
    Tnode *def = _t_newr(symbols,SYMBOL_DECLARATION);
    _t_newi(def,SYMBOL_STRUCTURE,s);
    _t_new(def,SYMBOL_LABEL,label,strlen(label)+1);
    return def;
}

/**
 * add a symbol definition to a symbol defs tree
 *
 * @param[in] symbols a symbol def tree containing symbol definitions
 * @param[in] s the structure type for this symbol
 * @param[in] label a c-string label for this symbol
 * @returns the new symbol
 * @todo this is not thread safe!
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/def_spec.h testDefSymbol
 */
Symbol _d_declare_symbol(Tnode *symbols,Structure s,char *label){
    __d_declare_symbol(symbols,s,label);
    return _t_children(symbols);
}

/**
 * add a structure definition to a structure defs tree
 *
 * @param[in] structures a structre def tree containing structure definitions
 * @param[in] label a c-string label for this symbol
 * @param[in] num_params number of symbols in the structure
 * @param[in] ... variable list of Symbol type symbols
 * @returns the new structure def
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/def_spec.h testDefStructure
 */
Structure _d_define_structure(Tnode *structures,char *label,int num_params,...) {
    va_list params;
    va_start(params,num_params);
    Tnode *def = _dv_define_structure(structures,label,num_params,params);
    va_end(params);
    return _t_children(structures);
}

/// va_list version of _d_define_structure
Tnode * _dv_define_structure(Tnode *structures,char *label,int num_params,va_list params) {
    Tnode *def = _t_newr(structures,STRUCTURE_DEFINITION);
    Tnode *l = _t_new(def,STRUCTURE_LABEL,label,strlen(label)+1);
    Tnode *p = _t_newr(def,STRUCTURE_PARTS);
    int i;
    for(i=0;i<num_params;i++) {
	_t_newi(p,STRUCTURE_PART,va_arg(params,Symbol));
    }
    return def;
}

/**
 * get the structure for a given symbol
 *
 * @param[in] symbols a symbol def tree containing symbol definitions
 * @param[in] s the symbol
 * @returns a Structure
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/def_spec.h testGetSymbolStructure
 */
Structure _d_get_symbol_structure(Tnode *symbols,Symbol s) {
    if (s>=NULL_SYMBOL && s <_LAST_SYS_SYMBOL) {
	return G_sys_symbol_structures[s-NULL_SYMBOL];
    }
    else if (s>=TEST_INT_SYMBOL && s < _LAST_TEST_SYMBOL)
	return G_test_symbol_structures[s-TEST_INT_SYMBOL];
    Tnode *def = _t_child(symbols,s);
    Tnode *t = _t_child(def,1); // first child of the def is the structure
    return *(Structure *)_t_surface(t);
}

/**
 * get the size of a symbol
 *
 * @param[in] symbols a symbol def tree containing symbol definitions
 * @param[in] structures a structure def tree containing structure definitions
 * @param[in] s the symbol
 * @param[in] surface the surface of the structure (may be necessary beause some structures have length info in the data)
 * @returns size of the structure
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/def_spec.h testGetSize
 */
size_t _d_get_symbol_size(Tnode *symbols,Tnode *structures,Symbol s,void *surface) {
    Structure st = _d_get_symbol_structure(symbols,s);
    return _d_get_structure_size(symbols,structures,st,surface);
}

/**
 * get the size of a structure
 *
 * @param[in] symbols a symbol def tree containing symbol definitions
 * @param[in] structures a structure def tree containing structure definitions
 * @param[in] s the structure
 * @param[in] surface the surface of the structure (may be necessary beause some structures have length info in the data)
 * @returns size of the structure
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/def_spec.h testGetSize
 */
size_t _d_get_structure_size(Tnode *symbols,Tnode *structures,Structure s,void *surface) {
    if (s>=NULL_STRUCTURE && s <_LAST_SYS_STRUCTURE) {
	switch(s) {
	case LIST:
	case TREE:
	case NULL_STRUCTURE: return 0;
	    //	case SEMTREX: return
	case SYMBOL: return sizeof(Symbol);
	case INTEGER: return sizeof(int);
	case FLOAT: return sizeof(float);
	case CSTRING: return strlen(surface)+1;
	case XADDR: return sizeof(Xaddr);
	default: raise_error2("DON'T HAVE A SIZE FOR STRUCTURE '%s' (%d)",_d_get_structure_name(structures,s),s);
	}
    }
    else {
	Tnode *structure = _t_child(structures,s);
	Tnode *parts = _t_child(structure,2);
	size_t size = 0;
	int i,c = _t_children(structure);
	for(i=1;i<=c;i++) {
	    Tnode *p = _t_child(parts,i);
	    size += _d_get_symbol_size(symbols,structures,*(Symbol *)_t_surface(p),surface +size);
	}
	return size;
    }
}

/**
 * add a new process coding to the processes tree
 *
 * @param[inout] processes a process def tree containing process codings which will be added to
 * @param[in] code the code tree for this process
 * @param[in] name the name of the process
 * @param[in] intention a description of what the process intends to do/transform
 * @param[in] in the input signature for the process
 * @param[in] out the output signature for the process
 * @returns the newly defined process
 * @todo this is not thread safe!
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/def_spec.h testCodeProcess
 */
Tnode *__d_code_process(Tnode *processes,Tnode *code,char *name,char *intention,Tnode *in,Tnode *out) {
    Tnode *def = _t_newr(processes,PROCESS_CODING);
    _t_new(def,PROCESS_NAME,name,strlen(name)+1);
    _t_new(def,PROCESS_INTENTION,intention,strlen(intention)+1);
    _t_add(def,code);
    _t_add(def,in);
    _t_add(def,out);
    return def;
}

/**
 * add a new process coding to the processes tree
 *
 * @param[inout] processes a process def tree containing process codings which will be added to
 * @param[in] code the code tree for this process
 * @param[in] name the name of the process
 * @param[in] intention a description of what the process intends to do/transform
 * @param[in] in the input signature for the process
 * @param[in] out the output signature for the process
 * @returns the process identifier
 * @todo this is not thread safe!
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/def_spec.h testCodeProcess
 */
Process _d_code_process(Tnode *processes,Tnode *code,char *name,char *intention,Tnode *in,Tnode *out) {
    __d_code_process(processes,code,name,intention,in,out);
    return _t_children(processes);
}


/*****************  Tree debugging utilities */

char * __t_dump(Tnode *symbols,Tnode *t,int level,char *buf) {
    if (!t) return "";
    Symbol s = _t_symbol(t);
    char b[255];
    char tbuf[2000];
    int i;
    char *n = _d_get_symbol_name(symbols,s);
    char *c;
    Xaddr x;
    Structure st = _d_get_symbol_structure(symbols,s);
    switch(st) {
    case CSTRING:
	sprintf(buf," (%s:%s",n,(char *)_t_surface(t));
	break;
    case INTEGER:
	sprintf(buf," (%s:%d",n,*(int *)_t_surface(t));
	break;
    case SYMBOL:
	c = _d_get_symbol_name(symbols,*(int *)_t_surface(t));
	sprintf(buf," (%s:%s",n,c?c:"<unknown>");
	break;
    case TREE_PATH:
	sprintf(buf," (%s:%s",n,_t_sprint_path((int *)_t_surface(t),b));
	break;
    case XADDR:
	x = *(Xaddr *)_t_surface(t);
	sprintf(buf," (%s:%s.%d",n,_d_get_symbol_name(symbols,x.symbol),x.addr);
	break;
    case TREE:
	if (t->context.flags == TFLAG_SURFACE_IS_TREE) {
	    c = __t_dump(symbols,(Tnode *)_t_surface(t),0,tbuf);
	    sprintf(buf," (%s:{%s}",n,c);
	    break;
	}
    case RECEPTOR:
	if (t->context.flags == TFLAG_SURFACE_IS_TREE+TFLAG_SURFACE_IS_RECEPTOR) {
	    c = __t_dump(symbols,((Receptor *)_t_surface(t))->root,0,tbuf);
	    sprintf(buf," (%s:{%s}",n,c);
	    break;
	}
    default:
	switch(s) {
	case LISTENER:
	    c = _d_get_symbol_name(symbols,*(int *)_t_surface(t));
	    sprintf(buf," (%s on %s",n,c?c:"<unknown>");
	    break;
	default:
	    if (n == 0)
		sprintf(buf," (<unknown:%d>",s);
	    else
		sprintf(buf," (%s",n);
	}
    }
    for(i=1;i<=_t_children(t);i++) __t_dump(symbols,_t_child(t,i),level+1,buf+strlen(buf));
    sprintf(buf+strlen(buf),")");
    return buf;
}

/** @}*/
