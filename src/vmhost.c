/**
  * @ingroup vmhost
 *
 * @{
 * @file vmhost.c
 * @brief virtual machine host receptor implementation
 *
 * @copyright Copyright (C) 2013-2014, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 */

#include "vmhost.h"
#include "tree.h"
/******************  create and destroy virtual machine */

/**
 * @brief Creates a new virtual machine host
 *
 * allocates all the memory needed in the heap
 *
 * @returns pointer to a newly allocated VMHost

 * <b>Examples (from test suite):</b>
 * @snippet spec/vmhost_spec.h testVMHostCreate
 */
VMHost * _v_new() {
    VMHost *v = malloc(sizeof(VMHost));
    v->r = _r_new(VM_HOST_RECEPTOR);
    v->c = _r_new(COMPOSITORY);
    v->active_receptors = _t_newr(v->r->root,ACTIVE_RECEPTORS);
    _t_new_receptor(v->r->root,COMPOSITORY,v->c);
    return v;
}

/**
 * Destroys a vmhost freeing all memory it uses.
 *
 * @param[in] v the VMHost to free
 */
void _v_free(VMHost *v) {
    int c = _t_children(v->active_receptors);

    // free any active receptors
    int i;
    for(i=1;i<=c;i++) {
	Receptor *r = *(Receptor **)_t_surface(_t_child(v->active_receptors,i));
	_r_free(r);
    }
    _r_free(v->r);
    free(v);
}

/**
 * Load a receptor into the local compository to make it available for installation and binding
 *
 * @param[in] v VMHost in which to install the receptor
 * @param[in] r Receptor to add to the local compository
 * @param[in] manifest
 * @param[in] uuid package identifier
 * @returns Xaddr of the receptor package in the compository
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/vmhost_spec.h testVMHostLoadReceptorPackage
 */
Xaddr _v_load_receptor_package(VMHost *v,Tnode *manifest,Receptor *r,UUID uuid) {
    Xaddr x;
    Tnode *p = _t_new_root(RECEPTOR_PACKAGE);
    _t_new(p,RECEPTOR_IDENTIFIER,&uuid,sizeof(uuid));
    _t_add(p,manifest);
    _t_new_receptor(p,PACKAGED_RECEPTOR,r);
    x = _r_new_instance(v->c,p);
    return x;
}

/**
 * install a receptor into vmhost, creating a symbol for it
 *
 * @param[in] v VMHost in which to install the receptor
 * @param[in] package xaddr of package to install
 * @param[in] bindings completed manifest which specifies how the receptor will be installed
 * @param[in] label label to be used for the semantic name for this receptor
 * @returns Xaddr of the instance
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/vmhost_spec.h testVMHostInstallReceptor
 */
Xaddr _v_install_r(VMHost *v,Xaddr package,Tnode *bindings,char *label) {
    Tnode *p = _r_get_instance(v->c,package);
    Symbol r_sym = _r_def_symbol(v->r,BOUND_RECEPTOR,label);

    Tnode *r = _t_new_root(r_sym);
    _t_add(r,bindings);
    //    _t_
    _t_add(p,r);

    Xaddr x = _r_new_instance(v->r,r);
    return x;
}

/**
 * Active a receptor
 *
 * unserializes the receptor from the RECEPTOR_PACKAGE and installs it into the
 * active receptors list
 *
 * @param[in] v VMHost
 * @param[in] x Xaddr of receptor to activate
 *
 * @todo for now we are just storing the active receptors in the receptor tree
 * later this will probably have to be optimized into a hash for faster access
 */
void _v_activate(VMHost *v, Xaddr x) {
    //    Tnode *bound_receptor = _r_get_instance(v->r,x);
    //   Receptor *r = _t__t_surface(r);
    //   _t_new_receptor(v->active_receptors,r);
}

/** @}*/
