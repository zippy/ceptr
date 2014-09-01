/**
 * @ingroup symbol
 *
 * @{
 * @file symbol.c
 * @brief implementation of symbol definition functions
 *
 * @copyright Copyright (C) 2013-2014, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 */

#include "tree.h"
#include "symbol.h"
char __s_extra_buf[10];

/**
 * get symbol's label
 *
 * @param[in] symbol_defs a symbol def tree containing symbol definitions
 * @param[in] s the Symbol to return the name for
 * @returns char * pointing to label
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/symbol_spec.h testSymbolGetName
 */
char *_s_get_symbol_name(Tnode *symbol_defs,Symbol s) {
    if (s>NULL_SYMBOL && s <_LAST_SYS_SYMBOL )
	return G_sys_symbol_names[s-NULL_SYMBOL];
    if (s>=TEST_SYMBOL && s < _LAST_TEST_SYMBOL)
	return G_test_symbol_names[s-TEST_SYMBOL];
    else if (symbol_defs) {
	Tnode *def = _t_child(symbol_defs,s);
	return (char *)_t_surface(_t_child(def,2));
    }
    sprintf(__s_extra_buf,"<unknown symbol:%d>",s);
    return __s_extra_buf;
}

/**
 * get structure's label
 *
 * @param[in] structure_defs a structre def tree containing structure definitions
 * @param[in] s the Structure to return the name for
 * @returns char * pointing to label
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/symbol_spec.h testStructureGetName
 */
char *_s_get_structure_name(Tnode *structure_defs,Structure s) {
    if (s>NULL_STRUCTURE && s <_LAST_SYS_STRUCTURE )
	return G_sys_structure_names[s-NULL_STRUCTURE];
    else if (structure_defs) {
	Tnode *def = _t_child(structure_defs,s);
	return (char *)_t_surface(def);
    }
    sprintf(__s_extra_buf,"<unknown structure:%d>",s);
    return __s_extra_buf;
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
 * @snippet spec/symbol_spec.h testDefSymbol
 */
Tnode * _s_def_symbol(Tnode *symbol_defs,Structure s,char *label){
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
 * @snippet spec/symbol_spec.h testDefStructure
 */
Tnode * _s_def_structure(Tnode *structure_defs,char *label,int num_params,...) {
    va_list params;
    va_start(params,num_params);
    Tnode *def = _vs_def_structure(structure_defs,label,num_params,params);
    va_end(params);
    return def;
}

/// va_list version of _s_def_structure
Tnode * _vs_def_structure(Tnode *structure_defs,char *label,int num_params,va_list params) {
    Tnode *def = _t_new(structure_defs,STRUCTURE_DEF,label,strlen(label)+1);
    int i;
    for(i=0;i<num_params;i++) {
	_t_newi(def,STRUCTURE_PART,va_arg(params,Symbol));
    }
    return def;
}


/** @}*/
