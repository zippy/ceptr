/**
 * @file spec_utils.h
 * @copyright Copyright (C) 2013-2015, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 * @ingroup tests
 */

#ifndef _CEPTR_TEST_UTILS_H
#define _CEPTR_TEST_UTILS_H

#include "../src/ceptr.h"

void wjson(SemTable *sem,T *t,char *n,int i);
void dump2json(SemTable *sem,T *t,char *n);
T *makeDelta(Symbol sym,int *path,T *t,int count);
void _visdump(SemTable *sem,T *x,int *path);
void visdump(SemTable *sem,T *x);
void _test_reduce_signals(Receptor *r);

#define startVisdump(n) G_visdump_fn = n;G_visdump_count = 1;
#define endVisdump() G_visdump_count = 0;

char *G_visdump_fn;
int G_visdump_count;

#endif
