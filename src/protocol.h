/**
 * @ingroup receptor
 *
 * @{
 * @file protocol.h
 * @brief protocol helpers header file
 *
 *
 * @copyright Copyright (C) 2013-2016, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 */

#ifndef _CEPTR_PROTOCOL_H
#define _CEPTR_PROTOCOL_H

#include "receptor.h"

void *_o_add_label(char *label,T *def);
T *_o_make_protocol_def(SemTable *sem,Context c,char *label,...);
T *_o_bindings2sem_map(T *bindings, T *sem_map);
T *_o_unwrap(SemTable *sem,T *def,T *sem_map);
T *_o_resolve(SemTable *sem,T *def, T *bindings);
void _o_express_role(Receptor *r,Symbol protocol,Symbol role,Aspect aspect,T *action);
void _o_initiate(Receptor *r,SemanticID protocol,SemanticID interaction,T *bindings);

#endif
/** @}*/
