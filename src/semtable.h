/**
 * @file semtable.h
 * @brief header file for table for coordinating meaning across a vmhost
 *
 * @copyright Copyright (C) 2013-2016, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 * @ingroup def
 */

#ifndef _CEPTR_SEMTABLE_H
#define _CEPTR_SEMTABLE_H

#include "ceptr_types.h"
#include "ceptr_error.h"

SemTable *_sem_new();
int _sem_new_context(SemTable *sem,T *definitions);
void _sem_free(SemTable *sem);
#define _sem_context(sem,s) __sem_context(sem,(s).context)
ContextStore *__sem_context(SemTable *sem,Context c);
void _sem_free_context(SemTable *sem,Context c);
#define _sem_get_defs(sem,s) __sem_get_defs(sem,(s).semtype,(s).context)
T *__sem_get_defs(SemTable *st,SemanticType semtype,Context c);
#define _sem_get_def(sem,s) __sem_get_def(sem,(s).semtype,(s).context,(s).id)
T *__sem_get_def(SemTable *sem,SemanticType semtype,Context c,SemanticAddr i);
char *_sem_get_name(SemTable *sem,SemanticID s);
T * _sem_get_label(SemTable *sem,SemanticID s,Symbol label_type);
void _sem_add_label(SemTable *sem,SemanticID s,Symbol label_type,char *label);
Structure _sem_get_symbol_structure(SemTable *sem,Symbol s);
bool __sem_get_by_label(SemTable *sem,char *label,SemanticID *s,Context ctx);
bool _sem_get_by_label(SemTable *sem,char *label,SemanticID *s);

#endif
