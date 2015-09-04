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

Symbol NULL_SYMBOL = {0,SEM_TYPE_SYMBOL,0};

Structure NULL_STRUCTURE = {0,SEM_TYPE_STRUCTURE,0};

Process NULL_PROCESS = {0,SEM_TYPE_PROCESS,0};

ContextStore G_contexts[_NUM_CONTEXTS];
#define _sd(s,c,t,i); s.context = c;s.semtype=t;s.id=i;

void init_defs_tree(ContextStore *cs) {
    cs->root = _t_new_root(DEFINITIONS);
    cs->defs.symbols = _t_newr(cs->root,SYMBOLS);
    cs->defs.structures = _t_newr(cs->root,STRUCTURES);
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
    _sd(STRUCTURE_PARTS,SYS_CONTEXT,SEM_TYPE_SYMBOL,STRUCTURE_PARTS_ID);
    _sd(STRUCTURE_PART,SYS_CONTEXT,SEM_TYPE_SYMBOL,STRUCTURE_PART_ID);
    _sd(SYMBOLS,SYS_CONTEXT,SEM_TYPE_SYMBOL,SYMBOLS_ID);
    _sd(PROCESSES,SYS_CONTEXT,SEM_TYPE_SYMBOL,PROCESSES_ID);
    _sd(SYMBOL_DECLARATION,SYS_CONTEXT,SEM_TYPE_SYMBOL,SYMBOL_DECLARATION_ID);
    _sd(SYMBOL_STRUCTURE,SYS_CONTEXT,SEM_TYPE_SYMBOL,SYMBOL_STRUCTURE_ID);
    _sd(SYMBOL_LABEL,SYS_CONTEXT,SEM_TYPE_SYMBOL,SYMBOL_LABEL_ID);

    // this has to happen after the _ds declarations!
    init_defs_tree(&G_contexts[SYS_CONTEXT]);
    init_defs_tree(&G_contexts[TEST_CONTEXT]);

    G_contexts[TEST_CONTEXT].symbols = malloc(NUM_TEST_SYMBOLS*sizeof(Symbol));

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
