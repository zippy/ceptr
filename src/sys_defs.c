/**
 * @ingroup def
 *
 * @{
 * @file sys_defs.c
 * @brief definition of system semantic elements for bootstrapping
 *
 * @copyright Copyright (C) 2013-2015, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 */

#include "tree.h"
#include "def.h"

#include "base_defs.h"
#include <stdarg.h>

Symbol NULL_SYMBOL = {0,SEM_TYPE_SYMBOL,0};

Structure NULL_STRUCTURE = {0,SEM_TYPE_STRUCTURE,0};

Process NULL_PROCESS = {0,SEM_TYPE_PROCESS,0};

ContextStore G_contexts[_NUM_CONTEXTS];
#define _sd(s,c,t,i); s.context = c;s.semtype=t;s.id=i;

void init_defs_tree(ContextStore *cs) {
    cs->root = _t_new_root(DEFINITIONS);
    cs->defs.structures = _t_newr(cs->root,STRUCTURES);
    cs->defs.symbols = _t_newr(cs->root,SYMBOLS);
    cs->defs.processes = _t_newr(cs->root,PROCESSES);
}

void def_sys() {

    int i;
    for(i=0;i<_NUM_CONTEXTS;i++) {
        G_contexts[i].structures = 0;
        G_contexts[i].symbols = 0;
        G_contexts[i].processes = 0;
        G_contexts[i].root = 0;
    }

    G_contexts[SYS_CONTEXT].structures = malloc(NUM_SYS_STRUCTURES*sizeof(Structure));
    G_contexts[SYS_CONTEXT].symbols = malloc(NUM_SYS_SYMBOLS*sizeof(Symbol));
    G_contexts[SYS_CONTEXT].processes = malloc(NUM_SYS_PROCESSES*sizeof(Process));

    // bootstrap the hard-coded definitions that we need to even be able to make
    // definitions

    _sd(DEFINITIONS,SYS_CONTEXT,SEM_TYPE_SYMBOL,DEFINITIONS_ID);
    _sd(STRUCTURES,SYS_CONTEXT,SEM_TYPE_SYMBOL,STRUCTURES_ID);
    _sd(STRUCTURE_DEFINITION,SYS_CONTEXT,SEM_TYPE_SYMBOL,STRUCTURE_DEFINITION_ID);
    _sd(STRUCTURE_LABEL,SYS_CONTEXT,SEM_TYPE_SYMBOL,STRUCTURE_LABEL_ID);
    _sd(STRUCTURE_SEQUENCE,SYS_CONTEXT,SEM_TYPE_SYMBOL,STRUCTURE_SEQUENCE_ID);
    _sd(STRUCTURE_SYMBOL,SYS_CONTEXT,SEM_TYPE_SYMBOL,STRUCTURE_SYMBOL_ID);
    _sd(STRUCTURE_SYMBOL_SET,SYS_CONTEXT,SEM_TYPE_SYMBOL,STRUCTURE_SYMBOL_SET_ID);
    _sd(STRUCTURE_OR,SYS_CONTEXT,SEM_TYPE_SYMBOL,STRUCTURE_OR_ID);
    _sd(STRUCTURE_ZERO_OR_MORE,SYS_CONTEXT,SEM_TYPE_SYMBOL,STRUCTURE_ZERO_OR_MORE_ID);
    _sd(STRUCTURE_ONE_OR_MORE,SYS_CONTEXT,SEM_TYPE_SYMBOL,STRUCTURE_ONE_OR_MORE_ID);
    _sd(STRUCTURE_ZERO_OR_ONE,SYS_CONTEXT,SEM_TYPE_SYMBOL,STRUCTURE_ZERO_OR_ONE_ID);
    _sd(SYMBOLS,SYS_CONTEXT,SEM_TYPE_SYMBOL,SYMBOLS_ID);
    _sd(PROCESSES,SYS_CONTEXT,SEM_TYPE_SYMBOL,PROCESSES_ID);
    _sd(SYMBOL_DECLARATION,SYS_CONTEXT,SEM_TYPE_SYMBOL,SYMBOL_DECLARATION_ID);
    _sd(SYMBOL_STRUCTURE,SYS_CONTEXT,SEM_TYPE_SYMBOL,SYMBOL_STRUCTURE_ID);
    _sd(SYMBOL_LABEL,SYS_CONTEXT,SEM_TYPE_SYMBOL,SYMBOL_LABEL_ID);

    // this has to happen after the _sd declarations so that the basic Symbols will be valid
    init_defs_tree(&G_contexts[SYS_CONTEXT]);
    init_defs_tree(&G_contexts[TEST_CONTEXT]);
    init_defs_tree(&G_contexts[LOCAL_CONTEXT]);

    G_contexts[TEST_CONTEXT].symbols = malloc(NUM_TEST_SYMBOLS*sizeof(Symbol));
    G_contexts[LOCAL_CONTEXT].symbols = malloc(NUM_LOCAL_SYMBOLS*sizeof(Symbol));

    base_defs();
}
void free_context(ContextStore *cs) {
    if (cs->structures) free(cs->structures);
    if (cs->symbols) free(cs->symbols);
    if (cs->processes) free(cs->processes);
    if (cs->root) _t_free(cs->root);
}

void sys_free() {
    int i;
    for(i=0;i<_NUM_CONTEXTS;i++) {
        free_context(&G_contexts[i]);
    }
}

T *sT_(Symbol sym,int num_params,...){
    va_list params;
    T *set = _t_newr(0,sym);
    va_start(params,num_params);
    int i;
    for(i=0;i<num_params;i++) {
        T * t = va_arg(params,T *);
        // if it's a SYMBOL_SET we need to just use the symbol type and
        // can throw away the node
        if (semeq(sym,STRUCTURE_SYMBOL_SET)) {
            _t_news(set,STRUCTURE_SYMBOL,_t_symbol(t));
            _t_free(t);
        }
        // otherwise they should all be T *
        else {
            _t_add(set,t);
        }
    }
    va_end(params);
    return set;
}

Structure sTD(Context c,T *structures) {
    Structure s = {c,SEM_TYPE_STRUCTURE,_t_children(structures)};
    return s;
}
