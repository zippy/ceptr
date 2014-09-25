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

#define ST(defs,name,num,...) name = _d_define_structure(defs.structures,"" #name "",num,__VA_ARGS__)
#define SY(defs,name,str) name = _d_declare_symbol(defs.symbols,str,"" #name "")

char *_d_get_symbol_name(Tnode *symbols,Symbol s);
char *_d_get_structure_name(Tnode *structures,Structure s);
char *_d_get_process_name(Tnode *processes,Process p);
Tnode *__d_declare_symbol(Tnode *symbols,Structure s,char *label);
Symbol _d_declare_symbol(Tnode *symbols,Structure s,char *label);
Structure _d_define_structure(Tnode *structures,char *label,int num_params,...);
Tnode * _dv_define_structure(Tnode *structures,char *label,int num_params,va_list params);
Structure _d_get_symbol_structure(Tnode *symbols,Symbol symbol);
size_t _d_get_symbol_size(Tnode *symbols,Tnode *structures,Symbol s,void *surface);
size_t _d_get_structure_size(Tnode *symbols,Tnode *structures,Symbol s,void *surface);
Tnode *__d_code_process(Tnode *processes,Tnode *code,char *name,char *intention,Tnode *in,Tnode *out);
Process _d_code_process(Tnode *processes,Tnode *code,char *name,char *intention,Tnode *in,Tnode *out);
Tnode *_d_get_process_code(Tnode *processes,Process p);
Tnode * _d_build_def_semtrex(Defs defs,Symbol s,Tnode *parent);

char * __t_dump(Defs *defs,Tnode *t,int level,char *buf);

#endif
/** @}*/
