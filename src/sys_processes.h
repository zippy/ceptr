/**
 * @file sys_processes.h
 * @brief Defines hard-coded system defined processes.
 *
 * Processes are definitons of how to transform data and what
 *
 * This file defines the hard-coded (negative value) system defined processes
 *
 * @copyright Copyright (C) 2013-2015, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 * @ingroup def
 */

#ifndef _CEPTR_SYS_PROCESSES_H
#define _CEPTR_SYS_PROCESSES_H

#define is_sys_process(s) (is_process(s) && (s.context == SYS_CONTEXT))

/// System defined processes
enum SystemProcessID    {
    NOOP_ID=1,
    IF_ID,                         ///< reduce to the left or right tree conditionally
    ADD_INT_ID,
    SUB_INT_ID,
    MULT_INT_ID,
    DIV_INT_ID,
    MOD_INT_ID,
    EQ_INT_ID,
    LT_INT_ID,
    GT_INT_ID,
    LTE_INT_ID,
    GTE_INT_ID,
    CONCAT_STR_ID,
    RESPOND_ID,                   ///< respond to the initiating signal with the what ever the sub-tree reduced to
    INTERPOLATE_FROM_MATCH_ID,    ///< reduce to replaced values from the match*/
    RAISE_ID,
};

Process NULL_PROCESS;
Process NOOP;
Process IF;
Process ADD_INT;
Process SUB_INT;
Process MULT_INT;
Process DIV_INT;
Process MOD_INT;
Process EQ_INT;
Process LT_INT;
Process GT_INT;
Process LTE_INT;
Process GTE_INT;
Process CONCAT_STR;
Process RESPOND;
Process INTERPOLATE_FROM_MATCH;
Process RAISE;

#endif
