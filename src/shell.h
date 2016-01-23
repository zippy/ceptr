/**
 * @ingroup receptor
 *
 * @{
 * @file shell.h
 * @brief command line shell receptor header files
 *
 *
 * @copyright Copyright (C) 2013-2016, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 */

#ifndef _CEPTR_SHELL_H
#define _CEPTR_SHELL_H

#include "accumulator.h"

void makeShell(VMHost *v,FILE *input, FILE *output,Receptor **irp,Receptor **orp,Stream **isp,Stream **osp);
#endif
/** @}*/
