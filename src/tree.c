/**
 * @ingroup tree
 *
 * @{
 *
 * @file tree.c
 * @brief semantic tree pointer implementation
 *
 * @copyright Copyright (C) 2013-2016, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 */

#include "tree.h"
#include "ceptr_error.h"
#include "hashfn.h"
#include "def.h"

#include "receptor.h"
#include "scape.h"
#include "util.h"
#include "debug.h"

/*****************  Node creation */
void __t_append_child(T *t,T *c) {
    if (t->structure.child_count == 0) {
        t->structure.children = malloc(sizeof(T *)*TREE_CHILDREN_BLOCK);
    } else if (!(t->structure.child_count % TREE_CHILDREN_BLOCK)){
        int b = t->structure.child_count/TREE_CHILDREN_BLOCK + 1;
        t->structure.children = realloc(t->structure.children,sizeof(T *)*(TREE_CHILDREN_BLOCK*b));
    }

    t->structure.children[t->structure.child_count++] = c;
}

T * __t_init(T *parent,Symbol symbol,bool is_run_node) {
    T *t = malloc(is_run_node ? sizeof(rT) : sizeof(T));
    t->structure.child_count = 0;
    t->structure.parent = parent;
    t->contents.symbol = symbol;
    t->context.flags = 0;
    if (is_run_node) {
        ((rT *)t)->cur_child = RUN_TREE_NOT_EVAULATED;
        t->context.flags |= TFLAG_RUN_NODE;
    }
    if (parent != NULL) {
        __t_append_child(parent,t);
    }
}

/**
 * Create a new tree node
 *
 * @param[in] parent parent node for the node to be created.  Can be 0 if this is a root node
 * @param[in] symbol semantic symbol for the node to be create
 * @param[in] surface pointer to node's data
 * @param[in] size size in bytes of the surface
 * @returns pointer to node allocated on the heap
 */
T * __t_new(T *parent,Symbol symbol,void *surface,size_t size,bool is_run_node) {
    T *t = __t_init(parent,symbol,is_run_node);
    if (is_run_node) t->context.flags |= TFLAG_RUN_NODE;
    if (size && surface) {
        void *dst;
        if (size <= sizeof(void *)) {
            dst = &t->contents.surface;
        }
        else {
            t->context.flags |= TFLAG_ALLOCATED;
            dst = t->contents.surface = malloc(size);
        }
        memcpy(dst,surface,size);
    }
    t->contents.size = size;
    return t;
}

/**
 * Create a new tree node with an char surface
 *
 * @param[in] parent parent node for the node to be created.  Can be 0 if this is a root node
 * @param[in] symbol semantic symbol for the node to be create
 * @param[in] surface char value to store in the surface
 * @returns pointer to node allocated on the heap
 */
T * __t_newc(T *parent,Symbol symbol,char surface,bool is_run_node) {
    return __t_new(parent,symbol,&surface,sizeof(char),is_run_node);
}

/**
 * Create a new tree node with an integer surface
 *
 * @param[in] parent parent node for the node to be created.  Can be 0 if this is a root node
 * @param[in] symbol semantic symbol for the node to be create
 * @param[in] surface integer value to store in the surface
 * @returns pointer to node allocated on the heap
 */
T * __t_newi(T *parent,Symbol symbol,int surface,bool is_run_node) {
    return __t_new(parent,symbol,&surface,sizeof(int),is_run_node);

}

/**
 * Create a new tree node with a SemanticID surface
 *
 * @param[in] parent parent node for the node to be created.  Can be 0 if this is a root node
 * @param[in] symbol semantic symbol for the node to be create
 * @param[in] surface semanticID value to store in the surface
 * @returns pointer to node allocated on the heap
 */
T *__t_news(T *parent,Symbol symbol,SemanticID surface,bool is_run_node){
    return __t_new(parent,symbol,&surface,sizeof(SemanticID),is_run_node);
}

/**
 * Create a new tree node with a tree as it's surface
 *
 * @param[in] parent parent node for the node to be created.  Can be 0 if this is a root node
 * @param[in] symbol semantic symbol for the node to be create
 * @param[in] surface pointer to tree to store as an orthogonal tree in the surface
 * @returns pointer to node allocated on the heap
 */
T * _t_newt(T *parent,Symbol symbol,T *surface) {
    T *t = __t_init(parent,symbol,false);
    *((T **)&t->contents.surface) = surface;
    t->contents.size = sizeof(T *);

    t->context.flags |= TFLAG_SURFACE_IS_TREE;
    return t;
}

/**
 * Create a new tree node with a cstring surface
 *
 * @param[in] parent parent node for the node to be created.  Can be 0 if this is a root node
 * @param[in] symbol semantic symbol for the node to be create
 * @param[in] surface string value to store in the surface
 * @returns pointer to node allocated on the heap
 */
T * __t_new_str(T *parent,Symbol symbol,char *surface,bool is_run_node) {
    return __t_new(parent,symbol,surface,strlen(surface)+1,is_run_node);
}

/**
 * Create a new tree root node (with null surface and no parent)
 *
 * @param[in] symbol semantic symbol for the node to be create
 * @returns pointer to node allocated on the heap
 */
T *_t_new_root(Symbol symbol) {
    return _t_new(0,symbol,0,0);
}

/**
 * Create a new tree sub-root node (with null surface)
 *
 * @param[in] parent parent node for the node to be created.
 * @param[in] symbol semantic symbol for the node to be create
 * @returns pointer to node allocated on the heap
 */
T *__t_newr(T *parent,Symbol symbol,bool is_run_node) {
    return __t_new(parent,symbol,0,0,is_run_node);
}

/* create tree node whose surface is a specially allocated c structure,
   i.e. a receptor, scape, or stream, these nodes get cloned as references
   so the c-structure isn't double freed
 */
T *__t_new_special(T *parent,Symbol symbol,void *s,int flag,bool is_run_node) {
    T *t = __t_init(parent,symbol,is_run_node);
    t->contents.surface = s;
    t->contents.size = sizeof(void *);

    t->context.flags |= flag;
    if (is_run_node) t->context.flags |= TFLAG_RUN_NODE;
    return t;
}

/**
 * Create a new tree node with a receptor as it's surface
 *
 * this is just like _t_newt except that it uses a different flag because
 * when cleaning up we'll need to know that this is a full receptor, not just
 * a plain tree
 *
 * @param[in] parent parent node for the node to be created.
 * @param[in] symbol semantic symbol for the node to be create
 * @param[in] r receptor
 * @returns pointer to node allocated on the heap
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/tree_spec.h testTreeNewReceptor
 */
T *_t_new_receptor(T *parent,Symbol symbol,Receptor *r) {
    T *t = __t_new_special(parent,symbol,r,TFLAG_SURFACE_IS_TREE+TFLAG_SURFACE_IS_RECEPTOR,0);
    return t;
}

/**
 * Create a new tree node with a scape as it's surface
 *
 * when cleaning up we'll need to know that this is a scape
 *
 * @param[in] parent parent node for the node to be created.
 * @param[in] symbol semantic symbol for the node to be create
 * @param[in] s scape
 * @returns pointer to node allocated on the heap
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/tree_spec.h testTreeScape
 */
T *_t_new_scape(T *parent,Symbol symbol,Scape *s) {
    return __t_new_special(parent,symbol,s,TFLAG_SURFACE_IS_SCAPE,0);
}

/**
 * Create a new tree node with a stream as it's surface
 *
 * when cleaning up we'll need to know that this is a stream
 *
 * @param[in] parent parent node for the node to be created.
 * @param[in] symbol semantic symbol for the node to be create
 * @param[in] s stream
 * @returns pointer to node allocated on the heap
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/tree_spec.h testTreeStream
 */
T *_t_new_cptr(T *parent,Symbol symbol,void *s) {
    return __t_new_special(parent,symbol,s,TFLAG_SURFACE_IS_CPTR+TFLAG_REFERENCE,0);
}

/**
 * Create a new tree node with a Process surface
 *
 * @param[in] parent parent node for the node to be created.  Can be 0 if this is a root node
 * @param[in] symbol semantic symbol for the node to be create
 * @param[in] surface Process value
 * @returns pointer to node allocated on the heap
 */
T * _t_newp(T *parent,Symbol symbol,Process surface) {
    return _t_news(parent,symbol,surface);
}

/**
 * add an existing tree onto another appending it as a child
 *
 * @param[in] t tree onto which c will be added
 * @param[in] c tree to add onto t
 */
void _t_add(T *t,T *c) {
    root_check(c);
    c->structure.parent = t;
    __t_append_child(t,c);
}

/**
 * Detatch the specified child from a node and return it
 *
 * @note does not free the memory occupied by the child
 * @param[in] t node to detach from
 * @param[in] i index of the child to detach
 * @returns the detatched child
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/tree_spec.h testTreeDetach
 */
T *_t_detach_by_idx(T *t,int i) {
    T *x = _t_child(t,i);
    _t_detach_by_ptr(t,x);
    return x;
}

/**
 * search for a given node in the child list of a node and detatch it if found
 *
 * @note does not free the memory occupied by c
 * @param[in] t node to search
 * @param[in] c node to search for in child list
 */
void _t_detach_by_ptr(T *t,T *c) {
    // search for the child to be removed
    DO_KIDS(t,
            if (_t_child(t,i) == c) {
                // if found remove it by decreasing the child count and shift all the other children down
                t->structure.child_count--;
                if (t->structure.child_count == 0) {
                    free(t->structure.children);
                }
                for(;i<_c;i++) {
                    t->structure.children[i-1] = t->structure.children[i];
                }
                break;
            }
            );

    if (c) c->structure.parent = 0;
}

/**
 * Convert the surface of a node
 *
 * Frees the original surface value if it was allocated.
 *
 * @note only converts the type and surface, not the children!
 * @param[in] t Node to be morphed
 * @param[in] s Symbol to change the surface to
 * @param[in] surface data to copy into the surface
 * @param[in] size of data
 * @param[in] allocate boolean to know whether to allocate surface or copy the surface as an int
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/tree_spec.h testTreeMorphLowLevel
 */
void __t_morph(T *t,Symbol s,void *surface,size_t size,int allocate) {
    t->contents.size = size;
    if (t->context.flags & TFLAG_ALLOCATED) {
        free(t->contents.surface);
    }

    if (allocate) {
        t->contents.surface = malloc(size);
        memcpy(t->contents.surface,surface,size);
        t->context.flags = TFLAG_ALLOCATED; /// @todo Handle the case where the surface of the node to be morphed is itself a tree
    }
    else {
        if (surface)
            memcpy(&t->contents.surface,surface,size);
        t->context.flags = 0;
    }

    t->contents.symbol = s;
}

/**
 * Convert the surface of one node to that of another
 *
 * Frees the original surface value if it was allocated.
 *
 * @note only converts the type and surface, not the children!
 * @param[in] dst Node to be morphed
 * @param[in] src Node type that dst should be morephed to
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/tree_spec.h testTreeMorph
 */
void _t_morph(T *dst,T *src) {
    __t_morph(dst,_t_symbol(src),_t_surface(src),_t_size(src),src->context.flags & TFLAG_ALLOCATED);
}

/**
 * Replace the specified child with the given node.

 * @note frees the replaced child
 * @param[in] t parent node on which to operate
 * @param[in] i index to child be replaced
 * @param[in] r node to replace
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/tree_spec.h testTreeReplace
 */
void _t_replace(T *t,int i,T *r) {
    T *c = _t_child(t,i);
    if (!c) {raise_error("tree doesn't have child %d",i);}
    _t_free(c);
    t->structure.children[i-1] = r;
    r->structure.parent = t;
}

/**
 * Replace the a node with the given node.
 *
 * @note frees any children of node to be replaced.  Beware that the second parameter pointer gets freed too and will become invalid
 *
 * @param[in] t node to be replaced
 * @param[in] r node to replace
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/tree_spec.h testTreeReplace
 */
void _t_replace_node(T *t,T *r) {
    root_check(r);
    if  ((t->context.flags & TFLAG_RUN_NODE) != (r->context.flags & TFLAG_RUN_NODE)) {
        raise_error("runnode mismatch");
    }
    __t_free(t);
    t->contents = r->contents;
    t->structure.child_count = r->structure.child_count;
    t->structure.children = r->structure.children;
    t->context = r->context;
    free(r);
    // fix the childrens' parent pointer
    DO_KIDS(t,_t_child(t,i)->structure.parent = t);
}

/**
 * Swap out the specified child for the given node.

 * @note returns the replaced child
 * @param[in] t input node on which to operate
 * @param[in] i index to child be replaced
 * @param[in] r node to replace
 * @returns pointer to node allocated on the heap
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/tree_spec.h testTreeSwap
 */
T *_t_swap(T *t,int i,T *r) {
    root_check(r);
    T *c = _t_child(t,i);
    if (!c) {raise_error("tree doesn't have child %d",i);}
    t->structure.children[i-1] = r;
    r->structure.parent = t;
    c->structure.parent = NULL;
    return c;
}

/**
 * Insert a tree at a given tree path position

 * @param[in] t tree on which to operate
 * @param[in] p path to position in tree to inesert
 * @param[in] i tree to insert
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/tree_spec.h testTreeInsertAt
 */
void _t_insert_at(T *t, int *path, T *i) {
    T *c = _t_get(t,path);
    int d = _t_path_depth(path)-1;
    if (c) {
        T *p = _t_parent(c);
        if (!p) {
            raise_error("Can't insert into the root!");
        }

        // first insert the new tree at the end just to use the code we
        // already have for mallocing children, etc,
        _t_add(p,i);

        // then shift the other children over
        int j,l = _t_children(p);
        j = l - path[d];
        T **tp = &p->structure.children[l-1];
        while(j--) {
            *tp = *(tp-1);
            tp--;
        }
        // and put the new tree where it belongs
        *tp = i;
    }
    else {
        // if path points to one beyond last child, we can simply add it.
        if (path[d]>1) {
            path[d]--;
            c = _t_get(t,path);
            if (c) {
                T *p = _t_parent(c);
                if (!p) {
                    raise_error("Can't insert into the root!");
                }
                _t_add(p,i);
                path[d]++;
                return;
            }
        }
        else if (d==0) { // special case to insert into first child of empty root
            _t_add(t,i);
            return;
        }
        raise_error("Path must lead to an existing node or one after last child.");
    }
}

/*****************  Node deletion */

void __t_free_children(T *t) {
    int c = t->structure.child_count;
    if (c > 0) {
        while(--c>=0) {
            _t_free(t->structure.children[c]);
        }
        free(t->structure.children);
    }
    t->structure.child_count = 0;
}

// free everything except the node itself
void __t_free(T *t) {
    __t_free_children(t);
    if (!(t->context.flags & TFLAG_REFERENCE)) {
        if (t->context.flags & TFLAG_ALLOCATED)
            free(t->contents.surface);
        else if (t->context.flags & TFLAG_SURFACE_IS_TREE) {
            if (t->context.flags & TFLAG_SURFACE_IS_RECEPTOR)
                _r_free((Receptor *)t->contents.surface);
            else
                _t_free((T *)t->contents.surface);
        }
        else if (t->context.flags & TFLAG_SURFACE_IS_SCAPE)
            _s_free((Scape *)t->contents.surface);
        else if (t->context.flags & TFLAG_SURFACE_IS_CPTR) {
            raise_error("WHAAA!");
        }
    }
}

/**
 * free the memory occupied by a tree
 *
 * free walks the tree freeing all the children and any orthogonal trees.
 *
 * @param[in] t tree to be freed
 * @todo make this remove the child from the parent's child-list?
 */
void _t_free(T *t) {
    __t_free(t);
    free(t);
}

T *__t_clone(T *t,T *p) {
    T *nt;
    uint32_t flags = t->context.flags;

    // if the tree points to a type that has an allocated c structure as its surface
    // the clone must be marked as a reference, otherwise it would get freed twice
    if (flags & (TFLAG_SURFACE_IS_RECEPTOR+TFLAG_SURFACE_IS_SCAPE+TFLAG_SURFACE_IS_CPTR)) {
        nt = __t_new_special(p,_t_symbol(t),_t_surface(t),flags,0);
        nt->context.flags |= TFLAG_REFERENCE;
    }
    else if (flags & TFLAG_SURFACE_IS_TREE) {
        nt = _t_newt(p,_t_symbol(t),__t_clone((T *)_t_surface(t),0));
    }
    else if(_t_size(t) == 0)
        nt = _t_newr(p,_t_symbol(t));
    else
        nt = _t_new(p,_t_symbol(t),_t_surface(t),_t_size(t));
    DO_KIDS(t,__t_clone(_t_child(t,i),nt));


    return nt;
}

T *__t_rclone(T *t,T *p) {
    T *nt;
    uint32_t flags = t->context.flags;
    if (flags & TFLAG_SURFACE_IS_RECEPTOR) {
        raise_error("can't rclone receptors");
    }
    // if the tree points to a type that has an allocated c structure as its surface
    // the clone must be marked as a reference, otherwise it would get freed twice
    else if (flags & (TFLAG_SURFACE_IS_RECEPTOR+TFLAG_SURFACE_IS_SCAPE+TFLAG_SURFACE_IS_CPTR)) {
        nt = __t_new_special(p,_t_symbol(t),_t_surface(t),flags,1);
        nt->context.flags |= TFLAG_REFERENCE;
    }
    else if (flags & TFLAG_SURFACE_IS_TREE) {
        nt = _t_newt(p,_t_symbol(t),__t_rclone((T *)_t_surface(t),0));
    }
    else if(_t_size(t) == 0)
        nt = __t_new(p,_t_symbol(t),0,0,1);
    else
        nt = __t_new(p,_t_symbol(t),_t_surface(t),_t_size(t),1);
    ((rT *)nt)->cur_child =  RUN_TREE_NOT_EVAULATED;
    DO_KIDS(t,__t_rclone(_t_child(t,i),nt));
    return nt;
}

/**
 * make a copy of a tree
 *
 * @param[in] t tree to clone
 * @returns T duplicated tree
 *
 * @note all receptor/scape/stream trees are cloned as references
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/tree_spec.h testTreeClone
 */
T *_t_clone(T *t) {
    return __t_clone(t,0);
}

T *_t_rclone(T *t) {
    return __t_rclone(t,0);
}

SemanticID _getBuildType(SemTable *sem,SemanticID param,Structure *stP,T **defP) {
    if (is_process(param)) {
        *defP = _sem_get_def(sem,param);
        return PROCESS_SIGNATURE;
    }
    /* if (is_protocol(param)) { */
    /*     *defP = _sem_get_def(sem,param); */
    /*     return PROTOCOL_SEMANTICS; */
    /* } */
    Structure st = _sem_get_symbol_structure(sem,param);
    *stP = st;
    if (semeq(st,NULL_STRUCTURE)) {
        *defP = NULL;
        return st;
    }
    T *st_def = _sem_get_def(sem,st);
    T *def = _t_child(st_def,2);
    *defP = def;
    Symbol stsym = _t_symbol(def);
    return stsym;
}
/* // check the symbol set to see if param is legal: */
/* int i,c = _t_children(def); */
/* for (i=1;i<=c;i++) { */
/*     if (semeq(*(Symbol *)_t_surface(_t_child(def,i)),param)) break; */
/*  } */
/* if (i > c) raise_error("got %s but expecting symbol in set: %s",_sem_get_name(sem,param),_t2s(sem,def)); */
enum buildStates {bReadSymbol,bPop,bAddRoot,bReadBase};
/**
 * helper for building trees in c
 *
 * @param[in] sem current semantic contexts
 * @param[in] parent the tree to add the built tree onto (can be NULL)
 * @param[in] ... varargs directing the building of the tree.  See examples
 * @returns built tree
 *
 * @note this function doesn't validate that built tree follows the restrictions in the definitions it just uses the definitions to know what to expect in the varargs.  Thus if you put the wrong things in the varargs it will likely segfault!  Also you can build illegal trees.  It would be possible to add an expectations layer that would see if what comes next looks ok according to what's expected as specified in the definitions.  It would also be possible to run the built tree against the semtrex produced by _d_build_def_semtrex
 */
T *_t_build(SemTable *sem,T *parent,...) {
    va_list ap;
    va_start (ap, parent);
    Symbol param,type;
    T *t = parent;
    bool done = false;
    Structure st = NULL_STRUCTURE;
    char *stn;
    T *def,*p;
    int state = bReadSymbol;
    //    Symbol expecting[100];
    //    T* expecting_def[100];
    int level = 0;
    while(!done) {
        switch(state) {
        case bReadSymbol:
            param = va_arg(ap,Symbol);
            if (semeq(param,NULL_SYMBOL)) {debug(D_TREE,"read NULL_SYMBOL\n");state=bPop;break;}
            //expecting[level]
            type = _getBuildType(sem,param,&st,&def);
            //expectig_def[level] = def;
            debug(D_TREE,"reading %s of type: %s with def:%s\n",_sem_get_name(sem,param),_sem_get_name(sem,type),_t2s(sem,def));
            if (semeq(type,PROCESS_SIGNATURE)) {
                state = bAddRoot;
            }
            else if (semeq(type,STRUCTURE_SYMBOL)) {
                if (semeq(*(Symbol *)_t_surface(def),NULL_SYMBOL)) {
                    state = bReadBase;
                }
                else {
                    state = bAddRoot;
                }
            }
            else if (semeq(type,NULL_STRUCTURE)) {
                // null structure literal so just add it and pop
                _t_newr(t,param);
                state = bPop;
            }
            else if (semeq(type,STRUCTURE_OR)) {
                state = bAddRoot;
            }
            else if (semeq(type,STRUCTURE_ZERO_OR_MORE) || semeq(type,STRUCTURE_ANYTHING) || semeq(type,STRUCTURE_SEQUENCE) || semeq(type,STRUCTURE_ONE_OR_MORE)) {
                state = bAddRoot;
            }
            else {
                debug(D_TREE,"type:%s \n",_sem_get_name(sem,type));
                raise_error("type: %s unimplemented in bReadSymbol",_sem_get_name(sem,type));
            }
            break;
        case bReadBase:
            // these are all the system defined structures
            debug(D_TREE,"building sys structure %s\n",_sem_get_name(sem,st));
            if (semeq(st,PROCESS) || semeq(st,SYMBOL) || semeq(st,STRUCTURE) || semeq(st,PROTOCOL)) {
                t = _t_news(t,param,va_arg(ap,SemanticID));
                // execption for SEMTREX_GROUP which has children...
                if (semeq(param,SEMTREX_GROUP)) {
                    state = bReadSymbol;
                    break;
                }
            }
            else if (semeq(st,INTEGER) || semeq(st,BIT)) {
                t = _t_newi(t,param,va_arg(ap,int));
            }
            else if (semeq(st,CSTRING)) {
                t = _t_new_str(t,param,va_arg(ap,char *));
            }
            else if (semeq(st,CHAR)) {
                int i = va_arg(ap,int);
                t = _t_newc(t,param,i);
            }
            else if (semeq(st,FLOAT)) {
                double d = va_arg(ap,double); // because vararg floats get promoted to doubles
                float f = d;
                t = _t_new(t,param,&f,sizeof(float));
            }
            else if (semeq(st,TREE_PATH)) {
                int path[100];
                int j = 0;
                while((path[j]=va_arg(ap,int)) != TREE_PATH_TERMINATOR) {
                    j++;
                    if (j==100) raise_error("path too deep!");
                }
                t = _t_new(t,param,path,sizeof(int)*(j+1));
            }
            else {
                raise_error("unimplemented surface type:%s",_sem_get_name(sem,st));
            }
            state = bPop;
            break;
        case bPop:
            p = _t_parent(t);
            if (p == parent) {done = true;break;}
            t = p;
            param = _t_symbol(t);
            type = _getBuildType(sem,param,&st,&def);
            debug(D_TREE,"popping to %s of type %s\n",_sem_get_name(sem,param),_sem_get_name(sem,type));
            if (semeq(type,STRUCTURE_ZERO_OR_MORE) || semeq(type,STRUCTURE_ANYTHING) || semeq(type,STRUCTURE_SEQUENCE) || semeq(type,STRUCTURE_ONE_OR_MORE)) {
                state = bReadSymbol;
            }
            else if (semeq(type,PROCESS_SIGNATURE)) {
                state = bReadSymbol;
            }
            else if (semeq(type,STRUCTURE_OR)) {
                state = bPop;  //gotta pop right away because we only expect one item
            }
            else if (semeq(type,STRUCTURE_SYMBOL)) {
                // this should only happen when popping back from a SEMTREX_GROUP
                // so just pop again
                state = bPop;  //gotta pop right away because we only expect one item
            }
            else {
                debug(D_TREE,"type:%s \n",_sem_get_name(sem,type));
                raise_error("type: %s unimplemented in bPop",_sem_get_name(sem,type));
            }
            break;
        case bAddRoot:
            debug(D_TREE,"adding node %s\n",_sem_get_name(sem,param));
            t  = _t_newr(t,param);
            state = bReadSymbol;
            break;
        }
    }
    va_end(ap);
    return t;
}

/**
 * another helper for building trees in c
 *
 * @param[in] sem current semantic contexts
 * @param[in] parent the tree to add the built tree onto (can be NULL)
 * @param[in] ... varargs directing the building of the tree.  See examples
 * @returns built tree
 *
 * @note this function doesn't validate that built tree follows the restrictions in the definitions it just uses the definitions to know what to expect in the varargs.  Thus if you put the wrong things in the varargs it will likely segfault!  Also you can build illegal trees.  It would be possible to add an expectations layer that would see if what comes next looks ok according to what's expected as specified in the definitions.  It would also be possible to run the built tree against the semtrex produced by _d_build_def_semtrex
 */
T *_t_build2(SemTable *sem,T *parent,...) {
    va_list ap;
    va_start (ap, parent);
    Symbol param,type,node;
    T *t = parent;
    bool done = false;
    Structure st = NULL_STRUCTURE;
    char *stn;
    T *def,*p;
    int level = 0;
    while(!done) {
        param = va_arg(ap,Symbol);
        if (semeq(param,STX_OP)) {
            node = va_arg(ap,Symbol);
            type = _getBuildType(sem,node,&st,&def);
            if (semeq(type,STRUCTURE_SYMBOL) && semeq(*(Symbol *)_t_surface(def),NULL_SYMBOL)) {
                debug(D_TREE,"building sys structure %s\n",_sem_get_name(sem,st));
                if (semeq(st,PROCESS) || semeq(st,SYMBOL) || semeq(st,STRUCTURE) || semeq(st,PROTOCOL)) {
                    t = _t_news(t,node,va_arg(ap,SemanticID));
                }
                else if (semeq(st,INTEGER) || semeq(st,BIT)) {
                    t = _t_newi(t,node,va_arg(ap,int));
                }
                else if (semeq(st,CSTRING)) {
                    t = _t_new_str(t,node,va_arg(ap,char *));
                }
                else if (semeq(st,CHAR)) {
                    int x = va_arg(ap,int);
                    t = _t_newc(t,node,x);
                }
                else if (semeq(st,TREE_PATH)) {
                    int path[100];
                    int j = 0;
                    while((path[j]=va_arg(ap,int)) != TREE_PATH_TERMINATOR) {
                        j++;
                        if (j==100) raise_error("path too deep!");
                    }
                    t = _t_new(t,node,path,sizeof(int)*(j+1));
                }
                else {
                    raise_error("unimplemented surface type:%s",_sem_get_name(sem,st));
                }
            }
            else {
                t = _t_newr(t,node);
            }
        }
        else if (semeq(param,STX_CP)) {
            p = _t_parent(t);
            if (p == parent) {done = true;break;}
            t = p;
        }
        else {
            raise_error("expecting open or close! got: %s",_sem_get_name(sem,param));
        }
    }
    va_end(ap);
    return t;
}



#include "semtrex.h"
// used to resolve a semantic link that links kind to kind.
T *__t_find_actual(T *sem_map,Symbol actual_kind,T *replacement_kind) {
    T *result = NULL;
    T *stx = _t_new_root(SEMTREX_WALK);
    T *l = _sl(stx,SEMANTIC_LINK);
    T *seq = _t_newr(l,SEMTREX_SEQUENCE);
    T *x = _t_newr(seq,SEMTREX_VALUE_LITERAL);
    T *k = _t_clone(replacement_kind);
    _t_add(x,k);
    x = _sl(seq,REPLACEMENT_VALUE);
    T *g = _t_news(x,SEMTREX_GROUP,actual_kind);
    x = _sl(g,actual_kind);
    T *mr;
    if (_t_matchr(stx,sem_map,&mr)) {
        result = _stx_get_matched_node(actual_kind,mr,sem_map,NULL);
        debug(D_TREE,"   re-mapping %s ->",t2s(replacement_kind));
        debug(D_TREE,"%s\n",t2s(result));
        _t_free(mr);
    }
    _t_free(stx);
    return result;
}

/**
 * replace SLOTS in a template with the replacement values from links in a SEMANTIC_MAP tree
 *
 * @param[in,out] template the tree with SLOTs to be filled
 * @param[in] sem_map mappings used to fill the template
 *
 * @note the template is modified in place, so the caller may need to clone a source template
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/tree_spec.h testTreeTemplate
*/
void __t_fill_template(T *template, T *sem_map,bool as_run_node) {
    if (!template) return;
    bool is_run_node = (template->context.flags |= TFLAG_RUN_NODE) || as_run_node;
    if (semeq(_t_symbol(template),SLOT)) {
        T *t = _t_child(template,SlotSemanticRefIdx);
        Symbol sym = _t_symbol(t);
        Symbol valsym = *(Symbol *)_t_surface(t);
        T *v = _t_child(template,SlotValueOfIdx);
        Symbol valof;
        T *children = NULL;
        if (v) {
            Symbol vsym = _t_symbol(v);
            if (semeq(vsym,SLOT_IS_VALUE_OF))
                valof = *(Symbol *)_t_surface(v);
            else if (semeq(vsym,SLOT_CHILDREN)) {
                __t_fill_template(v,sem_map,as_run_node);
                _t_detach_by_ptr(template,v);
                children = v;
                v = NULL;
            }
            else {
                raise_error("expecting SLOT_IS_VALUE_OF or SLOT_CHILDREN, got %s",_sem_get_name(G_sem,vsym));
            }
        }
        debug(D_TREE,"replacing %s\n",t2s(template));
        // scan all the sem_map for semantic refs that match this slot.
        // @todo convert this to a hashtable based implementation, probably on the treehash of the semantic ref
        int i,c = _t_children(sem_map);
        for(i=1;i<=c;i++) {
            T *m = _t_child(sem_map,i);
            T *ref = _t_child(m,SemanticMapSemanticRefIdx);
            debug(D_TREE,"checking to see if sym:%s == _t_symbol(ref):%s\n",_sem_get_name(G_sem,sym),_sem_get_name(G_sem,_t_symbol(ref)));
            debug(D_TREE," and that valsym:%s == _t_surface(ref):%s\n",_sem_get_name(G_sem,valsym),_sem_get_name(G_sem,*(Symbol *)_t_surface(ref)));
            if (semeq(sym,_t_symbol(ref)) && semeq(valsym,*(Symbol *)_t_surface(ref))) {
                debug(D_TREE,"with %s\n",t2s(m));
                T *r = NULL;
                T *replacement_value = _t_child(_t_child(m,SemanticMapReplacementValIdx),1);
                if (semeq(sym,GOAL) && !v) { // treat a VALUE_OF slot as a symbol not a process
                    SemanticID p = _t_symbol(replacement_value);
                    if (!is_process(p)) {
                        if (semeq(ACTUAL_PROCESS,p)) {
                            p = *(Process *)_t_surface(replacement_value);
                        }
                        else if (semeq(GOAL,p)) {
                            // if the replacement value is a goal, we need to look for
                            // it's actual value in the map
                            T *x = __t_find_actual(sem_map,ACTUAL_PROCESS,replacement_value);
                            if (!x)
                                raise_error("unable to find actual for %s",t2s(m));
                            p = *(Process *)_t_surface(x);
                        }
                        else {
                            raise_error("expecting GOAL or ACTUAL_PROCESS for replacement values. got: %s",t2s(m));
                        }
                    }
                    r = __t_new(0,p,0,0,is_run_node);
                    if (is_run_node)
                        ((rT *)r)->cur_child = RUN_TREE_NOT_EVAULATED;
                }
                else {
                    SemanticID rsid = _t_symbol(replacement_value);
                    // if the replacement value is a kind try to re-resolve from the map
                    T *x = NULL;
                    if (semeq(rsid,ROLE)) {
                        T *x = __t_find_actual(sem_map,ACTUAL_RECEPTOR,replacement_value);
                        if (x) replacement_value = x;
                    }
                    else if (semeq(rsid,USAGE)) {
                        T *x = __t_find_actual(sem_map,ACTUAL_SYMBOL,replacement_value);
                        if (x) replacement_value = x;
                    }
                    if (v) {
                        // in the value case the replacement node is of the type specified in the template
                        // and the "value" is either the surface of the "ACTUAL_X" or its children
                        if (_t_children(replacement_value)) {
                            if (is_run_node)
                                r = _t_rclone(replacement_value);
                            else
                                r = _t_clone(replacement_value);
                            r->contents.symbol = valof;
                        }
                        else {
                            r = __t_new(0,valof,_t_surface(replacement_value),_t_size(t),is_run_node);
                        }
                    }
                    else {
                        // in the structure case: first, check to see if the replacement node is an "ACTUAL_SYMBOL" in which case we try to find an actual value for that symbol in the semantic map, and if that fails, simply assume that the symbol value is the actual value.
                        T *temp = NULL;
                        if (semeq(rsid,ACTUAL_SYMBOL)) {
                            Symbol acsym = *(Symbol *)_t_surface(replacement_value);
                            T *ac = _t_news(0,ACTUAL_SYMBOL,acsym);
                            T *x = __t_find_actual(sem_map,ACTUAL_VALUE,ac);
                            _t_free(ac);
                            if (x) {
                                replacement_value = _t_child(x,1);
                            }
                            else replacement_value = temp = _t_new_root(acsym);
                        }
                        else if (semeq(rsid,ACTUAL_VALUE)) {
                            replacement_value = _t_child(replacement_value,1);
                        }
                        if (is_run_node)
                            r = _t_rclone(replacement_value);
                        else
                            r = _t_clone(replacement_value);
                        if (temp) _t_free(temp);
                    }
                }
                if (children) {
                    T *t;
                    while(t = _t_detach_by_idx(children,1))
                        _t_add(r,t);
                    _t_free(children);
                }
                _t_replace_node(template,r);
                break;
            }
        }
    }
    else {
        DO_KIDS(template,_t_fill_template(_t_child(template,i),sem_map));
    }
}

/******************** Node data accessors */
/**
 * get the number of children of a given node
 *
 * @param[in] t the node
 * @returns number of children
 */
int _t_children(T *t) {
    return t->structure.child_count;
}

/**
 * get the data of a given node
 *
 * @param[in] t the node
 * @returns pointer to node's surface
 */
void * _t_surface(T *t) {
    if (t->context.flags & (TFLAG_ALLOCATED|TFLAG_SURFACE_IS_TREE|TFLAG_SURFACE_IS_SCAPE|TFLAG_SURFACE_IS_CPTR))
        return t->contents.surface;
    else
        return &t->contents.surface;
}

/**
 * Get a tree node's semantic symbol
 *
 * @param[in] t the node
 * @returns Symbol for the given node
 */
Symbol _t_symbol(T *t) {
    return t->contents.symbol;
}

/**
 * Get a tree node's surface size
 *
 * @param[in] t the node
 * @returns size
 */
size_t _t_size(T *t) {
    return t->contents.size;
}

/*****************  Tree navigation */

/**
 * Get a tree node's nth child
 *
 * @param[in] t the node
 * @param[in] i desired child
 * @returns child or NULL if that child doesn't exist
 */
T *_t_child(T *t,int i) {
    if (i>t->structure.child_count || i < 1) return 0;
    return t->structure.children[i-1];
}

/**
 * Get a tree node's parent
 *
 * @param[in] t the node
 * @returns parent or NULL if node is root
 */
T * _t_parent(T *t) {
    return t->structure.parent;
}

/**
 * Get a tree node's root parent
 *
 * @param[in] t the node
 * @returns root node by walking up the parents
 */
T * _t_root(T *t) {
    T *p;
    while ((p = _t_parent(t)) != 0) t = p;
    return t;
}

/**
 * Get a tree node's index position
 *
 * @param[in] t the node
 * @returns the index of the node
 */
int _t_node_index(T *t) {
    int c;
    int i;
    T *p = _t_parent(t);
    if (p==0) return 0;
    c = _t_children(p);
    for(i=0;i<c;i++) {
        if (p->structure.children[i] == t) {
            return i+1;
        }
    }
    return 0;
}

/**
 * Get a tree node's index position
 *
 * @param[in] t the node
 * @returns the index of the node
 *
 * @todo improve algorithm as this is very expensive if called all the time.
 */
T * _t_next_sibling(T *t) {
    int c;
    int i;
    T *p = _t_parent(t);
    if (p==0) return 0;
    c = _t_children(p);
    for(i=0;i<c;i++) {
        if (p->structure.children[i] == t) {
            i++;
            return i<c ? p->structure.children[i] : 0;
        }
    }
    return 0;
}

/**
 * search the children of a tree for symbol matching sym
 * @param[in] t the tree
 * @param[in] sym the SemanticID to search for
 * @returns the found node or NULL
 */
T *_t_find(T *t,SemanticID sym) {
    T *p;
    DO_KIDS(t,
            if (semeq(_t_symbol(p=_t_child(t,i)),sym)) return p;
            );
    return NULL;
}


/*****************  Tree path based accesses */
/**
 * compare equality of two paths
 *
 * @param[in] p1 first path
 * @param[in] p2 second path
 * @returns 1 if equal 0 if not
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/tree_spec.h testTreePathEqual
 */
int _t_path_equal(int *p1,int *p2){
    while(*p1 != TREE_PATH_TERMINATOR && *p2 != TREE_PATH_TERMINATOR)
        if (*(p1++) != *(p2++)) return 0;
    return *p1 == TREE_PATH_TERMINATOR && *p2 == TREE_PATH_TERMINATOR;
}

/**
 * return the depth of a given path
 *
 * @param[in] p path
 * @returns int value of depth
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/tree_spec.h testTreePathDepth
 */
int _t_path_depth(int *p) {
    int i=0;
    while(*p++ != TREE_PATH_TERMINATOR) i++;
    return i;
}

/**
 * return the tree path of a given node
 *
 * this function allocates a buffer to hold a path of the given node which
 * it calculates by walkinga back up the tree
 *
 * @todo  implement jumping into orthogonal trees  (i.e. return paths with 0 in them)
 * @param[in] t tree node
 * @returns path of node
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/tree_spec.h testTreePathGetPath
 */
int * _t_get_path(T *t) {
    if (!t) return NULL;
    T *n;
    // allocate an array to hold the
    int s = sizeof(int)*20; // assume most trees are shallower than 10 nodes to prevent realloc
    int *p = malloc(s);
    int j,k,l,i=0,temp;

    // store the children's path index values into the array as we walk back up the tree to the root
    for(n=t;n;) {
        p[i] = _t_node_index(n);
        n =_t_parent(n);
        if (++i >= s) {
            s*=2;p=realloc(p,s);} // realloc array if tree too deep
    }
    if (i > 2) {
        // reverse the list by swapping elements going from the outside to the center
        i-=2;
        l = i+1;
        k = i/2+1;
        for(j=0;j<k;j++) {
            temp = p[j];
            p[j] = p[i];
            p[i--] = temp;
        }
    }
    else l = i-1;
    p[l]= TREE_PATH_TERMINATOR;
    return p;
}

/**
 * copy a path
 *
 * @param[inout] dst_p pointer to buffer holding destination path
 * @param[in] src_p path
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/tree_spec.h testTreePathCopy
 */
void _t_pathcpy(int *dst_p,int *src_p) {
    while(*src_p != TREE_PATH_TERMINATOR) {
        *dst_p++ = *src_p++;
    }
    *dst_p = TREE_PATH_TERMINATOR;
}

/**
 * get a node by path
 *
 * @param[in] t the tree to search
 * @param[in] p the path to search for
 * @returns pointer to a T
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/tree_spec.h testTreePathGet
 */
T * _t_get(T *t,int *p) {
    int i = *p++;
    T *c;
    if (i == TREE_PATH_TERMINATOR)
        return t;
    else if (i == 0) {
        if (!(t->context.flags & TFLAG_SURFACE_IS_TREE)) {
            raise_error("surface is not a tree!");
        }
        c = (T *)(_t_surface(t));
    }
    else
        c = _t_child(t,i);
    if (c == NULL ) return NULL;
    if (*p == TREE_PATH_TERMINATOR) return c;
    return _t_get(c,p);
}

/**
 * get a node by path using variable arguments.
 *
 * @param[in] t the tree to search
 * @returns pointer to a T
 *
 * note: the last argument MUST be TREE_PATH_TERMINATOR
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/tree_spec.h testTreePathGet
 */
T * _t_getv(T *t,...) {
    int p[100];
    va_list ap;
    va_start (ap, t);
    int i = 0;
    while((p[i] = va_arg(ap,int)) != TREE_PATH_TERMINATOR) {
        if (i++ == 100) raise_error("tree path to deep");
    }
    va_end(ap);
    return _t_get(t,p);
}

/**
 * get the surface of a node by path
 *
 * @param[in] t the tree to search
 * @param[in] p the path to search for
 * @returns pointer to surface or NULL if path not found
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/tree_spec.h testTreePathGetSurface
 */
void * _t_get_surface(T *t,int *p) {
    T *c = _t_get(t,p);
    if (c == NULL) return NULL;
    return _t_surface(c);
}

/**
 * print a path to a string
 *
 * @param[in] p the path to print
 * @param[inout] buf buffer to copy the string text to
 * @returns pointer to the bufer
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/tree_spec.h testTreePathSprint
 */
char * _t_sprint_path(int *fp,char *buf) {
    char *b = buf;
    int d=_t_path_depth(fp);
    if (d > 0) {
        int i;
        for(i=0;i<d;i++) {
            sprintf(b,"/%d",fp[i]);
            b += strlen(b);
        }
    }
    else buf[0] = 0;

    return buf;
}


/*****************  Tree hashing utilities */

/**
 * reduce a tree to a hash value
 *
 * @param[in] symbols declarations of symbols
 * @param[in] structures definitions of structures
 * @param[in] t the tree to hash
 * @returns TreeHash value
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/tree_spec.h testTreeHash
 */
TreeHash _t_hash(SemTable *sem,T *t) {
    int i,c = _t_children(t);
    TreeHash result;
    if (c == 0) {
        struct {Symbol s;TreeHash h;} h;
        void *surface = _t_surface(t);
        h.s = _t_symbol(t);
        //@todo fix this so we don't have keep doing this on the recursive calls...
        T *symbols = _sem_get_defs(sem,h.s);
        T *structures = _sem_get_defs(sem,h.s);
        size_t l = _d_get_symbol_size(sem,h.s,surface);
        if (l > 0)
            h.h = hashfn((char *)surface,l);
        else
            h.h = 0;
        result = hashfn((char *)&h,sizeof(h));
    }
    else {
        size_t l = sizeof(TreeHash)*c+sizeof(Symbol);
        TreeHash *h,*hashes = malloc(l);
        h = hashes;
        for(i=1;i<=c;i++) {
            *(h++) = _t_hash(sem,_t_child(t,i));
        }
        (*(Symbol *)h) = _t_symbol(t);
        result = hashfn((char *)hashes,l);
        free(hashes);
    }
    return result;
}

/**
 * comparison function for hash tree equality
 *
 * we have this because TreeHash may become a larger structure
 * not subject to direct equality testing.
 */
int _t_hash_equal(TreeHash h1,TreeHash h2) {
    return h1 == h2;
}

// scaffolding for uuid generator
// for now we just use the current time
UUIDt __uuid_gen() {
    UUIDt u;
    struct timespec c;
    clock_gettime(CLOCK_MONOTONIC, &c);
    u.time = ((c.tv_sec * (1000000)) + (c.tv_nsec / 1000));
    u.data = 0;
    return u;
}

int __uuid_equal(UUIDt *u1,UUIDt *u2) {
    return memcmp(u1,u2,sizeof(UUIDt))==0;
};
/*****************  Tree serialization */

/// macro to write data by type into *bufferP and increment offset by the size of the type
#define SWRITE(type,value) type * type##P = (type *)(*bufferP +offset); *type##P=value;offset += sizeof(type);

/**
 * Serialize a tree by recursive descent.
 *
 * @param[in] d definitions
 * @param[in] t tree to be serialized
 * @param[in] bufferP a pointer to a malloced ptr of "current_size" which will be realloced if serialized tree is bigger than initial buffer allocation.
 * @param[in] offset current offset into buffer at which to put serialized data
 * @param[in] current_size size of buffer
 * @param[in] compact boolean to indicate whether to add in extra information
 * @returns the length that was added to the buffer
 *
 * @todo compact is really a shorthand for whether this is a fixed size tree or not
 * this should actually be determined on the fly by looking at the structure types.
 */
size_t __t_serialize(SemTable *sem,T *t,void **bufferP,size_t offset,size_t current_size,int compact){
    size_t cl =0,l = _t_size(t);
    int i, c = _t_children(t);

    //    printf("\ncurrent_size:%ld offset:%ld  size:%ld symbol:%s",current_size,offset,l,_sem_get_name(sem,_t_symbol(t)));
    while ((offset+l+sizeof(Symbol)) > current_size) {
        current_size*=2;
        *bufferP = realloc(*bufferP,current_size);
    }
    if (!compact) {
        Symbol s = _t_symbol(t);
        SWRITE(Symbol,s);
        SWRITE(int,c);
    }
    if (l) {
        memcpy(*bufferP+offset,_t_surface(t),l);
        offset += l;
    }

    for(i=1;i<=c;i++) {
        offset = __t_serialize(sem,_t_child(t,i),bufferP,offset,current_size,compact);
    }
    return offset;
}


/**
 * Serialize a tree.
 *
 * @param[in] d definitions
 * @param[in] t tree to be serialized
 * @param[inout] surfaceP a pointer to a buffer that will be malloced
 * @param[inout] lengthP the serialized length of the tree
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/tree_spec.h testTreeSerialize
 */
void _t_serialize(SemTable *sem,T *t,void **surfaceP,size_t *lengthP) {
    size_t buf_size = 1000;
    *surfaceP = malloc(buf_size);
    *lengthP = __t_serialize(sem,t,surfaceP,0,buf_size,0);
    *surfaceP = realloc(*surfaceP,*lengthP);
}

/// macro to read typed date from the surface and update length and surface values
#define SREAD(type,var_name) type var_name = *(type *)*surfaceP;*lengthP -= sizeof(type);*surfaceP += sizeof(type);
/// macro to read typed date from the surface and update length and surface values (assumes variable has already been declared)
#define _SREAD(type,var_name) var_name = *(type *)*surfaceP;*lengthP -= sizeof(type);*surfaceP += sizeof(type);

T * _t_unserialize(SemTable *sem,void **surfaceP,size_t *lengthP,T *t) {
    size_t size;

    SREAD(Symbol,s);
    //    printf("\nSymbol:%s",_sem_get_name(sem,s));

    SREAD(int,c);

    Structure st = _sem_get_symbol_structure(sem,s);

    if (is_sys_structure(st)) {
        size = _sys_structure_size(st.id,*surfaceP);
        if (size == -1) {raise_error("BANG!");}
    }
    else size = 0;
    if (size > 0) {
        //    printf(" reading: %ld bytes\n",size);
        if (semeq(st,INTEGER))
            t = _t_newi(t,s,*(int *)*surfaceP);
        else
            t = _t_new(t,s,*surfaceP,size);
        *lengthP -= size;
        *surfaceP += size;
    }
    else {
        t = _t_newr(t,s);
    }

    int i;
    for(i=1;i<=c;i++) {
        _t_unserialize(sem,surfaceP,lengthP,t);
    }
    return t;
}


#define _add_char2buf(c,buf) *buf=c;buf++;*buf=0

#define _add_sem(buf,s) sprintf(buf,"{ \"ctx\":%d,\"type\":%d,\"id\":%d }",s.context,s.semtype,s.id);
/**
 * dump tree in raw JSON format
 *
 * @param[in] sem the semantic context
 * @param[in] t the tree to dump
 * @param[in] level current level used for recursion (call with 0)
 * @param[in,out] buf buffer to dump into
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/tree_spec.h testTreeJSON
 */
char * _t2rawjson(SemTable *sem,T *t,int level,char *buf) {
    char *result = buf;
    if (!t) return "";
    Symbol s = _t_symbol(t);
    char b[255];
    char tbuf[2000];
    int i;
    char *c,cr;
    Xaddr x;
    buf = _indent_line(level,buf);

    sprintf(buf,"{\"sem\":");
    buf+= strlen(buf);
    _add_sem(buf,s);
    buf+= strlen(buf);
    if (is_symbol(s)) {
        Structure st = _sem_get_symbol_structure(sem,s);

        if (is_sys_structure(st)) {
            switch(st.id) {
            case ENUM_ID: // for now enum surfaces are just strings so we can see the text value
            case CSTRING_ID:
                //@todo add escaping of quotes, carriage returns, etc...
                sprintf(buf,",\"surface\":\"\%s\"",(char *)_t_surface(t));
                break;
            case CHAR_ID:
                cr = *(char *)_t_surface(t);
                if (cr == '"') {
                    sprintf(buf,",\"surface\":\"\\\"\"");
                } else {
                    sprintf(buf,",\"surface\":\"%c\"",cr);
                }
                break;
            case BIT_ID:
                sprintf(buf,",\"surface\":%s",(*(int *)_t_surface(t)) ? "1" : "0");
                break;
            case BLOB_ID:
                raise_error("not implemented");
                break;
            case INTEGER_ID:
                sprintf(buf,",\"surface\":%d",*(int *)_t_surface(t));
                break;
            case FLOAT_ID:
                sprintf(buf,",\"surface\":%f",*(float *)_t_surface(t));
                break;
            case SYMBOL_ID:
            case STRUCTURE_ID:
            case PROCESS_ID:
            case PROTOCOL_ID:
            case RECEPTOR_ID:
                {
                    SemanticID sem =*(SemanticID *)_t_surface(t);
                    sprintf(buf,",\"surface\":");
                    buf+= strlen(buf);
                    _add_sem(buf,sem);
                }
                break;
            case TREE_PATH_ID:
                sprintf(buf,",\"surface\":\"%s\"",_t_sprint_path((int *)_t_surface(t),b));
                break;
            case XADDR_ID:
                x = *(Xaddr *)_t_surface(t);
                sprintf(buf,",\"surface\":{ \"symbol\":\"%s\",\"addr\":%d }",_sem_get_name(sem,x.symbol),x.addr);
                break;
            case CPOINTER_ID:
                sprintf(buf,",\"surface\":\"%p\"",_t_surface(t));
                break;
            case TREE_ID:
                if (t->context.flags & TFLAG_SURFACE_IS_TREE) {
                    c = _t2rawjson(sem,(T *)_t_surface(t),0,tbuf);
                    sprintf(buf,",\"surface\":%s",c);
                    break;
                }
            case RECEPTOR_SURFACE_ID:
                if (t->context.flags & (TFLAG_SURFACE_IS_TREE+TFLAG_SURFACE_IS_RECEPTOR)) {
                    c = _t2rawjson(sem,((Receptor *)_t_surface(t))->root,0,tbuf);
                    sprintf(buf,",\"surface\":%s",c);
                    break;
                }
            case SCAPE_ID:
                if (t->context.flags & TFLAG_SURFACE_IS_SCAPE) {
                    Scape *sc = (Scape *)_t_surface(t);
                    //TODO: fixme!
                    raise_error("not-implemented");
                    sprintf(buf,"(key %s,data %s",_sem_get_name(sem,sc->key_source),_sem_get_name(sem,sc->data_source));
                    break;
                }
            default:
                // other structures are composed so work automatically
                if (st.id > _t_children(__sem_get_defs(sem,SEM_TYPE_STRUCTURE,st.context)))
                    raise_error("don't know how to convert surface of %s, structure id %d seems invalid",_sem_get_name(sem,s),st.id);

            }
        }
    }
    buf += strlen(buf);
    int _c = _t_children(t);
    if ( _c > 0) {
        sprintf(buf,",\"children\":[");
        buf += strlen(buf);
        for(i=1;i<=_c;i++){
            _t2rawjson(sem,_t_child(t,i),level < 0 ? level-1 : level+1,buf);
            buf += strlen(buf);
            if (i<_c) {
                _add_char2buf(',',buf);
            }
        }
        _add_char2buf(']',buf);
    }
    _add_char2buf('}',buf);
    return result;
}

/**
 * dump tree in JSON format
 *
 * @param[in] sem the semantic context
 * @param[in] t the tree to dump
 * @param[in] level current level used for recursion (call with 0)
 * @param[in,out] buf buffer to dump into
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/tree_spec.h testTreeJSON
 */
char * _t2json(SemTable *sem,T *t,int level,char *buf) {
    char *result = buf;
    if (!t) return "";
    Symbol s = _t_symbol(t);
    char b[255];
    char tbuf[2000];
    int i;
    char *c,cr;
    Xaddr x;
    buf = _indent_line(level,buf);

    if (is_process(s)) {
        sprintf(buf,"{ \"type\":\"process\",\"name\" :\"%s\"",_sem_get_name(sem,s));
    }
    else {
        char *n = _sem_get_name(sem,s);
        Structure st = _sem_get_symbol_structure(sem,s);
        sprintf(buf,"{ \"symbol\":{ \"context\":%d,\"id\":%d },",s.context,s.id);
        buf+= strlen(buf);

        if (!is_sys_structure(st)) {
            // if it's not a system structure, it's composed, so all we need to do is
            // print out the symbol name, and the reset will take care of itself
            sprintf(buf,"\"type\":\"%s\",\"name\":\"%s\"",_sem_get_name(sem,st),n);
        }
        else {

            switch(st.id) {
            case ENUM_ID: // for now enum surfaces are just strings so we can see the text value
            case CSTRING_ID:
                //@todo add escaping of quotes, carriage returns, etc...
                sprintf(buf,"\"type\":\"CSTRING\",\"name\":\"%s\",\"surface\":\"\%s\"",n,(char *)_t_surface(t));
                break;
            case CHAR_ID:
                cr = *(char *)_t_surface(t);
                if (cr == '"') {
                    sprintf(buf,"\"type\":\"CHAR\",\"name\":\"%s\",\"surface\":\"\\\"\"",n);
                } else {
                    sprintf(buf,"\"type\":\"CHAR\",\"name\":\"%s\",\"surface\":\"%c\"",n,cr);
                }
                break;
            case BIT_ID:
                sprintf(buf,"\"type\":\"BIT\",\"name\":\"%s\",\"surface\":%s",n,(*(int *)_t_surface(t)) ? "1" : "0");
                break;
            case BLOB_ID:
                raise_error("not implemented");
                break;
            case INTEGER_ID:
                sprintf(buf,"\"type\":\"INTEGER\",\"name\":\"%s\",\"surface\":%d",n,*(int *)_t_surface(t));
                break;
            case FLOAT_ID:
                sprintf(buf,"\"type\":\"FLOAT\",\"name\":\"%s\",\"surface\":%f",n,*(float *)_t_surface(t));
                break;
            case SYMBOL_ID:
                c = _sem_get_name(sem,*(Symbol *)_t_surface(t));
                sprintf(buf,"\"type\":\"SYMBOL\",\"name\":\"%s\",\"surface\":\"%s\"",n,c?c:"<unknown>");
                break;
            case STRUCTURE_ID:
                c = _sem_get_name(sem,*(Structure *)_t_surface(t));
                sprintf(buf,"\"type\":\"STRUCTURE\",\"name\":\"%s\",\"surface\":\"%s\"",n,c?c:"<unknown>");
                break;
            case PROCESS_ID:
                c = _sem_get_name(sem,*(Process *)_t_surface(t));
                sprintf(buf,"\"type\":\"PROCESS\",\"name\":\"%s\",\"surface\":\"%s\"",n,c?c:"<unknown>");
                break;
            case PROTOCOL_ID:
                c = _sem_get_name(sem,*(Protocol *)_t_surface(t));
                sprintf(buf,"\"type\":\"PROTOCOL\",\"name\":\"%s\",\"surface\":\"%s\"",n,c?c:"<unknown>");
                break;
            case RECEPTOR_ID:
                c = _sem_get_name(sem,*(SemanticID *)_t_surface(t));
                sprintf(buf,"\"type\":\"RECEPTOR\",\"name\":\"%s\",\"surface\":\"%s\"",n,c?c:"<unknown>");
                break;
            case TREE_PATH_ID:
                sprintf(buf,"\"type\":\"TREE_PATH\",\"name\":\"%s\",\"surface\":\"%s\"",n,_t_sprint_path((int *)_t_surface(t),b));
                break;
            case XADDR_ID:
                x = *(Xaddr *)_t_surface(t);
                sprintf(buf,"\"type\":\"XADDR\",\"name\":\"%s\",\"surface\":{ \"symbol\":\"%s\",\"addr\":%d }",n,_sem_get_name(sem,x.symbol),x.addr);
                break;
            case CPOINTER_ID:
                sprintf(buf,"\"type\":\"CPOINTER\",\"name\":\"%s\",\"surface\":\"%p\"",n,_t_surface(t));
                break;
            case TREE_ID:
                if (t->context.flags & TFLAG_SURFACE_IS_TREE) {
                    c = _t2json(sem,(T *)_t_surface(t),0,tbuf);
                    sprintf(buf,"\"type\":\"TREE\",\"name\":\"%s\",\"surface\":%s",n,c);
                    break;
                }
            case RECEPTOR_SURFACE_ID:
                if (t->context.flags & (TFLAG_SURFACE_IS_TREE+TFLAG_SURFACE_IS_RECEPTOR)) {
                    c = _t2json(sem,((Receptor *)_t_surface(t))->root,0,tbuf);
                    sprintf(buf,"\"type\":\"RECEPTOR\",\"name\":\"%s\",\"surface\":%s",n,c);
                    break;
                }
            case SCAPE_ID:
                if (t->context.flags & TFLAG_SURFACE_IS_SCAPE) {
                    Scape *sc = (Scape *)_t_surface(t);
                    //TODO: fixme!
                    sprintf(buf,"(%s:key %s,data %s",n,_sem_get_name(sem,sc->key_source),_sem_get_name(sem,sc->data_source));
                    break;
                }
            default:
                if (semeq(s,SEMTREX_MATCH_CURSOR)) {
                    c = _t2json(sem,*(T **)_t_surface(t),0,tbuf);
                    //c = "null";
                    sprintf(buf,"(%s:{%s}",n,c);
                    break;
                }
                if (n == 0)
                    sprintf(buf,"(<unknown:%d.%d.%d>",s.context,s.semtype,s.id);
                else {
                    c = _sem_get_name(sem,st);
                    sprintf(buf,"\"type\":\"%s\",\"name\":\"%s\"",c,n);
                }
            }
        }
    }
    buf += strlen(buf);
    int _c = _t_children(t);
    if ( _c > 0) {
        sprintf(buf,",\"children\":[");
        buf += strlen(buf);
        for(i=1;i<=_c;i++){
            _t2json(sem,_t_child(t,i),level < 0 ? level-1 : level+1,buf);
            buf += strlen(buf);
            if (i<_c) {
                _add_char2buf(',',buf);
            }
        }
        _add_char2buf(']',buf);
    }
    _add_char2buf('}',buf);
    return result;
}

// assumes that t is a CSTRING structured tree
int __t_writeln(T *t,Stream *stream) {
    int err = 0;
    char *str = _t_surface(t);
    int len = strlen(str);
    if (len) err = _st_write(stream,str,len);
    if (!len || err > 0) {
        err = _st_write(stream,"\n",1);
        if (err > 0) err += err;
    }
    return err;
}

/**
 * write a tree out to a stream
 *
 * @param[in] sem current semantic contexts
 * @param[in] t the tree to write out
 * @param[in] stream the stream to write to
 * @returns unix error code, or number of bytes written
 */
int _t_write(SemTable *sem,T *t,Stream *stream) {
    int err;
    Symbol sym = _t_symbol(t);
    if (semeq(sym,LINE)) {
        err = __t_writeln(t,stream);
    }
    else if (semeq(sym,LINES)) {
        DO_KIDS(t,
                err = __t_writeln(_t_child(t,i),stream);
                if (err == 0) return 0;
                );
    }
    else {
        char *str;
        Structure struc = _sem_get_symbol_structure(sem,sym);
        if (semeq(struc,CSTRING)) {
            str = _t_surface(t);
        }
        else {
            str = _t2s(sem,t);
        }
        err = _st_write(stream,str,strlen(str));
    }
    return err;
}

/** @}*/
