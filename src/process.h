/**
 * @file process.h
 * @brief ceptr processing happen by instantiating code trees into run trees and reducing them
 *
 */

#ifndef _CEPTR_PROCESS_H
#define _CEPTR_PROCESS_H

#include "receptor.h"

void _p_reduce(Tnode *run_tree);
Tnode *_p_make_run_tree(Tnode *code,int num_params,...);

#endif
