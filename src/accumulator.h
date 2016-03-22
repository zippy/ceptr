/**
 * @defgroup accumulator Accumulator
 * @brief Where the VMHost takes care of processing and thinking, the Accumulator takes care of persistence and signaling (i.e. manifestation and sensation).  It can be thought of as implementing the "body" of receptors.
 *
 * @{
 * @file accumulator.h
 * @brief header file for the accumulator
 *
 * @copyright Copyright (C) 2013-2014, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 */

#ifndef _CEPTR_ACCUMULATOR_H
#define _CEPTR_ACCUMULATOR_H

#include "tree.h"
#include "mtree.h"
#include "vmhost.h"

void _a_boot(char *dir_name);
void _a_start_vmhost();
void _a_shut_down();

Xaddr _a_new_instance(Instances *i,T *t);
T *_a_get_instance(Instances *i,Xaddr x);
void _a_get_instances(Instances *instances,Symbol s,T *t);
T *_a_set_instance(Instances *instances,Xaddr x,T *t);
void _a_delete_instance(Instances *instances,Xaddr x);
void _a_free_instances(Instances *i);

S *__a_serialize_instances(Instances *i);
void _a_serialize_instances(Instances *i,char *file);
void __a_unserialize_instances(SemTable *sem,Instances *instances,S *s);
void _a_unserialize_instances(SemTable *sem,Instances *i,char *file);

T *_a_gen_token(Instances *i,Xaddr x,T *dependency);
Xaddr _a_get_token_xaddr(Instances *i,T *token,T *dependency);
void _a_add_dependency(Instances *instances,T *token,T *dependency);
void _a_delete_dependency(Instances *instances,T *token,T *dependency);
void _a_delete_token(Instances *instances,T *token);

VMHost *G_vm;

#endif
/** @}*/
