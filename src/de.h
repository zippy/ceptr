/**
 * @defgroup data Data Engine
 * @brief All data in ceptr is handled by our data engine, which also takes care of synchronizing between vmhost instances
 *
 * @{
 * @file de.h
 * @brief header file for data engine
 *
 * @copyright Copyright (C) 2013-2014, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 */

#ifndef _CEPTR_DE_H
#define _CEPTR_DE_H

#include "tree.h"
#include "vmhost.h"

void _de_boot();
void _de_start_vmhost();
VMHost *G_vm;

#endif
/** @}*/
