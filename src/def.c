/**
 * @ingroup def
 *
 * @{
 * @file def.c
 * @brief implementation file for symbol and structure definition functions
 *
 * @copyright Copyright (C) 2013-2014, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 */

#include "tree.h"
#include "def.h"
char __d_extra_buf[10];

/**
 * get symbol's label
 *
 * @param[in] symbol_defs a symbol def tree containing symbol definitions
 * @param[in] s the Symbol to return the name for
 * @returns char * pointing to label
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/def_spec.h testSymbolGetName
 */
char *_d_get_symbol_name(Tnode *symbol_defs,Symbol s) {
    if (s>NULL_SYMBOL && s <_LAST_SYS_SYMBOL )
	return G_sys_symbol_names[s-NULL_SYMBOL];
    if (s>=TEST_SYMBOL && s < _LAST_TEST_SYMBOL)
	return G_test_symbol_names[s-TEST_SYMBOL];
    else if (symbol_defs) {
	Tnode *def = _t_child(symbol_defs,s);
	Tnode *l = _t_child(def,1);
	return (char *)_t_surface(_t_child(def,2));
    }
    sprintf(__d_extra_buf,"<unknown symbol:%d>",s);
    return __d_extra_buf;
}

/**
 * get structure's label
 *
 * @param[in] structure_defs a structre def tree containing structure definitions
 * @param[in] s the Structure to return the name for
 * @returns char * pointing to label
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/def_spec.h testStructureGetName
 */
char *_d_get_structure_name(Tnode *structure_defs,Structure s) {
    if (s>NULL_STRUCTURE && s <_LAST_SYS_STRUCTURE )
	return G_sys_structure_names[s-NULL_STRUCTURE];
    else if (structure_defs) {
	Tnode *def = _t_child(structure_defs,s);
	return (char *)_t_surface(def);
    }
    sprintf(__d_extra_buf,"<unknown structure:%d>",s);
    return __d_extra_buf;
}


/**
 * add a symbol definition to a symbol defs tree
 *
 * @param[in] symbol_defs a symbol def tree containing symbol definitions
 * @param[in] s the structure type for this symbol
 * @param[in] label a c-string label for this symbol
 * @returns the new symbol def
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/def_spec.h testDefSymbol
 */
Tnode * _d_def_symbol(Tnode *symbol_defs,Structure s,char *label){
    Tnode *def = _t_newr(symbol_defs,SYMBOL_DEF);
    _t_newi(def,SYMBOL_STRUCTURE,s);
    _t_new(def,SYMBOL_LABEL,label,strlen(label)+1);
    return def;
}

/**
 * add a structure definition to a structure defs tree
 *
 * @param[in] structure_defs a structre def tree containing structure definitions
 * @param[in] label a c-string label for this symbol
 * @param[in] num_params number of symbols in the structure
 * @param[in] ... variable list of Symbol type symbols
 * @returns the new structure def
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/def_spec.h testDefStructure
 */
Tnode * _d_def_structure(Tnode *structure_defs,char *label,int num_params,...) {
    va_list params;
    va_start(params,num_params);
    Tnode *def = _dv_def_structure(structure_defs,label,num_params,params);
    va_end(params);
    return def;
}

/// va_list version of _d_def_structure
Tnode * _dv_def_structure(Tnode *structure_defs,char *label,int num_params,va_list params) {
    Tnode *def = _t_newr(structure_defs,STRUCTURE_DEF);
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
 * @param[in] symbol_defs a symbol def tree containing symbol definitions
 * @param[in] s the symbol
 * @returns a Structure
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/def_spec.h testGetSymbolStructure
 */
Structure _d_get_symbol_structure(Tnode *symbol_defs,Symbol s) {
    if (s>=NULL_SYMBOL && s <_LAST_SYS_SYMBOL) {
	return G_sys_symbol_structures[s-NULL_SYMBOL];
    }
    else if (s>=TEST_SYMBOL && s < _LAST_TEST_SYMBOL)
	return G_test_symbol_structures[s-TEST_SYMBOL];
    Tnode *def = _t_child(symbol_defs,s);
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
	case INTEGER: return sizeof(int);
	case FLOAT: return sizeof(float);
	case CSTRING: return strlen(surface)+1;
	case SERIALIZED_TREE: return *(size_t *)surface;
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

/** @}*/
