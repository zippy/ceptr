/**
 * @defgroup def Symbol and Structure Definition
 * @brief Data has both meaningful and structural aspects, both of which must be defined.
 *
 * Structures are definitons of how data is organized; Symbols are a numeric semantic indicator (like a type) of what a given structure means.  For example an "integer"" defines a structures of bits and how to interpret them, whereas a "shoe size" is a symbol for what a use of an integer means.
 *
 * @{
 * @file def.h
 * @brief header file for symbol and structure definition functions
 *
 * @copyright Copyright (C) 2013-2014, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 */

#ifndef _CEPTR_SYMBOL_H
#define _CEPTR_SYMBOL_H

#include "tree.h"
#include <stdarg.h>

char *_d_get_symbol_name(Tnode *symbol_defs,Symbol s);
char *_d_get_structure_name(Tnode *structure_defs,Structure s);
Tnode *__d_def_symbol(Tnode *symbol_defs,Structure s,char *label);
Symbol _d_def_symbol(Tnode *symbol_defs,Structure s,char *label);
Structure _d_def_structure(Tnode *structure_defs,char *label,int num_params,...);
Tnode * _dv_def_structure(Tnode *structure_defs,char *label,int num_params,va_list params);
Structure _d_get_symbol_structure(Tnode *symbol_defs,Symbol symbol);
size_t _d_get_symbol_size(Tnode *symbols,Tnode *structures,Symbol s,void *surface);
size_t _d_get_structure_size(Tnode *symbols,Tnode *structures,Symbol s,void *surface);
char * __t_dump(Tnode *symbols,Tnode *t,int level,char *buf);

#endif
/** @}*/
