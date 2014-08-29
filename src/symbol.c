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

char __s_extra_buf[50];

/**
 * get symbols label
 *
 * @param[in] symbol_defs a symbol def tree containing symbol definitions
 * @param[in] s the Symbol to return the name for
 * @returns char * pointing to label
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
 * add a symbol definition to a symbol defs tree
 *
 * @param[in] symbol_defs a symbol def tree containing symbol definitions
 * @param[in] s the structure type for this symbol
 * @param[in] label a c-string label for this symbol
 * @returns the new symbol def
 *
 * @snippet spec/symbol_spec.h testSymbolDef
 */
Tnode * _s_def(Tnode *symbol_defs,Structure s,char *label){
    Tnode *def = _t_newr(symbol_defs,SYMBOL_DEF);
    _t_newi(def,SYMBOL_STRUCTURE,s);
    _t_new(def,SYMBOL_LABEL,label,strlen(label)+1);
    return def;
}

/** @}*/
