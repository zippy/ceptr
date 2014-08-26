/**
 * @copyright Copyright (C) 2013-2014, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 *
 * @ingroup receptor
 *
 * @{
 * @file process.h
 * @brief processing header files
 *
 * ceptr processing happens by instantiating code trees into run trees and reducing them
 *
 */

#ifndef _CEPTR_PROCESS_H
#define _CEPTR_PROCESS_H

#include "receptor.h"

void _p_reduce(Tnode *run_tree);
Tnode *_p_make_run_tree(Tnode *code,int num_params,...);

#endif
/** @}*/
