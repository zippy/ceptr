/**
 * @defgroup vmhost Virtual Machine Host Receptor
 *
 * @brief The Virtual Machine Host runs receptors and interfaces between the outside world and the cptr world
 *
 * @{
 * @file vmhost.h
 * @brief vmhost implementation header file
 *
 * @copyright Copyright (C) 2013-2015, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
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
    T *active_receptors;    ///< pointer to tree that holds all currently active receptors
    T *pending_signals;
    Scape *installed_receptors;
};
typedef struct VMHost VMHost;

/******************  create and destroy virtual machine */
VMHost * _v_new();
void _v_free(VMHost *r);

Xaddr _v_load_receptor_package(VMHost *v,T *p);
Xaddr _v_install_r(VMHost *v,Xaddr package,T *bindings,char *label);
Xaddr _v_new_receptor(VMHost *v,Symbol s, Receptor *r);
void _v_activate(VMHost *v, Xaddr x);
void __v_activate(VMHost *v, Receptor *r);
void _v_send(VMHost *v,Xaddr from,Xaddr to,Aspect aspect,T *contents);
void _v_send_signals(VMHost *v,T *signals);

void __v_deliver_signals(VMHost *v);

#endif
/** @}*/
