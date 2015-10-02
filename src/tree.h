/**
 * @defgroup tree Semantic Trees
 *
 * @brief Semantic trees are the basic data building block for ceptr
 *
 * Everything in ceptr is built out of semantic trees.  In ceptr all data is assumed to be
 * meaningfull, not just structured.
 *
 * @{
 * @file tree.h
 * @brief semantic trees header file
 *
 * @copyright Copyright (C) 2013-2015, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 *
 */

#ifndef _CEPTR_TREE_H
#define _CEPTR_TREE_H

#include <string.h>
#include <stdlib.h>
#include "ceptr_error.h"
#include "sys_defs.h"
#include "base_defs.h"
#include "ceptr_types.h"
#include "stream.h"

#define TREE_CHILDREN_BLOCK 5
#define TREE_PATH_TERMINATOR -9999

enum TreeSurfaceFlags {TFLAG_ALLOCATED=0x0001,TFLAG_SURFACE_IS_TREE=0x0002,TFLAG_SURFACE_IS_RECEPTOR = 0x0004,TFLAG_SURFACE_IS_SCAPE=0x0008,TFLAG_SURFACE_IS_STREAM=0x0010,TFLAG_DELETED=0x0020,TFLAG_RUN_NODE=0x0040,TFLAG_REFERENCE=0x8000};

/*****************  Node creation and deletion*/
T *__t_new(T *t,Symbol symbol, void *surface, size_t size,int is_run_node);
#define _t_new(p,sy,su,s) __t_new(p,sy,su,s,0)
T *__t_newi(T *parent,Symbol symbol,int surface,int is_run_node);
#define _t_newi(p,sy,su) __t_newi(p,sy,su,0)
T *__t_news(T *parent,Symbol symbol,SemanticID surface,int is_run_node);
#define _t_news(parent,symbol,surface) __t_news(parent,symbol,surface,0)
T *_t_newt(T *parent,Symbol symbol,T *t);
T *_t_new_str(T *parent,Symbol symbol,char *str);
T *_t_new_root(Symbol symbol);
T *_t_newr(T *parent,Symbol symbol);
T *_t_new_receptor(T *parent,Symbol symbol,Receptor *r);
T *_t_new_scape(T *parent,Symbol symbol,Scape *s);
T *_t_new_stream(T *parent,Symbol symbol,Stream *s);
T *_t_newp(T *parent,Symbol symbol,Process surface);

void _t_add(T *t,T *c);
void _t_detach_by_ptr(T *t,T *c);
T *_t_detach_by_idx(T *t,int i);
void _t_replace(T *t,int i,T *r);
T *_t_swap(T *t,int i,T *r);
void _t_insert_at(T *t, int *path, T *i);
void _t_morph(T *dst,T *src);
void __t_morph(T *t,Symbol s,void *surface,size_t length,int allocate);
void __t_free_children(T *t);
void __t_free(T *t);
void _t_free(T *t);
T *_t_clone(T *t);
T *_t_rclone(T *t);

/******************** Node data accessors */
int _t_children(T *t);
void * _t_surface(T *t);
Symbol _t_symbol(T *t);
size_t _t_size(T *t);

/*****************  Tree navigation */
T * _t_parent(T *t);
T *_t_child(T *t,int i);
T * _t_root(T *t);
T * _t_next_sibling(T *t);
int _t_node_index(T *t);

/*****************  Tree path based accesses */
int _t_path_equal(int *p1,int *p2);
int _t_path_depth(int *p);
void _t_pathcpy(int *dst_p,int *src_p);
T * _t_get(T *t,int *p);
int *_t_get_path(T *t);
void * _t_get_surface(T *t,int *p);
char * _t_sprint_path(int *fp,char *buf);

/*****************  Tree hashing utilities */
TreeHash _t_hash(T *symbols,T *structures,T *t);
int _t_hash_equal(TreeHash h1,TreeHash h2);

/*****************  UUID utilities */
UUIDt __uuid_gen();
int __uuid_equal(UUIDt *u1,UUIDt *u2);

/*****************  Tree serialization */
size_t __t_serialize(Defs *d,T *t,void **bufferP,size_t offset,size_t current_size,int compact);
void _t_serialize(Defs *d,T *t,void **surfaceP,size_t *sizeP);
T * _t_unserialize(Defs *d,void **surfaceP,size_t *lengthP,T *t);

char * _t2rawjson(Defs *defs,T *t,int level,char *buf);
char * _t2json(Defs *defs,T *t,int level,char *buf);

/*****************  Misc... */

#define DO_KIDS(t,x) {int i,_c=_t_children(t);for(i=1;i<=_c;i++){x;}}

#define root_check(c) if (c->structure.parent != 0) {raise_error("can't add a node that isn't a root!");}

#endif
/** @}*/
