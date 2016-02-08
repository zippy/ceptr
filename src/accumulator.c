/**
 * @ingroup accumulator
 *
 * @{
 * @file accumulator.c
 * @brief implementation file for accumulator
 *
 * @copyright Copyright (C) 2013-2014, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 */

#include "accumulator.h"
#include "semtrex.h"
#include "mtree.h"
#include "receptor.h"
#include <sys/types.h>
#include <sys/stat.h>
#include "debug.h"

VMHost *G_vm = 0;

#define PATHS_FN "paths"
#define SEM_FN "sem"

#define __a_vm_state_fn(buf,dir) __a_vm_fn(buf,dir,"state")
#define __a_vmfn(buf,dir) __a_vm_fn(buf,dir,"")
void __a_vm_fn(char *buf,char *dir,char *suffix) {
    sprintf(buf,"%s/vmhost%s.x",dir,suffix);
}

void __a_serializet(T *t,char *name) {
    char fn[1000];
    H h =_m_new_from_t(t);
    S *s = _m_serialize(h.m);
    __a_vm_fn(fn,G_vm->dir,name);
    writeFile(fn,s,s->total_size);
    free(s);
    _m_free(h);
}

T *__a_unserializet(char *dir_path,char *name) {
    char fn[1000];
    __a_vm_fn(fn,dir_path,name);
    S *s;
    readFile(fn,&s,0);
    H h = _m_unserialize(s);
    free(s);
    T *t = _t_new_from_m(h);
    _m_free(h);
    return t;
}

/**
 * bootstrap the ceptr system
 *
 * starts up the vmhost and wakes up receptors that should be running in it.
 *
 * @TODO check the compository to verify our version of the vmhost
 *
 */
void _a_boot(char *dir_path) {

    // check if the storage directory exists
    struct stat st = {0};
    if (stat(dir_path, &st) == -1) {
        // if no directory we are firing up an initial instance, so
        // create directory
        mkdir(dir_path,0700);

        // instantiate a VMHost object
        G_vm = _v_new();
        // create the basic receptors that all VMHosts have
        _v_instantiate_builtins(G_vm);
    }
    else {
        char fn[1000];
        void *buffer;
        // unserialize the semtable base tree
        SemTable *sem = _sem_new();
        T *t = __a_unserializet(dir_path,SEM_FN);
        sem->stores[0].definitions = t;

        // restore definitions to the correct store slots
        T *paths = __a_unserializet(dir_path,PATHS_FN);
        int i = 0;
        int c = _t_children(paths);
        for(i=1;i<=c;i++) {
            T *p = _t_child(paths,i);
            if (semeq(RECEPTOR_PATH,_t_symbol(p))) {
                T *x = _t_get(t,(int *)_t_surface(p));
                sem->stores[i-1].definitions = x;
            }
        }
        _t_free(paths);
        sem->contexts = c+1;

        // unserialize all of the vmhost's instantiated receptors and other instances
        __a_vmfn(fn,dir_path);
        readFile(fn,&buffer,0);

        Receptor *r = _r_unserialize(sem,buffer);
        G_vm = __v_init(r,sem);
        free(buffer);

        // unserialize other vmhost state data
        S *s;
        __a_vm_state_fn(fn,dir_path);
        readFile(fn,&s,0);
        H h = _m_unserialize(s);
        free(s);

        H hars; hars.m=h.m; hars.a = _m_child(h,1); // first child is ACTIVE_RECEPTORS
        H har; har.m=h.m;
        int j = _m_children(hars);
        for (i=1;i<=j;i++) {
            har.a = _m_child(hars,i);
            if(!semeq(_m_symbol(har),RECEPTOR_XADDR)) raise_error("expecting RECEPTOR_XADDR!");
            _v_activate(G_vm,*(Xaddr *)_m_surface(har));
        }
        _m_free(h);
    }
    G_vm->dir = dir_path;

    // _a_check_vm_host_version_on_the_compository();

    _v_start_vmhost(G_vm);
}


/**
 * clean shutdown of the the ceptr system
 *
 * should be called by the thread that called _a_boot() (or _a_start_vmhost())
 */
void _a_shut_down() {
    // cleanly close down any processing in the VM_Host
    __r_kill(G_vm->r);

    _v_join_thread(&G_vm->clock_thread);
    _v_join_thread(&G_vm->vm_thread);

    char fn[1000];

    // serialize the semtable
    __a_serializet(_t_root(G_vm->sem->stores[0].definitions),SEM_FN);

    int i;
    T *paths = _t_new_root(RECEPTOR_PATHS);
    for (i=0;i<G_vm->sem->contexts;i++) { // we don't need the path of the root so start at 1
        int *p = _t_get_path(G_vm->sem->stores[i].definitions);
        if (p) {
            _t_new(paths,RECEPTOR_PATH,p,sizeof(int)*(_t_path_depth(p)+1));
            free(p);
        }
        else
            _t_newr(paths,STRUCTURE_ANYTHING); // should be something like DELETED_CONTEXT
    }
    __a_serializet(paths,PATHS_FN);
    _t_free(paths);

    // serialize the receptor part of the vmhost
    void *surface;
    size_t length;
    _r_serialize(G_vm->r,&surface,&length);
    //    _r_unserialize(surface);
    __a_vmfn(fn,G_vm->dir);
    writeFile(fn,surface,length);
    free(surface);

    // serialize other parts of the vmhost
    H h = _m_newr(null_H,SYS_STATE);
    H har = _m_newr(h,ACTIVE_RECEPTORS);
    for (i=0;i<G_vm->active_receptor_count;i++) {
        _m_new(har,RECEPTOR_XADDR,&G_vm->active_receptors[i].x,sizeof(Xaddr));
    }
    S *s = _m_serialize(h.m);
    __a_vm_state_fn(fn,G_vm->dir);
    writeFile(fn,s,s->total_size);
    free(s);
    _m_free(h);

    // free the memory used by the SYS_RECEPTOR
    _v_free(G_vm);
    G_vm = NULL;
}

/*------------------------------------------------------------------------*/

T *__a_find(T *t,SemanticID sym) {
    T *p;
    DO_KIDS(t,
            p =_t_child(t,i);
            if (semeq(*(Symbol *)_t_surface(p),sym)) return p;
            );
    return NULL;
}

Xaddr _a_new_instance(Instances *instances,T *t) {
    T *x = *instances;
    if (!x) x = *instances = _t_new_root(INSTANCES);
    Symbol s = _t_symbol(t);
    T *si =  __a_find(x,s);
    if (!si) si = _t_news(x,SYMBOL_INSTANCES,s);
    _t_add(si,t);
    Xaddr result;
    result.symbol = s;
    result.addr = _t_children(si);
    return result;
}

T *_a_get_instance(Instances *instances,Xaddr x) {
    T *t = *instances;
    if (!t) return NULL;
    t = __a_find(t,x.symbol);
    if (t) {
        t = _t_child(t,x.addr);
        if (t && !semeq(_t_symbol(t),DELETED_INSTANCE)) return t;
    }
    return NULL;
}

T *_a_set_instance(Instances *instances,Xaddr x,T *r) {
    //@todo sanity check on t's symbol type?
    T *t = _a_get_instance(instances,x);
    if (t) {
        _t_replace(_t_parent(t),_t_node_index(t),r);
        return t;
    }
    return NULL;
}

void _a_get_instances(Instances *instances,Symbol s,T *t) {
    T *c,*x = *instances;
    if (!x) return;
    x = __a_find(x,s);
    if (x)
        DO_KIDS(x,
                c = _t_child(x,i);
                if (!semeq(_t_symbol(c),DELETED_INSTANCE))
                    _t_add(t,_t_clone(c));
                );
}

void _a_delete_instance(Instances *instances,Xaddr x) {
    T *t = *instances;
    if (!t) return;
    t = __a_find(t,x.symbol);
    if (t) {
        t = _t_child(t,x.addr);
        if (t) {
            T *d = _t_new_root(DELETED_INSTANCE);
            _t_replace_node(t,d);
        }
    }
}

void _a_free_instances(Instances *instances) {
    T *x = *instances;
    if (x) {
        _t_free(x);
        *instances = NULL;
    }
}

S *__a_serialize_instances(Instances *instances) {
    T *x = *instances;
    T *t = _t_new_root(PARAMS);
    if (x) {
        T *p;
        Symbol s;
        DO_KIDS(x,
                p =_t_child(x,i);
                s = *(Symbol *)_t_surface(p);
                T *sym = _t_news(t,STRUCTURE_SYMBOL,s);  // just using this symbol to store the symbol type
                T *c;
                DO_KIDS(p,
                        c = _t_child(p,i);
                        if (is_receptor(s)) {
                            void *surface;
                            size_t length;
                            Receptor *r = __r_get_receptor(c);
                            _r_serialize(r,&surface,&length);
                            c = _t_new(0,SERIALIZED_RECEPTOR,surface,length);
                            //@todo create a version of _t_new that doesn't have to realloc the surface but can just use it
                            free(surface);
                        }
                        else c = _t_clone(c);
                        _t_add(sym,c);
                        );
                );
    }
    H h = _m_new_from_t(t);
    S *s = _m_serialize(h.m);
    _m_free(h);_t_free(t);
    return s;
}

// the quick and dirty serialization builds a new tree
// which we will just serialize as an mtree.
// is is very inefficient @fixme
void _a_serialize_instances(Instances *i,char *file) {

    S *s = __a_serialize_instances(i);

    FILE *ofp;

    ofp = fopen(file, "w");
    if (ofp == NULL) {
        raise_error("Can't open output file %s!\n",file);
    }
    else {
        fwrite(s,1,s->total_size,ofp);
        fclose(ofp);
    }
    free(s);
}

void __a_unserialize_instances(SemTable *sem,Instances *instances,S *s) {
    H h = _m_unserialize(s);
    T *t = _t_new_from_m(h);

    _m_free(h);
    int j,c = _t_children(t);
    for(j=1;j<=c;j++) {
        T *u = _t_child(t,j);
        SemanticID s = *(SemanticID *)_t_surface(u);
        int is_receptor = is_receptor(s);
        while(_t_children(u)) {
            T *i = _t_detach_by_idx(u,1);
            if (is_receptor) {
                Receptor *r = _r_unserialize(sem,_t_surface(i));
                _t_free(i);
                i = _t_new_receptor(0,s,r);
            }
            _a_new_instance(instances, i);
        }
    }
    _t_free(t);
}

void _a_unserialize_instances(SemTable *sem,Instances *instances,char *file) {
    FILE *ofp;

    ofp = fopen(file, "r");
    if (ofp == NULL) {
        raise_error("Can't open input file %s!\n",file);
    }
    else {
        Mmagic magic;
        fread(&magic, 1,sizeof(magic), ofp);
        // @todo check magic value
        size_t total_size;
        fread(&magic, 1,sizeof(magic), ofp);
        S *s = malloc(total_size);
        s->magic = magic;
        s->total_size = total_size;
        size_t read_size = total_size-sizeof(Mmagic)-sizeof(total_size);
        fread(&s->levels,1,read_size,ofp);
        fclose(ofp);
        __a_unserialize_instances(sem,instances,s);
        free(s);
    }
}

/** @}*/
