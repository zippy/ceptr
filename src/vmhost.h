/**
 * @defgroup vmhost Virtual Machine Host Receptor
 *
 * @brief The Virtual Machine Host runs receptors and interfaces between the outside world and the cptr world
 *
 * @{
 * @file vmhost.h
 * @brief vmhost implementation header file
 *
 * @copyright Copyright (C) 2013-2014, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 */

#ifndef _CEPTR_VMHOST_H
#define _CEPTR_VMHOST_H

#include "receptor.h"


/**
 * VMHost holds all the data for an active virtual machine host
 */
struct VMHost {
    Receptor *r;                ///< Receptor data for this vm host
    Receptor *c;                ///< Receptor data for the vm host's local version of the compository
    Tnode *active_receptors;    ///< pointer to tree that holds all currently active receptors
};
typedef struct VMHost VMHost;

/******************  create and destroy virtual machine */
VMHost * _v_new();
void _v_free(VMHost *r);

Xaddr _v_load_receptor_package(VMHost *v,Tnode *p);
Xaddr _v_install_r(VMHost *v,Xaddr package,Tnode *bindings,char *label);
void _v_activate(VMHost *v, Xaddr x);

#endif
/** @}*/
