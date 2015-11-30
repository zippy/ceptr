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
#include "receptor.h"

#include "base_defs.h"
#include <stdarg.h>

Symbol NULL_SYMBOL = {0,SEM_TYPE_SYMBOL,0};

Structure NULL_STRUCTURE = {0,SEM_TYPE_STRUCTURE,0};

Process NULL_PROCESS = {0,SEM_TYPE_PROCESS,0};

ContextStore1 G_contexts[_NUM_DEFAULT_CONTEXTS];
#define _sd(s,c,t,i); s.context = c;s.semtype=t;s.id=i;

SemTable *G_sem;

T *init_defs_tree(int context) {
    ContextStore1 *cs = &G_contexts[context];
    cs->root = __r_make_definitions();
    G_sem->stores[context].definitions = cs->root;
    return cs->root;
}

void def_sys() {

    G_sem = _sem_new();

    int i;
    for(i=0;i<_NUM_DEFAULT_CONTEXTS;i++) {
        G_contexts[i].structures = 0;
        G_contexts[i].symbols = 0;
        G_contexts[i].processes = 0;
        G_contexts[i].root = 0;
    }

    G_contexts[SYS_CONTEXT].structures = malloc(NUM_SYS_STRUCTURES*sizeof(Structure));
    G_contexts[SYS_CONTEXT].symbols = malloc(NUM_SYS_SYMBOLS*sizeof(Symbol));
    G_contexts[SYS_CONTEXT].processes = malloc(NUM_SYS_PROCESSES*sizeof(Process));

    G_contexts[TEST_CONTEXT].symbols = malloc(NUM_TEST_SYMBOLS*sizeof(Symbol));
    //    G_contexts[TEST_CONTEXT].structures = malloc(NUM_TEST_STRUCTURES*sizeof(Structure));
    G_contexts[LOCAL_CONTEXT].symbols = malloc(NUM_LOCAL_SYMBOLS*sizeof(Symbol));
    G_contexts[LOCAL_CONTEXT].structures = malloc(NUM_LOCAL_STRUCTURES*sizeof(Structure));

    for (i=0;i<NUM_SYS_SYMBOLS;i++) {
        G_contexts[SYS_CONTEXT].symbols[i].id = -1;
        G_contexts[SYS_CONTEXT].symbols[i].context = SYS_CONTEXT;
    }
    for (i=0;i<NUM_LOCAL_SYMBOLS;i++) {
        G_contexts[LOCAL_CONTEXT].symbols[i].id = -1;
        G_contexts[LOCAL_CONTEXT].symbols[i].context = LOCAL_CONTEXT;
    }
    for (i=0;i<NUM_TEST_SYMBOLS;i++) {
        G_contexts[TEST_CONTEXT].symbols[i].id = -1;
        G_contexts[TEST_CONTEXT].symbols[i].context = TEST_CONTEXT;
    }

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
    init_defs_tree(SYS_CONTEXT);
    init_defs_tree(TEST_CONTEXT);
    init_defs_tree(LOCAL_CONTEXT);
    G_sem->contexts = TEST_CONTEXT+1;

    base_sys_defs(G_sem);
    base_local_defs(G_sem);
    base_test_defs(G_sem);
}
void free_context(ContextStore1 *cs) {
    if (cs->structures) free(cs->structures);
    if (cs->symbols) free(cs->symbols);
    if (cs->processes) free(cs->processes);
    if (cs->root) _t_free(cs->root);
}

void sys_free() {
    int i;
    for(i=0;i<_NUM_DEFAULT_CONTEXTS;i++) {
        free_context(&G_contexts[i]);
    }
    _sem_free(G_sem);
}

Context G_ctx;
char * G_label;

T *sT_(SemTable *sem,Symbol sym,int num_params,...){
    va_list params;
    T *set = _t_newr(0,sym);
    va_start(params,num_params);
    int i;
    // if (strcmp("STX_SYMBOL_OR_SYMBOL_SET", G_label) == 0) {raise(SIGINT);}
    for(i=0;i<num_params;i++) {
        T * t = va_arg(params,T *);
        // if it's a SYMBOL_SET we need to just use the symbol type and
        // can throw away the node
        if (semeq(sym,STRUCTURE_SYMBOL_SET)) {
            Symbol ss = *(Symbol *)_t_surface(t);
            if (!is_symbol(ss)) raise_error("only symbols allowed in SYMBOL_SET, def of %s",G_label);
            _t_news(set,STRUCTURE_SYMBOL,ss);
            _t_free(t);
        }
        // otherwise they should all be T *
        else {
            if (semeq(_t_symbol(t),STRUCTURE_SYMBOL)) {
                Symbol ss = *(Symbol *)_t_surface(t);
                if (is_structure(ss)) {
                    T *structures = _sem_get_defs(G_sem,ss);
                    T *st = _t_child(structures,ss.id);
                    if (!st) {
                        raise_error("Structure used in %s definition is undefined!",G_label);
                    }
                    else {
                        _t_free(t);
                        t = _t_clone(_t_child(st,2));
                    }
                }
                else if (ss.id == -1) {raise_error("Symbol used in %s definition is undefined!",G_label);}
            }
            _t_add(set,t);
        }
    }
    va_end(params);
    return set;
}
