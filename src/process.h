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

enum ReductionError {Ascend=-1,Descend=-2,Pushed=-3,Pop=-4,Eval=-5,Done=0,noReductionErr=0,raiseReductionErr,tooFewParamsReductionErr=TOO_FEW_PARAMS_ERR_ID,tooManyParamsReductionErr=TOO_MANY_PARAMS_ERR_ID,badSignatureReductionErr=BAD_SIGNATURE_ERR_ID,notProcessReductionError=NOT_A_PROCESS_ERR_ID,divideByZeroReductionErr=ZERO_DIVIDE_ERR_ID};

typedef struct R R;
struct R {
    int err;
    int state;
    T *run_tree;
    T *node_pointer;  // pointer to the tree node to execute next
    T *parent;        // node_pointer's parent      (cached here for efficiency)
    int idx;          // node pointers child index  (cached here for efficiency)
    R *next;      // a pointer to the next context in the round robin
    R *caller;    // a pointer to the context that called this run tree
    R *callee;   // a pointer to the context we've called
};

R *__p_make_context(T *run_tree,R *caller);
Error _p_step(Defs defs, R **contextP);
Error __p_reduce_sys_proc(Defs *defs,Symbol s,T *code);
Error _p_reduce(Defs defs,T *run_tree);
Error __p_reduceR(Defs defs,T *run_tree, T *code);
Error _p_reduceR(Defs defs,T *run_tree);
T *__p_make_run_tree(T *processes,Process p,T *params);
T *_p_make_run_tree(T *processes,T *p,int num_params,...);

#endif
/** @}*/
