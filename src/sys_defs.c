/**
 * @ingroup def
 *
 * @{
 * @file sys_defs.c
 * @brief definition of system semantic elements for bootstrapping
 *
 * @copyright Copyright (C) 2013-2016, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 */

#include "tree.h"
#include "def.h"
#include "receptor.h"

#include "base_defs.h"
#include <stdarg.h>

const Symbol NULL_SYMBOL = {0,SEM_TYPE_SYMBOL,0};
const Structure NULL_STRUCTURE = {0,SEM_TYPE_STRUCTURE,0};
const Process NULL_PROCESS = {0,SEM_TYPE_PROCESS,0};

#define _sd(s,c,t,i); s.context = c;s.semtype=t;s.id=i;

SemTable *G_sem;

void def_sys() {

    G_sem = _sem_new();

    // bootstrap the hard-coded definitions that we need to even be able to make
    // definitions

    _sd(DEFINITIONS,SYS_CONTEXT,SEM_TYPE_SYMBOL,DEFINITIONS_ID);
    _sd(STRUCTURES,SYS_CONTEXT,SEM_TYPE_SYMBOL,STRUCTURES_ID);
    _sd(STRUCTURE_DEFINITION,SYS_CONTEXT,SEM_TYPE_SYMBOL,STRUCTURE_DEFINITION_ID);
    _sd(STRUCTURE_LABEL,SYS_CONTEXT,SEM_TYPE_SYMBOL,STRUCTURE_LABEL_ID);
    _sd(STRUCTURE_SEQUENCE,SYS_CONTEXT,SEM_TYPE_SYMBOL,STRUCTURE_SEQUENCE_ID);
    _sd(STRUCTURE_SYMBOL,SYS_CONTEXT,SEM_TYPE_SYMBOL,STRUCTURE_SYMBOL_ID);
    _sd(STRUCTURE_OR,SYS_CONTEXT,SEM_TYPE_SYMBOL,STRUCTURE_OR_ID);
    _sd(STRUCTURE_ZERO_OR_MORE,SYS_CONTEXT,SEM_TYPE_SYMBOL,STRUCTURE_ZERO_OR_MORE_ID);
    _sd(STRUCTURE_ONE_OR_MORE,SYS_CONTEXT,SEM_TYPE_SYMBOL,STRUCTURE_ONE_OR_MORE_ID);
    _sd(STRUCTURE_ZERO_OR_ONE,SYS_CONTEXT,SEM_TYPE_SYMBOL,STRUCTURE_ZERO_OR_ONE_ID);
    _sd(SYMBOLS,SYS_CONTEXT,SEM_TYPE_SYMBOL,SYMBOLS_ID);
    _sd(PROCESSES,SYS_CONTEXT,SEM_TYPE_SYMBOL,PROCESSES_ID);
    _sd(PROTOCOLS,SYS_CONTEXT,SEM_TYPE_SYMBOL,PROTOCOLS_ID);
    _sd(RECEPTORS,SYS_CONTEXT,SEM_TYPE_SYMBOL,RECEPTORS_ID);
    _sd(SCAPES,SYS_CONTEXT,SEM_TYPE_SYMBOL,SCAPES_ID);
    _sd(SYMBOL_DEFINITION,SYS_CONTEXT,SEM_TYPE_SYMBOL,SYMBOL_DEFINITION_ID);
    _sd(SYMBOL_STRUCTURE,SYS_CONTEXT,SEM_TYPE_SYMBOL,SYMBOL_STRUCTURE_ID);
    _sd(SYMBOL_LABEL,SYS_CONTEXT,SEM_TYPE_SYMBOL,SYMBOL_LABEL_ID);
    _sd(RECEPTOR_DEFINITION,SYS_CONTEXT,SEM_TYPE_SYMBOL,RECEPTOR_DEFINITION_ID);
    _sd(RECEPTOR_LABEL,SYS_CONTEXT,SEM_TYPE_SYMBOL,RECEPTOR_LABEL_ID);

    base_contexts(G_sem);

    // this has to happen after the _sd declarations so that the basic Symbols will be valid
    base_defs(G_sem);

    _r_defineClockReceptor(G_sem);
}


void sys_free() {
    _t_free(_t_root(G_sem->stores[0].definitions));
    _sem_free(G_sem);
}

Context G_ctx;
char * G_label;

T *sT_(SemTable *sem,Symbol sym,int num_params,...){
    va_list params;
    T *set = _t_newr(0,sym);
    va_start(params,num_params);
    int i;
    for(i=0;i<num_params;i++) {
        T * t = va_arg(params,T *);
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
    va_end(params);
    return set;
}
