/**
 * @file spec_utils.c
 * @copyright Copyright (C) 2013-2015, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 * @ingroup tests
 */

#include "spec_utils.h"

// write out a tree as json
void wjson(Defs *d,T *t,char *n,int i) {
    char fn[100];
    char json[100000] = {0};
    _t2json(d,t,-1,json);
    if (i >= 0)
	sprintf(fn,"web/%s_%d.json",n,i);
    else
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

void _visdump(Defs *defs,T *x,int *path) {
    T *delta = makeDelta(TREE_DELTA_REPLACE,path,x,1);
    wjson(defs,delta,G_visdump_fn,G_visdump_count++);
    _t_free(delta);
}

void visdump(Defs *defs,T *x) {
    if (G_visdump_count) {
	int *path = _t_get_path(x);
	_visdump(defs,x,path);
	free(path);
    }
}
