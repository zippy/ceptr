/**
 * @file spec_utils.c
 * @copyright Copyright (C) 2013-2016, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 * @ingroup tests
 */

#include "spec_utils.h"

bool G_done = false;

// write out a tree as json
void wjson(SemTable *sem,T *t,char *n,int i) {
    char fn[100];
    char json[100000] = {0};
    _t2json(sem,t,-1,json);
    if (i >= 0)
    sprintf(fn,"web/%s_%d.json",n,i);
    else
    sprintf(fn,"web/%s.json",n);
    writeFile(fn,json,strlen(json));
}

void dump2json(SemTable *sem,T *t,char *n) {
    char fn[100];
    char json[1000000] = {0};
    _t2rawjson(sem,t,-1,json);
    sprintf(fn,"web/%s.json",n);
    writeFile(fn,json,strlen(json));
}

// create a TREE_DELTA tree
T *makeDelta(Symbol sym,int *path,T *t,int count) {
    T *d = _t_new_root(sym);
    _t_new(d,TREE_DELTA_PATH,path,sizeof(int)*(_t_path_depth(path)+1));
    _t_add(_t_newr(d,TREE_DELTA_VALUE),_t_clone(t));
    if (count)
    _t_newi(d,TREE_DELTA_COUNT,count);
    return d;
}

char *G_visdump_fn = 0;
int G_visdump_count = 0;

void _visdump(SemTable *sem,T *x,int *path) {
    T *delta = makeDelta(TREE_DELTA_REPLACE,path,x,1);
    wjson(sem,delta,G_visdump_fn,G_visdump_count++);
    _t_free(delta);
}

void visdump(SemTable *sem,T *x) {
    if (G_visdump_count) {
    int *path = _t_get_path(x);
    _visdump(sem,x,path);
    free(path);
    }
}

#include "../src/process.h"
#include "../src/receptor.h"

void _test_reduce_signals(Receptor *r) {
    while (r->q->contexts_count) {
        _p_reduceq(r->q);

        // @todo fix this fake signal sending which only works here in this test because
        // the signals are being sent to ourself!
        T *signals = r->pending_signals;
        while(_t_children(signals)>0) {
            T *s = _t_detach_by_idx(signals,1);
            Error err = _r_deliver(r,s);
            if (err) raise_error("delivery error: %d",err);
        }
    }
}

char *doSys(char *cmd) {
    FILE *pipe = popen(cmd, "r");
    if (!pipe) raise_error("Error running %s\n",cmd);
    int sz = 1000;
    int remaining = sz;
    char *buffer = malloc(sz);
    char buf[255];
    while (fgets(buf, 255, pipe) != NULL) {
        int l = strlen(buf);
        if (l > remaining) {
            int increase_by = l < sz ? sz : l*2;
            remaining += increase_by;
            sz += increase_by;
            buffer = realloc(buffer,sz);
        }
        memcpy(&buffer[sz-remaining],buf,l);
        remaining -= l;
    }
    buffer[sz-remaining] = 0;
    pclose(pipe);
    return buffer;
}
