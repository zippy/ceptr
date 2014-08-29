/**
 * @file symbol.h
 * @brief header file for symbol related functions
 *
 * @copyright Copyright (C) 2013-2014, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 * @ingroup symbol
 */

#ifndef _CEPTR_SYMBOL_H
#define _CEPTR_SYMBOL_H

#include "tree.h"
#include <stdarg.h>

char *_s_get_symbol_name(Tnode *symbol_defs,Symbol s);
char *_s_get_structure_name(Tnode *structure_defs,Structure s);
Tnode * _s_def_symbol(Tnode *symbol_defs,Structure s,char *label);
Tnode * _s_def_structure(Tnode *structure_defs,char *label,int num_params,...);
Tnode * _vs_def_structure(Tnode *structure_defs,char *label,int num_params,va_list params);

#endif
