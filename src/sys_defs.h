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
#define is_sys_process(s) (is_process(s) && (s.context == SYS_CONTEXT))
#define is_sys_structure(s) (is_structure(s) && (s.context == SYS_CONTEXT))

Structure NULL_SYMBOL;
Process NULL_PROCESS;
Structure NULL_STRUCTURE;

typedef struct ContextStore {
    T *root;
    Defs defs;
    Structure *structures;  ///< pointer for quick access to structures
    Symbol *symbols;     ///< pointer for quick access to symbols
    Process *processes;   ///< pointer for quick access to processes
} ContextStore;

ContextStore G_contexts[_NUM_CONTEXTS];

#define sT(ctx,name,num,...) name = _d_define_structure(G_contexts[ctx].defs.symbols,G_contexts[ctx].defs.structures,"" #name "",ctx,num,__VA_ARGS__)
#define sY(ctx,name,str) name = _d_declare_symbol(G_contexts[ctx].defs.symbols,G_contexts[ctx].defs.structures,str,"" #name "",ctx)
#define sP(ctx,name,intention,...) name = _d_code_process(G_contexts[ctx].defs.processes,0,"" #name "",intention,__p_make_signature(__VA_ARGS__),ctx)

#endif
