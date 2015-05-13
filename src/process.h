/**
 * @ingroup receptor
 *
 * @{
 * @file process.h
 * @brief processing header files
 *
 * ceptr processing happens by instantiating code trees into run trees and reducing them
 *
 * @copyright Copyright (C) 2013-2015, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 */

#ifndef _CEPTR_PROCESS_H
#define _CEPTR_PROCESS_H

#include "tree.h"

enum ReductionError {noReductionErr,tooFewParamsReductionErr,tooManyParamsReductionErr,badSignatureReductionErr,notProcessReductionError,badParamReductionErr};

typedef struct R {
    T *node_pointer;
    T *parent;
    int idx;
} R;

void _p_init_context(T *run_tree,R *context);
Error _p_step(Defs defs,T *run_tree, R *context);
Error __p_reduce_sys_proc(Defs *defs,Symbol s,T *code);
Error _p_reduce(Defs defs,T *run_tree);
Error __p_reduceR(Defs defs,T *run_tree, T *code);
Error _p_reduceR(Defs defs,T *run_tree);
T *__p_make_run_tree(T *processes,Process p,T *params);
T *_p_make_run_tree(T *processes,T *p,int num_params,...);

#endif
/** @}*/
