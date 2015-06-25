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

VMHost *G_vm = 0;

/**
 * bootstrap the ceptr system
 *
 * starts up the vmhost and wakes up receptors that should be running in it.
 *
 * @TODO check the compository to verify our version of the vmhost
 *
 */
void _a_boot() {
    // _de_check_vm_host_version_on_the_compository();
    _a_start_vmhost();
}

// for now starting the vmhost just means creating a global instance of it.
void _a_start_vmhost() {
    G_vm = _v_new();
}

/*------------------------------------------------------------------------*/

T *_a_get_instance(Instances *instances,Xaddr x) {
    instances_elem *e = 0;
    HASH_FIND_INT( *instances, &x.symbol, e );
    if (e) {
        instance_elem *i = 0;
        Instance *iP = &e->instances;
        HASH_FIND_INT( *iP, &x.addr, i );
        if (i) {
            return i->instance;
        }
    }
    return 0;
}

Xaddr _a_new_instance(Instances *instances,T *t) {
    Symbol s = _t_symbol(t);
    instances_elem *e;

    HASH_FIND_INT( *instances, &s, e );
    if (!e) {
        e = malloc(sizeof(struct instances_elem));
        e->instances = NULL;
        e->s = s;
        e->last_id = 0;
        HASH_ADD_INT(*instances,s,e);
    }
    e->last_id++;
    instance_elem *i;
    i = malloc(sizeof(struct instance_elem));
    i->instance = t;
    i->addr = e->last_id;
    Instance *iP = &e->instances;
    HASH_ADD_INT(*iP,addr,i);

    Xaddr result;
    result.symbol = s;
    result.addr = e->last_id;

    return result;
}

void instanceFree(Instance *i) {
    instance_elem *cur,*tmp;
    HASH_ITER(hh, *i, cur, tmp) {
        _t_free((*i)->instance);
        HASH_DEL(*i,cur);  /* delete; cur advances to next */
        free(cur);
    }
}

void _a_free_instances(Instances *i) {
    instances_elem *cur,*tmp;
    HASH_ITER(hh, *i, cur, tmp) {
        instanceFree(&(*i)->instances);
        HASH_DEL(*i,cur);  /* delete; cur advances to next */
        free(cur);
    }
}

// the quick and dirty serialization builds a new tree
// which we will just serialize as an mtree.
// is is very inefficient @fixme
void _a_serialize_instances(Instances *i,char *file) {

    T *t = _t_new_root(PARAMS);
    instances_elem *cur,*tmp;
    HASH_ITER(hh, *i, cur, tmp) {
        T *sym = _t_news(t,STRUCTURE_PART,cur->s);
        Instance *iP = &cur->instances;
        instance_elem *curi,*tmpi;
        HASH_ITER(hh, *iP, curi, tmpi) {
            _t_add(sym,_t_clone(curi->instance));
        }
    }
    size_t size;
    H h = _m_new_from_t(t);
    S *s = _m_serialize(h.m,&size);

    FILE *ofp;

    ofp = fopen(file, "w");
    if (ofp == NULL) {
        raise_error("Can't open output file %s!\n",file);
    }
    else {
        fwrite(&size, 1,sizeof(size), ofp);
        fwrite(s,1,size,ofp);
        fclose(ofp);
    }
    _m_free(h);free(s);_t_free(t);
}

void _a_unserialize_instances(Instances *instances,char *file) {
    FILE *ofp;

    ofp = fopen(file, "r");
    if (ofp == NULL) {
        raise_error("Can't open input file %s!\n",file);
    }
    else {
        size_t size;
        fread(&size, 1,sizeof(size), ofp);
        S *s = malloc(size);
        fread(s,1,size,ofp);
        fclose(ofp);
        H h = _m_unserialize(s);
        T *t = _t_new_from_m(h);
        free(s);
        _m_free(h);
        int j,c = _t_children(t);
        for(j=1;j<=c;j++) {
            T *u = _t_child(t,j);
            while(_t_children(u)) {
                _a_new_instance(instances, _t_detach_by_idx(u,1));
            }
        }
        _t_free(t);
    }
}

/** @}*/
