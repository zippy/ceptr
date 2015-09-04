/**
 * @file sys_defs.h
 * @brief Defines types and vars for system defined semantics
 *
 * @copyright Copyright (C) 2013-2015, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 * @ingroup def
 */

#ifndef _CEPTR_SYS_DEFS_H
#define _CEPTR_SYS_DEFS_H

#include "ceptr_types.h"

#define is_sys_symbol(s) (s.context == SYS_CONTEXT)
#define is_sys_test_symbol(s) (s.context == TEST_CONTEXT)
#define is_sys_process(s) (is_process(s) && (s.context == SYS_CONTEXT))
#define is_sys_structure(s) (is_structure(s) && (s.context == SYS_CONTEXT))

Structure NULL_SYMBOL;
Process NULL_PROCESS;
Structure NULL_STRUCTURE;

Symbol TEST_INT_SYMBOL;
Symbol TEST_INT_SYMBOL2;
Symbol TEST_STR_SYMBOL;
Symbol TEST_FLOAT_SYMBOL;
Symbol TEST_TREE_SYMBOL;
Symbol TEST_TREE_SYMBOL2;
Symbol TEST_NAME_SYMBOL;
Symbol TEST_STREAM_SYMBOL;
Symbol TEST_RECEPTOR_SYMBOL;
Symbol TEST_ALPHABETIZE_SCAPE_SYMBOL;

typedef struct ContextStore {
    T *root;
    Defs defs;
    Structure *structures;  ///< pointer for quick access to structures
    Symbol *symbols;     ///< pointer for quick access to symbols
    Process *processes;   ///< pointer for quick access to processes
} ContextStore;

ContextStore G_contexts[_NUM_CONTEXTS];

#define G_sys_root G_contexts[SYS_CONTEXT].root
#define G_sys_defs G_contexts[SYS_CONTEXT].defs

#define sT(name,num,...) name = _d_define_structure(G_sys_defs.symbols,G_sys_defs.structures,"" #name "",SYS_CONTEXT,num,__VA_ARGS__)
#define sY(name,str) name = _d_declare_symbol(G_sys_defs.symbols,G_sys_defs.structures,str,"" #name "",SYS_CONTEXT)
#define sP(name,intention,in,out) name = _d_code_process(G_sys_defs.processes,0,"" #name "",intention,in,out,SYS_CONTEXT)
#define sYt(name,str) name = _d_declare_symbol(G_sys_defs.symbols,G_sys_defs.structures,str,"" #name "",TEST_CONTEXT)

#endif
