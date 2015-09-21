/**
 * @ingroup vmhost
 *
 * @{
 * @file vmhost.c
 * @brief virtual machine host receptor implementation
 *
 * @copyright Copyright (C) 2013-2015, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 */

#include "vmhost.h"
#include "tree.h"
#include "accumulator.h"
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
    v->installed_receptors = _s_new(RECEPTOR_IDENTIFIER,RECEPTOR);
    v->vm_thread.state = 0;
    v->clock_thread.state = 0;
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
Xaddr _v_load_receptor_package(VMHost *v,T *p) {
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
Xaddr _v_install_r(VMHost *v,Xaddr package,T *bindings,char *label) {
    T *p = _r_get_instance(v->c,package);
    T *id = _t_child(p,2);
    TreeHash h = _t_hash(v->r->defs.symbols,v->r->defs.structures,id);

    // make sure we aren't re-installing an already installed receptor
    Xaddr x = _s_get(v->installed_receptors,h);
    if (!(is_null_xaddr(x))) return G_null_xaddr;
    _s_add(v->installed_receptors,h,package);

    // confirm that the bindings match the manifest
    /// @todo expand the manifest to allow optional binding, etc, using semtrex to do the matching instead of assuming positional matching
    if (bindings) {
        T *m = _t_child(p,1);
        int c = _t_children(m);
        if (c%2) {raise_error("manifest must have even number of children!");}
        int i;
        for(i=1;i<=c;i++) {
            T *mp = _t_child(m,i);
            T *s = _t_child(mp,2);
            T *bp = _t_child(bindings,i);
            if (!bp) {
                raise_error("missing binding for %s",(char *)_t_surface(_t_child(mp,1)));
            }
            T *v = _t_child(bp,2);
            Symbol spec = *(Symbol *)_t_surface(s);
            if (semeq(_t_symbol(v),spec)) {
                T *symbols = _t_child(p,3);
                raise_error("bindings symbol %s doesn't match spec %s",_d_get_symbol_name(symbols,_t_symbol(v)),_d_get_symbol_name(symbols,spec));
            }
        }
    }

    Symbol s = _r_declare_symbol(v->r,RECEPTOR,label);

    Receptor *r = _r_new_receptor_from_package(s,p,bindings);
    return _v_new_receptor(v,s,r);
}

Xaddr _v_new_receptor(VMHost *v,Symbol s, Receptor *r) {
    T *ir = _t_new_root(INSTALLED_RECEPTOR);
    _t_new_receptor(ir,s,r);

    return _r_new_instance(v->r,ir);
}

/**
 * Activate a receptor from the installed packages
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
    T *t = _r_get_instance(v->r,x);
    _t_add(v->active_receptors,t);

    // handle special cases
    T *rt = _t_child(t,1);
    if (semeq(_t_symbol(rt),CLOCK_RECEPTOR)) {
        _v_start_thread(&v->clock_thread,___clock_thread,_t_surface(rt));
    }
}

/**
 * Activate a receptor
 *
 * adds a receptor tree to the active receptors list
 *
 * @param[in] v VMHost
 * @param[in] r receptor to activate
 *
 * @todo for now we are just storing the active receptors in the receptor tree
 * later this will probably have to be optimized into a hash/scape for faster access
 */
void __v_activate(VMHost *v, Receptor *r) {
    _t_add(v->active_receptors,r->root);
}

/**
 * scaffolding for send a signal from ouside the VMhost
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/vmhost_spec.h testVMHostActivateReceptor
 */
void _v_send(VMHost *v,ReceptorAddress from,ReceptorAddress to,Aspect aspect,T *contents) {
    T *s = __r_make_signal(from,to,aspect,contents);
    T *x = __r_send_signal(v->r,s);
    _t_free(x);
}

/**
 * walk through the list of signals and send them
 */
void _v_send_signals(VMHost *v,T *signals) {
    while(_t_children(signals)>0) {
        T *s = _t_detach_by_idx(signals,1);
        T *r = __r_send_signal(v->r,s);
        _t_free(r);  //@todo WHAT????  throwing away the rsult??
    }
}

/**
 * scaffolding function for signal delviery
 */
void _v_deliver_signals(VMHost *v) {
    T *signals = v->r->q->pending_signals;
    __v_deliver_signals(v->r,signals,&v->r->instances);
}

/// @todo this should probably be implemented in a scape but for now
// we just loop through all instances searching for a match
Xaddr __v_get_receptor_xaddr(Instances *instances,Receptor *r) {
    instances_elem *e = 0;
    Xaddr result = {INSTALLED_RECEPTOR,0};
    HASH_FIND_INT( *instances, &INSTALLED_RECEPTOR, e );
    if (e) {
        Instance *iP = &e->instances;

        instance_elem *curi,*tmpi;
        HASH_ITER(hh, *iP, curi, tmpi) {
            if (__r_get_receptor(curi->instance) == r) {
                result.addr = curi->addr;
                return result;
            }
        }

    }
    return result;
}

// low level function to deliver signals in a pending signals list
// assumes that instances is the VMhost's receptor instances list
void __v_deliver_signals(Receptor *self,T *signals,Instances *receptor_instances) {

    while(_t_children(signals)>0) {
        T *s = _t_detach_by_idx(signals,1);
        T *envelope = _t_child(s,1);
        //      T *contents = _t_child(s,2);
        ReceptorAddress *toP = (ReceptorAddress *)_t_surface(_t_child(envelope,2));
        ReceptorAddress *fromP = (ReceptorAddress *)_t_surface(_t_child(envelope,1));

        // fix from if needed so return path will work
        if (*fromP == 0) {
            *fromP = __v_get_receptor_xaddr(receptor_instances,self).addr;
        }
        // if the xaddr is the "self" we do a reverse lookup to find which xaddr
        // points to the self receptor so we can fix it in the signal that we are
        // about to deliver.
        Receptor *r;
        if (*toP == 0) {
            r = self;
            *toP = __v_get_receptor_xaddr(receptor_instances,r).addr;
        }
        else  {
            Xaddr t = {INSTALLED_RECEPTOR,*toP};
            r = __r_get_receptor(_a_get_instance(receptor_instances,t));
        }

        Error err = _r_deliver(r,s);
        if (err) {
            raise_error("delivery error: %d",err);
        }
    }
}

/**
 * this is the VMhost main monitoring and execution thread
 */
void *__v_process(void *arg) {
    VMHost *v = (VMHost *) arg;
    int c,i;

    while(v->r->state == Alive) {
        // make sure everybody's doing the right thing...
        // reallocate threads as necessary...
        // do edge-receptor type stuff..
        // what ever other watchdoggy type things are necessary...
        //        printf ("something\n");
        //    sleep(1);

        // for now we will check all receptors for any active contexts and
        // we will reduce them here.  Really this should be a thread pool manager
        // where we put allocate receptor's queues for processing according to
        // priority/etc...

        c = _t_children(v->active_receptors);
        for (i=1;v->r->state == Alive && i<=c;i++) {
            /// @todo refactor being able to walk through all currently active receptors
            Receptor *r = __r_get_receptor(_t_child(v->active_receptors,i));
            if (r->q && r->q->contexts_count > 0) {
                _p_reduceq(r->q);
            }

            // send any signals generated by the reduction
            if (r->q && r->q->pending_signals && _t_children(r->q->pending_signals)> 0) {
                __v_deliver_signals(r,r->q->pending_signals,&v->r->instances);
            }

            // cleanup any fully reduced run-trees
            if (r->q->completed) _p_cleanup(r->q,_t_child(r->root,4));
        }
    }

    // close down all receptors
    c = _t_children(v->active_receptors);
    for (i=1;i<=c;i++) {
        Receptor *r = __r_get_receptor(_t_child(v->active_receptors,i));
        __r_kill(r);
        // if other receptors have threads associated with them, the possibly we should
        // be doing a thread_join here, or maybe even inside __r_kill @fixme
    }

    int err =0;
    pthread_exit(&err);  //@todo determine if we should use pthread_exit or just return 0
    return 0;
}

// fire up the threads that make the vmhost work
void _v_start_vmhost(VMHost *v) {
    _v_start_thread(&v->vm_thread,__v_process,G_vm);
}

/**
 * create all the built in receptors that exist in all VMhosts
 */
void _v_instantiate_builtins(VMHost *v) {
    Receptor *r = _r_makeClockReceptor();
    Xaddr clock = _v_new_receptor(v,CLOCK_RECEPTOR,r);
    _v_activate(v,clock);
}

/******************  thread handling */

void _v_start_thread(thread *t,void *(*start_routine)(void*), void *arg) {
    int rc;
    if (t->state) {
        raise_error("attempt to double-start a thread");
    }
    rc = pthread_create(&t->pthread,0,start_routine,arg);
    if (rc){
        raise_error("Error starting thread; return code from pthread_create() is %d\n", rc);
    }
    t->state = 1;
}

void _v_join_thread(thread *t) {
    if (t->state) { // make sure the thread was started before trying to join it
        void *status;
        int rc;

        rc = pthread_join(t->pthread, &status);
        if (rc) {
            raise_error("ERROR; return code from pthread_join() is %d\n", rc);
        }
        t->state = 0;
    }
}

/** @}*/
