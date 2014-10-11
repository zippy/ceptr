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

#define ST(defs,name,num,...) name = _d_define_structure(defs.structures,"" #name "",RECEPTOR_CONTEXT,num,__VA_ARGS__)
#define SY(defs,name,str) name = _d_declare_symbol(defs.symbols,str,"" #name "",RECEPTOR_CONTEXT)
#define SP(defs,code,name,intention,in,out) name = _d_code_process(defs.processes,code,"" #name "",RECEPTOR_CONTEXT,intention,in,out)

int semeq(SemanticID s1,SemanticID s2);
char *_d_get_symbol_name(Tnode *symbols,Symbol s);
char *_d_get_structure_name(Tnode *structures,Structure s);
char *_d_get_process_name(Tnode *processes,Process p);
Tnode *__d_declare_symbol(Tnode *symbols,Structure s,char *label);
Symbol _d_declare_symbol(Tnode *symbols,Structure s,char *label,Context c);
Structure _d_define_structure(Tnode *structures,char *label,Context c,int num_params,...);
Tnode * _dv_define_structure(Tnode *structures,char *label,int num_params,va_list params);
Structure _d_get_symbol_structure(Tnode *symbols,Symbol symbol);
size_t _d_get_symbol_size(Tnode *symbols,Tnode *structures,Symbol s,void *surface);
size_t _d_get_structure_size(Tnode *symbols,Tnode *structures,Symbol s,void *surface);
Tnode *__d_code_process(Tnode *processes,Tnode *code,char *name,char *intention,Tnode *in,Tnode *out);
Process _d_code_process(Tnode *processes,Tnode *code,char *name,char *intention,Tnode *in,Tnode *out,Context c);
Tnode * _d_build_def_semtrex(Defs defs,Symbol s,Tnode *parent);

char * __t_dump(Defs *defs,Tnode *t,int level,char *buf);

#define _d_get_def(defs,s) _t_child(defs,(s).id);
#define _d_get_process_code(processes,p) _d_get_def(processes,p)
#define _d_get_symbol_def(symbols,s) _d_get_def(symbols,s)
#define _d_get_structure_def(structures,s) _d_get_def(structures,s)

#define spec_is_sem_equal(got, expected) spec_total++; if (semeq(expected,got)){putchar('.');} else {putchar('F');sprintf(failures[spec_failures++],"%s:%d expected %s to be %d.%d.%d but was %d.%d.%d",__FUNCTION__,__LINE__,#got,(expected).flags,(expected).context,(expected).id,(got).flags,(got).context,(got).id);}
#endif
/** @}*/
