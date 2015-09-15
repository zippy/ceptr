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

enum ReductionError {Ascend=-1,Descend=-2,Pushed=-3,Pop=-4,Eval=-5,Block=-6,Send=-7,SendAsync=-8,Done=0,noReductionErr=0,raiseReductionErr,tooFewParamsReductionErr=TOO_FEW_PARAMS_ERR_ID,tooManyParamsReductionErr=TOO_MANY_PARAMS_ERR_ID,signatureMismatchReductionErr=SIGNATURE_MISMATCH_ERR_ID,notProcessReductionError=NOT_A_PROCESS_ERR_ID,divideByZeroReductionErr=ZERO_DIVIDE_ERR_ID,notInSignalContextReductionError=NOT_IN_SIGNAL_CONTEXT_ERR_ID,incompatibleTypeReductionErr=INCOMPATIBLE_TYPE_ERR_ID,unixErrnoReductionErr=UNIX_ERRNO_ERR_ID};

enum QueueError {noErr = 0, contextNotFoundErr};

enum ReplicationPhase {EvalCondition,EvalBody};
enum ReplicationType {ReplicateTypeCount,ReplicateTypeUnknown,ReplicateTypeCond};

typedef struct ReplicationState {
    T *code;
    int phase;
    int count;
    int type;
} ReplicationState;

R *__p_make_context(T *run_tree,R *caller);
Error _p_step(Defs *defs, R **contextP);
Error __p_reduce_sys_proc(R *context,Symbol s,T *code,Defs *defs);
void _p_enqueue(Qe **listP,Qe *e);
Error _p_unblock(Q *q,R *context);
Error _p_reduce(Defs *defs,T *run_tree);
Q *_p_newq(Defs *defs);
void _p_freeq(Q *q);
void _p_addrt2q(Q *q,T *t);
Error _p_reduceq(Q *q);
void *_p_reduceq_thread(void *arg);
T *__p_make_run_tree(T *processes,Process p,T *params);
T *_p_make_run_tree(T *processes,T *p,int num_params,...);
T *__p_build_run_tree(T* code,int num_params,...);
void _p_cleanup(Q *q,T* receptor_state);
#endif
/** @}*/
