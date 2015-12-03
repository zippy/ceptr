/**
 * @ingroup def
 *
 * @{
 * @file semtable.c
 * @brief implementation of table for coordinating meaning across a vmhost
 *
 * @copyright Copyright (C) 2013-2015, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 */

#include "semtable.h"
#include "def.h"

SemTable *_sem_new() {
    SemTable * sem= malloc(sizeof(SemTable));
    memset(sem,0,sizeof(SemTable));
}

int _sem_new_context(SemTable *sem,T *definitions) {
    if (sem->contexts >= MAX_CONTEXTS-1) raise_error("no more room in semtable");
    int idx = sem->contexts++;
    sem->stores[idx].definitions = definitions;
    return idx;
}

void _sem_free(SemTable *sem) {
    free(sem);
}

ContextStore *__sem_context(SemTable *st,Context c) {
    return &st->stores[c];
}

void _sem_free_context(SemTable *sem,Context c) {
    ContextStore *ctx = __sem_context(sem,c);
    // definition tree belong to the receptors that allocated them so
    // we never free them.
    ctx->definitions = NULL;
    if ((c+1) == sem->contexts)
        sem->contexts--;
}

// get the definitions for the semantic type of the semid
T *__sem_get_defs(SemTable *st,SemanticType semtype,Context c) {
    ContextStore *ctx = __sem_context(st,c);
    if (!ctx->definitions) raise_error("no definitions in context %d",c);
    T *defs = _t_child(ctx->definitions,semtype);
    if (!defs) raise_error("no defs for semtype %d in context %d",semtype,c);
    return defs;
}

T *__sem_get_def(SemTable *sem,SemanticType semtype,Context c,SemanticAddr i) {
    T *defs = __sem_get_defs(sem,semtype,c);
    return _t_child(defs,i);
}

char *_sem_get_name(SemTable *st,SemanticID s) {
    if (s.id == 0) {
        if (s.context == SYS_CONTEXT) {
            switch (s.semtype) {
            case SEM_TYPE_STRUCTURE: return "NULL_STRUCTURE";
            case SEM_TYPE_SYMBOL: return "NULL_SYMBOL";
            case SEM_TYPE_PROCESS: return "NULL_PROCESS";
            default: raise_error("bad semtype");
            }
        }
        else {
            raise_error("unexpected semantic NULL id!");
        }
    }
    return __d_get_sem_name(_sem_get_defs(st,s),s);
}

Structure _sem_get_symbol_structure(SemTable *st,Symbol s){
    if (!is_symbol(s)) raise_error("Bad symbol: semantic type not SEM_TYPE_SYMBOL");
    return __d_get_symbol_structure(_sem_get_defs(st,s),s);
}

/** @}*/
