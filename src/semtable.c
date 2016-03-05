/**
 * @ingroup def
 *
 * @{
 * @file semtable.c
 * @brief implementation of table for coordinating meaning across a vmhost
 *
 * @copyright Copyright (C) 2013-2016, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
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
    //    sem->stores[idx].table = NULL;
    sem->stores[idx].definitions = definitions;
    return idx;
}

void _sem_free(SemTable *sem) {
    free(sem);
}

ContextStore *__sem_context(SemTable *sem,Context c) {
    return &sem->stores[c];
}

void _sem_free_context(SemTable *sem,Context c) {
    ContextStore *ctx = __sem_context(sem,c);
    // definition tree belong to the receptors that allocated them so
    // we never free them.
    ctx->definitions = NULL;
    //if (ctx->table) lableTableFree(ctx->table);

    if ((c+1) == sem->contexts)
        sem->contexts--;
}

char G_ctx_buf[20];

char *_sem_ctx2s(SemTable *sem,Context c) {
    switch(c) {
    case SYS_CONTEXT:return "SYS_CONTEXT";
    case COMPOSITORY_CONTEXT:return "COMPOSITORY_CONTEXT";
    case DEV_COMPOSITORY_CONTEXT:return "DEV_COMPOSITORY_CONTEXT";
    case TEST_CONTEXT:return "TEST_CONTEXT";
    default:
        sprintf(G_ctx_buf,"%d",c);
        return G_ctx_buf;
    }
}

// get the definitions for the semantic type of the semid
T *__sem_get_defs(SemTable *sem,SemanticType semtype,Context c) {
    ContextStore *ctx = __sem_context(sem,c);
    if (!ctx->definitions) raise_error("no definitions in context %s",_sem_ctx2s(sem,c));
    T *defs = _t_child(ctx->definitions,semtype);
    if (!defs) raise_error("no defs for semtype %d in context %s",semtype,_sem_ctx2s(sem,c));
    return defs;
}

T *__sem_get_def(SemTable *sem,SemanticType semtype,Context c,SemanticAddr i) {
    T *defs = __sem_get_defs(sem,semtype,c);
    return _t_child(defs,i);
}

/**
 * get symbol's label
 *
 * @param[in] sem is the semantic table where symbols and structures are define
 * @param[in] s the Symbol to return the label of
 * @returns char * pointing to surface of label node
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/semtable_spec.h testSemTableGetName
 */
char *_sem_get_name(SemTable *sem,SemanticID s) {
    if (s.id == 0) {
        if (s.context == SYS_CONTEXT) {
            switch (s.semtype) {
            case SEM_TYPE_STRUCTURE: return "NULL_STRUCTURE";
            case SEM_TYPE_SYMBOL: return "NULL_SYMBOL";
            case SEM_TYPE_PROCESS: return "NULL_PROCESS";
            case SEM_TYPE_RECEPTOR: return "SYS_RECEPTOR";
            default: raise_error("bad semtype");
            }
        }
        else {
            raise_error("unexpected semantic NULL id!");
        }
    }
    T *def = _sem_get_def(sem,s);
    char *n = NULL;
    if (def) {
        int path[] ={DefLabelIdx,1,TREE_PATH_TERMINATOR};
        T *t = _t_get(def,path);
        if (!t) raise_error("missing label!");
        n = (char *)_t_surface(t);
    }
    return n;
}


/**
 * add a name to a definition
 *
 * @param[in] sem is the semantic table where symbols and structures are defined
 * @param[in] s the semantic id of the definition to add a label to
 * @param[in] label_type is a symbol of the label type to add
 * @param[in] label text of the label
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/semtable_spec.h testSemGetByLabel
 */
void _sem_add_label(SemTable *sem,SemanticID s,Symbol label_type,char *label) {
    T *def = _sem_get_def(sem,s);
    T *labels  = _t_child(def,DefLabelIdx);
    _t_new_str(labels,label_type,label);
}

Structure _sem_get_symbol_structure(SemTable *sem,Symbol s){
    if (!is_symbol(s)) raise_error("Bad symbol: semantic type not SEM_TYPE_SYMBOL");
    return __d_get_symbol_structure(_sem_get_defs(sem,s),s);
}

// @todo, convert this to hash table label table!!
SemanticID _sem_get_by_label(SemTable *sem,char *label,Context c) {
    ContextStore *ctx = __sem_context(sem,c);
    T *d = ctx->definitions;
    if (!d) raise_error("no definitions in context %s",_sem_ctx2s(sem,c));
    int i,j;
    SemanticID sid = {c,0,0};
    for(i=1;i<=_t_children(d);i++) {
        T *defs = _t_child(d,i);
        for(j=1;j<=_t_children(defs);j++) {
            T *def = _t_child(defs,j);
            if (strcmp(label,(char *)_t_surface(_t_child(_t_child(def,DefLabelIdx),1)))==0) {
                sid.semtype = i;
                sid.id = j;
                return sid;
            }
        }
    }
    raise_error("implement what to do if label not found!");
    return sid;
}

/** @}*/
