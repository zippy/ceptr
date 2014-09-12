/**
 * @file sys_processes.h
 * @brief Defines hard-coded system defined processes.
 *
 * Processes are definitons of how to transform data and what
 *
 * This file defines the hard-coded (negative value) system defined processes
 *
 * @copyright Copyright (C) 2013-2014, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 * @ingroup def
 */

#ifndef _CEPTR_SYS_PROCESSES_H
#define _CEPTR_SYS_PROCESSES_H

/// System defined processes
enum SystemProcess
    {
	NULL_PROCESS = -999,
	_LAST_SYS_PROCESS

    };
static char *G_sys_process_names[] = {
    "NULL_PROCESS",
};
#endif
