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
    v->pending_signals = _t_newr(v->r->root,PENDING_SIGNALS);
    v->installed_receptors = _s_new(RECEPTOR_IDENTIFIER,RECEPTOR);
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

    // detach any active receptors which would otherwise be doubly freed
    while(_t_children(v->active_receptors) > 0) {
	_t_detach_by_idx(v->active_receptors,1);
    }
    _r_free(v->r);
    _s_free(v->installed_receptors);
    free(v);
}

/**
 * Add a receptor package into the local compository to make it available for installation and binding
 *
 * @param[in] v VMHost in which to install the receptor
 * @param[in] p receptor package
 * @returns Xaddr of the receptor package in the compository
 * @todo validate signature and checksums??
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/vmhost_spec.h testVMHostLoadReceptorPackage
 */
Xaddr _v_load_receptor_package(VMHost *v,Tnode *p) {
    Xaddr x;
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
    Tnode *id = _t_child(p,2);
    TreeHash h = _t_hash(v->r->defs.symbols,v->r->defs.structures,id);

    // make sure we aren't re-installing an already installed receptor
    Xaddr x = _s_get(v->installed_receptors,h);
    if (!(is_null_xaddr(x))) return G_null_xaddr;
    _s_add(v->installed_receptors,h,package);

    // confirm that the bindings match the manifest
    // @todo expand the manifest to allow optional binding, etc, using semtrex to do the matching instead of assuming positional matching
    if (bindings) {
	Tnode *m = _t_child(p,1);
	int c = _t_children(m);
	if (c%2) {raise_error0("manifest must have even number of children!");}
	int i;
	for(i=1;i<=c;i++) {
	    Tnode *mp = _t_child(m,i);
	    Tnode *s = _t_child(mp,2);
	    Tnode *bp = _t_child(bindings,i);
	    if (!bp) {
		raise_error("missing binding for %s",(char *)_t_surface(_t_child(mp,1)));
	    }
	    Tnode *v = _t_child(bp,2);
	    Symbol spec = *(Symbol *)_t_surface(s);
	    if (semeq(_t_symbol(v),spec)) {
		Tnode *symbols = _t_child(p,3);
		raise_error2("bindings symbol %s doesn't match spec %s",_d_get_symbol_name(symbols,_t_symbol(v)),_d_get_symbol_name(symbols,spec));
	    }
	}
    }

    Symbol s = _r_declare_symbol(v->r,RECEPTOR,label);

    Receptor *r = _r_new_receptor_from_package(s,p,bindings);
    Tnode *ir = _t_new_root(INSTALLED_RECEPTOR);
    _t_new_receptor(ir,s,r);

    x = _r_new_instance(v->r,ir);
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
 * later this will probably have to be optimized into a hash/scape for faster access
 */
void _v_activate(VMHost *v, Xaddr x) {
    Tnode *t = _r_get_instance(v->r,x);
    _t_add(v->active_receptors,t);
}

/**
 * queue a signal for processing
 *
 * first builds a SIGNAL tree, then instantiates and scapes it
 * @todo understand how it makes any sense at all to make an instance of the signal in context of the vmhost in which the content tree's symbols aren't defined!!!
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/vmhost_spec.h testVMHostActivateReceptor
 */
Xaddr _v_send(VMHost *v,Xaddr to,Xaddr from,Aspect aspect,Tnode *contents) {
    Tnode *s = __r_make_signal(from,to,aspect,contents);
    Xaddr xs = _r_new_instance(v->r,s);

    _t_add(v->pending_signals,s);
    return xs;
}

/**
 * walk through the list of pending signals and deliver them
 */
void _v_process_signals(VMHost *v) {
    Tnode *signals = v->pending_signals;
    while(_t_children(signals)>0) {
	Tnode *s = _t_detach_by_idx(signals,1);
	Tnode *envelope = _t_child(s,1);
	//	Tnode *contents = _t_child(s,2);
	Xaddr to = *(Xaddr *)_t_surface(_t_child(envelope,2));
	Receptor *r = (Receptor *)_t_surface(_t_child(_r_get_instance(v->r,to),1)); // the receptor itself is the surface of the first child of the INSTALLED_RECEPTOR (bleah)
	Aspect a = *(Aspect *)_t_surface(_t_child(envelope,3));
	Tnode *result = _r_deliver(r,s);
	//@todo handle results
    }
}
/** @}*/
