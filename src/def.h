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
 * @copyright Copyright (C) 2013-2015, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 */

#ifndef _CEPTR_DEF_H
#define _CEPTR_DEF_H

#include "tree.h"
#include <stdarg.h>

#define ST(defs,name,num,...) name = _d_define_structure(defs.symbols,defs.structures,"" #name "",RECEPTOR_CONTEXT,num,__VA_ARGS__)
#define SY(defs,name,str) name = _d_declare_symbol(defs.symbols,str,"" #name "",RECEPTOR_CONTEXT)
#define SP(defs,code,name,intention,in,out) name = _d_code_process(defs.processes,code,"" #name "",RECEPTOR_CONTEXT,intention,in,out)

int semeq(SemanticID s1,SemanticID s2);
char *_d_get_symbol_name(T *symbols,Symbol s);
char *_d_get_structure_name(T *structures,Structure s);
char *_d_get_process_name(T *processes,Process p);
void __d_validate_symbol(T *symbols,Symbol s,char *n);
T *__d_declare_symbol(T *symbols,Structure s,char *label);
Symbol _d_declare_symbol(T *symbols,Structure s,char *label,Context c);
Structure _d_define_structure(T *symbols,T *structures,char *label,Context c,int num_params,...);
T * _dv_define_structure(T *symbols,T *structures,char *label,int num_params,va_list params);
Structure _d_get_symbol_structure(T *symbols,Symbol symbol);
size_t _d_get_symbol_size(T *symbols,T *structures,Symbol s,void *surface);
size_t _d_get_structure_size(T *symbols,T *structures,Symbol s,void *surface);
T *__d_code_process(T *processes,T *code,char *name,char *intention,T *in,T *out);
Process _d_code_process(T *processes,T *code,char *name,char *intention,T *in,T *out,Context c);
T * _d_build_def_semtrex(Defs defs,Symbol s,T *parent);
size_t _sys_structure_size(int id,void *surface);

enum{NO_INDENT=0,INDENT=-1};
#define t2s(t) _t2s(0,t)
#define _t2s(d,t) __t2s(d,t,NO_INDENT)
char * __t2s(Defs *defs,T *t,int indent);
char *_indent_line(int level,char *buf);
char * __t_dump(Defs *defs,T *t,int level,char *buf);

#define _d_get_def(defs,s) _t_child(defs,(s).id);
#define _d_get_process_code(processes,p) _d_get_def(processes,p)
#define _d_get_symbol_def(symbols,s) _d_get_def(symbols,s)
#define _d_get_structure_def(structures,s) _d_get_def(structures,s)

#define spec_is_sem_equal(got, expected) spec_total++; if (semeq(expected,got)){putchar('.');} else {putchar('F');sprintf(failures[spec_failures++],"%s:%d expected %s to be %d.%d.%d but was %d.%d.%d",__FUNCTION__,__LINE__,#got,(expected).semtype,(expected).context,(expected).id,(got).semtype,(got).context,(got).id);}

void def_sys();
void sys_free();
#endif
/** @}*/
