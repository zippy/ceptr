/**
 * @defgroup vmhost Virtual Machine Host Receptor
 *
 * @brief The Virtual Machine Host runs receptors and interfaces between the outside world and the cptr world
 *
 * @{
 * @file vmhost.h
 * @brief vmhost implementation header file
 *
 */

#ifndef _CEPTR_VMHOST_H
#define _CEPTR_VMHOST_H

#include "receptor.h"


/**
 * VMHost holds all the data for an active virtual machine host
 */
struct VMHost {
    Receptor *r;         ///< Receptor data for this vm host
};
typedef struct VMHost VMHost;

/******************  create and destroy virtual machine */
VMHost * _v_new();
void _v_free(VMHost *r);

Xaddr _v_install_r(VMHost *v,Receptor *r,char *label);

#endif
/** @}*/
