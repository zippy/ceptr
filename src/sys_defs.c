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
#include <glob.h>

#include "util.h"
#include "debug.h"

const Symbol NULL_SYMBOL = {0,SEM_TYPE_SYMBOL,0};
const Structure NULL_STRUCTURE = {0,SEM_TYPE_STRUCTURE,0};
const Process NULL_PROCESS = {0,SEM_TYPE_PROCESS,0};

#define _sd(s,c,t,i); s.context = c;s.semtype=t;s.id=i;

SemTable *G_sem;

SemTable *def_sys() {

    SemTable *sem = _sem_new();

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
    _sd(STRUCTURE_STRUCTURE,SYS_CONTEXT,SEM_TYPE_SYMBOL,STRUCTURE_STRUCTURE_ID);
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
    _sd(ENGLISH_LABEL,SYS_CONTEXT,SEM_TYPE_SYMBOL,ENGLISH_LABEL_ID);

    base_contexts(sem);

    // this has to happen after the _sd declarations so that the basic Symbols will be valid
    base_defs(sem);

    _r_defineClockReceptor(sem);
    return sem;
}


void sys_free(SemTable *sem) {
    _t_free(_t_root(sem->stores[0].definitions));
    _sem_free(sem);
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
                T *structures = _sem_get_defs(sem,ss);
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

void load_contexts(SemTable *sem) {
    glob_t  paths;
    int     x;
    char    **p;

    /* Find all ".c" files in given directory*/
    x = glob("contexts/*.cptr", 0, NULL, &paths);

    if (x == 0)
    {
        Receptor *r = _r_new(sem,SYS_RECEPTOR);
        for (p=paths.gl_pathv; *p != NULL; ++p)
            load_context(*p,r);
        globfree(&paths);
        _r_free(r);
    }
}

void load_context(char *path, Receptor *parent) {

    debug(D_BOOT,"loading %s\n",path);
    // get just the file name from the path
    size_t l = strlen(path);
    int i;
    i = l-1;
    char *s = &path[i];
    while (i-- > 0 && *s != '/' ) s--;
    if (i > 0) s++;
    char name[255];
    strcpy(name,s);
    name[strlen(name)-5]=0;

    // see if the file is an existing context that we should just add to
    // or if we will use its name to create a new context
    SemTable *sem = parent->sem;
    SemanticID rsid;
    Receptor *r = 0;

    char *code_text = readFile(path,&l);
    char *start,*t,c;
    char p = 0;

    char ctx[255];

    s = code_text;
    while(l>0) {
        start = s;
        while(l>0) {
            l--; c = *s++;
            if (p == '\n' && c == '-') {
                *(s-1) = 0;
                t = ctx;
                while((c=*s++) && c != '\n') *t++ = c;
                *t = 0;
                break;
            }
            p = c;
        }
        if (ctx[0] && strcmp(ctx,name)) {
            strcpy(name,ctx);
            if (r) {_r_free(r);r = 0;};
        }
        if (!r) {
            if (_sem_get_by_label(sem,name,&rsid)) {
                if (!is_receptor(rsid)) raise_error("%s is not a receptor!",name);
            }
            else {
                T *def = _t_parse(sem,0,"(RECEPTOR_DEFINITION (RECEPTOR_LABEL %) (DEFINITIONS (STRUCTURES) (SYMBOLS) (PROCESSES) (RECEPTORS) (PROTOCOLS) (SCAPES)))",_t_new_str(0,ENGLISH_LABEL,name));
                rsid = __d_define_receptor(sem,def,parent->context);
            }
            r = _r_new(sem,rsid);
        }
        debug(D_BOOT,"executing into %s: %s\n",name,start);
        T *code = _t_parse(sem,0,start);
        Q *q = r->q;
        T *run_tree = __p_build_run_tree(code,0);
        _t_free(code);
        Qe *e = _p_addrt2q(q,run_tree);
        _p_reduceq(q);
        debug(D_BOOT,"results in: %s\n",_t2s(sem,run_tree));
    }
    if (r) _r_free(r);
}
