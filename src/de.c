/**
 * @ingroup data
 *
 * @{
 * @file de.c
 * @brief implementation file for data engine
 *
 * @copyright Copyright (C) 2013-2014, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 */

#include "de.h"
#include "semtrex.h"

VMHost *G_vm = 0;

/**
 * bootstrap the ceptr system
 *
 * starts up the vmhost and wakes up receptors that should be running in it.
 *
 * @TODO check the compository to verify our version of the vmhost
 *
 */
void _de_boot() {
    // _de_check_vm_host_version_on_the_compository();
    _de_start_vmhost();
}

// for now starting the vmhost just means creating a global instance of it.
void _de_start_vmhost() {
    G_vm = _v_new();
}

/** @}*/
