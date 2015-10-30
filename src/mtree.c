/**
 * @ingroup tree
 *
 * @{
 *
 * @file mtree.c
 * @brief semantic tree matrix implementation
 *
 * @copyright Copyright (C) 2013-2015, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 */

#include "mtree.h"
#include "ceptr_error.h"
#include "hashfn.h"
#include "def.h"

const H null_H = {0,{NULL_ADDR,NULL_ADDR}};

// low-level function to allocate a new tree level to an mtree
/// @todo make this not realloc each time?
void __m_add_level(M *m) {
    if (!m->levels++) {
    m->lP = malloc(sizeof(L));
    }
    else {
    m->lP = realloc(m->lP,sizeof(L)*m->levels);
    }
    int i = m->levels-1;
    m->lP[i].nodes = 0;
}

// low-level function to add c nodes to given level
/// @todo make this not realloc each time!!
N *__m_add_nodes(H h,L *l,int c) {
    N *n;
    Mindex i = l->nodes;
    if (!i) {
    size_t s = sizeof(N)*c;
        l->nP = malloc(s);
    memset(l->nP,0,s);
    l->nodes = c;
    }
    else {
    //  size_t os = sizeof(N)*l->nodes;
    l->nodes += c;
    size_t ns = sizeof(N)*l->nodes;
        l->nP = realloc(l->nP,ns);
    //  memset(l->nP+ns,0,sizeof(N)*c);
    }
    n = _GET_NODE(h,l,i);
    return n;
}

// low level function to initialize a new node under a parent
void __m_new_init(H parent,H *h,L **l) {
    h->m = parent.m;
    h->a.l = parent.a.l+1;

    if (parent.a.l >= parent.m->levels) {
    raise_error("address too deep!");
    }
    else if (parent.a.l == parent.m->levels-1) {
    h->a.i = 0;
    __m_add_level(h->m);
    *l = GET_LEVEL(*h);
    }
    else {
    *l = GET_LEVEL(*h);
    h->a.i = (*l)->nodes;
    }
}

// low level function to initialize a new node as a root node
void __m_new_root(H *h, L **l) {
    M *m = h->m = malloc(sizeof(M));
    m->magic = matrixImpl;
    m->levels = 0;
    h->a.l = 0;
    h->a.i = 0;
    __m_add_level(m);
    *l = GET_LEVEL(*h);
}

/**
 * Create a new tree node
 *
 * @param[in] parent handle to parent node
 * @param[in] symbol semantic symbol for the node to be create
 * @param[in] surface pointer to node's data
 * @param[in] size size in bytes of the surface
 * @param[in] flags
 * @returns updated handle
 */
H __m_new(H parent,Symbol symbol,void *surface,size_t size,uint32_t flags) {
    H h;
    L *l = 0;

    if (parent.m) {
        __m_new_init(parent,&h,&l);
    }
    else {
        __m_new_root(&h,&l);
    }

    // add a node
    N *n,*nl;
    n = __m_add_nodes(h,l,1);

    n->symbol = symbol;
    n->size = size;
    n->parenti = parent.m ? parent.a.i : NULL_ADDR;
    n->flags = flags;
    if (size) {
        if (size <= sizeof(void *)) {
            memcpy(&n->surface,surface,size);
        }
        else {
            n->flags |= TFLAG_ALLOCATED;
            n->surface = malloc(size);
            if (surface)
                memcpy(n->surface,surface,size);
        }
    }

    return h;
}

/**
 * Create a new tree
 *
 * @param[in] symbol semantic symbol for the node to be created
 * @returns handle to root node
 */
H _m_new_root(Symbol s) {
    return _m_new(null_H,s,0,0);
}

/**
 * Create a new mtree node with no surface value
 *
 * @param[in] parent parent node handle for the node to be created.  Can be 0 if this is a root node
 * @param[in] symbol semantic symbol for the node to be create
 * @returns handle to node
 */
H _m_newr(H parent,Symbol s) {
    return _m_new(parent,s,0,0);
}

/**
 * Create a new mtree node with an integer surface
 *
 * @param[in] parent parent node handle for the node to be created.  Can be 0 if this is a root node
 * @param[in] symbol semantic symbol for the node to be create
 * @param[in] surface integer value to store in the surface
 * @returns handle to node
 */
H _m_newi(H parent,Symbol symbol,int surface) {
    return _m_new(parent,symbol,&surface,sizeof(int));
}

// helper function to recursively traverse a ttree and build an mtree out of it
// used by _m_new_from_t
H __mnft(H parent,T *t) {
    int i, c = _t_children(t);

    // clear the allocated flag, because that will get recalculated in __m_new
    uint32_t flags = t->context.flags & ~TFLAG_ALLOCATED;
    // if the ttree points to a type that has an allocated c structure as its surface
    // it must be copied into the mtree as reference, otherwise it would get freed twice
    // when the mtree is freed

    if (flags & (TFLAG_SURFACE_IS_RECEPTOR+TFLAG_SURFACE_IS_SCAPE+TFLAG_SURFACE_IS_STREAM)) flags |= TFLAG_REFERENCE;
    void *surface = _t_surface(t);
    void *sp;
    if (flags & (TFLAG_SURFACE_IS_TREE+TFLAG_SURFACE_IS_SCAPE+TFLAG_SURFACE_IS_STREAM)) {
        sp = surface;
        surface = &sp;
    }

    H h = __m_new(parent,_t_symbol(t),surface,_t_size(t),flags);
    if (flags&TFLAG_RUN_NODE) {
        // @todo, make this more efficient, ie we shouldn't have to
        // do an _m_get, instead there should be a way for mtrees to create run_nodes
        N *n = __m_get(h);
        n->cur_child = ((rT *)t)->cur_child;
    }
    for(i=1;i<=c;i++) {
        __mnft(h,_t_child(t,i));
    }
    return h;
}

/**
 * Create a new mtree that is a copy of a ttree
 *
 * @param[in] t pointer to source ttree
 * @returns handle to mtree
 */
H _m_new_from_t(T *t) {
    H h = null_H;
    h = __mnft(h,t);
    h.a.l = 0;
    h.a.i = 0;
    return h;
}

// mtree walk function for creating ttree nodes
// used by _t_new_from_m
void _m_2tfn(H h,N *n,void *data,MwalkState *s,Maddr ap) {

    T **tP = (T**) &(((struct {T *t;} *)data)->t);
    T *t =  h.a.l ? (s[h.a.l-1].user.t) : NULL;
    int is_run_node = (n->flags&TFLAG_RUN_NODE);

    T *nt;

    if (n->flags & TFLAG_SURFACE_IS_TREE && !(n->flags & TFLAG_SURFACE_IS_RECEPTOR)) {
        if (is_run_node) raise_error("not implemented");
        nt = _t_newt(t,n->symbol,_t_new_from_m(*(H *)n->surface));
    }
    else if (n->flags & TFLAG_ALLOCATED) {
        nt = __t_new(t,n->symbol,n->surface,n->size,is_run_node);
    }
    else {
        nt = __t_new(t,n->symbol,&n->surface,n->size,is_run_node);
    }
    nt->context.flags |= (~TFLAG_ALLOCATED)&(n->flags);

    if (is_run_node) {
        ((rT *)nt)->cur_child = n->cur_child;
    }
    *tP = nt;

    s[h.a.l].user.t = nt;
}

/**
 * Create a new ttree that is a copy of an mtree
 *
 * @param[in] h handle to source mtree
 * @returns handle to mtree
 */
T *_t_new_from_m(H h) {
    struct {T *t;} d = {NULL};
    Maddr ac = {0,0};
    _m_walk(h,_m_2tfn,&d);
    return _t_root(d.t);
}

/**
 * get an mtree node by handle
 *
 * @param[in] h the handle to an mtree node
 * @returns pointer to an N (the node)

 */
N *__m_get(H h) {
    L *l = GET_LEVEL(h);
    N *n = GET_NODE(h,l);
    return n;
}

/**
 * Get an mtree node's surface size
 *
 * @param[in] h handle to the node
 * @returns size
 */
size_t _m_size(H h) {return __m_get(h)->size;}

/**
 * free the memory used by an mtree
 *
 * @param[in] h handle to mtree to free
 * @param[in] free_surface boolean to indicate whether to free the surface values
 */
void __m_free(H h,int free_surface) {
    int i = h.m->levels;
    while(i--) {
    L *l = _GET_LEVEL(h,i);
    Mindex j = l->nodes;
    if (free_surface) {
        while(j--) {
            N *n = _GET_NODE(h,l,j);
            if (!n->flags & TFLAG_REFERENCE) {
                if (n->flags & TFLAG_SURFACE_IS_RECEPTOR) raise_error("mtree can't free receptor!");
                if (n->flags & TFLAG_SURFACE_IS_TREE && !(n->flags & TFLAG_SURFACE_IS_RECEPTOR)) {
                    _m_free(*(H *)n->surface);
                }
                if (n->flags & TFLAG_ALLOCATED) {
                    free(n->surface);
                }
            }
        }
    }
    free(l->nP);
    }
    free(h.m->lP);
    free(h.m);
}

/**
 * return the number of children of a given mtree node
 *
 * @param[in] h handle to the node
 * @returns child count
 */
int _m_children(H h) {
    Mlevel levels = h.m->levels;

    if (h.a.l >= levels) {
        raise_error("address too deep!");
    }
    else if (h.a.l == levels-1) {
        return 0;
    }
    L *l = _GET_LEVEL(h,h.a.l+1);
    Mindex c = 0;
    Mindex i = 0,pi = h.a.i;
    Mindex max = l->nodes;
    N *n = &l->nP[0];

/* this works if nodes are sorted
    while (i < max && n->parenti != h.a.i) {
    n++;i++;
    }
    while (i < max && n->parenti == h.a.i) {
    n++;i++;c++;
    }
*/
    while (i < max) {
        if (!(n->flags & TFLAG_DELETED) && pi == n->parenti) c++;
        n++;i++;
    }

    return c;
}

/**
 * get the data of a given mtree node
 *
 * @param[in] h handle to the node
 * @returns pointer to node's surface
 */
void * _m_surface(H h) {
    N *n = __m_get(h);
    if (n->flags & TFLAG_ALLOCATED)
        return n->surface;
    else
        return &n->surface;
}

/**
 * get matrix address of a mtree node's parent
 *
 * @param[in] h handle to the node
 * @returns Maddr of parent node
 */
Maddr _m_parent(H h) {
    Maddr a = {NULL_ADDR,NULL_ADDR};
    if (h.a.l > 0) {
        N *n = __m_get(h);
        a.l = h.a.l-1;
        a.i = n->parenti;
    }
    return a;
}

/**
 * get matrix address of a child of a given mtree node
 *
 * @param[in] h handle to the node
 * @param[in] c index of child
 * @returns Maddr of child
 */
Maddr _m_child(H h,Mindex c) {
    Maddr a = {NULL_ADDR,NULL_ADDR};
    Mlevel levels = h.m->levels;
    if (h.a.l >= levels) {
        raise_error("address too deep!");
    }
    else if (h.a.l == levels-1) {
        return a;
    }
    a.l = h.a.l+1;
    L *l = &h.m->lP[a.l];
    Mindex ci = 0,max = l->nodes;
    N *n = &l->nP[0];
    if (c == NULL_ADDR) {
        a.i = NULL_ADDR;
        while(ci < max) {
            if (n->parenti == h.a.i) a.i = ci;
            ci++;
            n++;
        }
        if (a.i == NULL_ADDR)
            a.l = NULL_ADDR;
    }
    else {
        a.i = 0;
        while(a.i < max) {
            if (n->parenti == h.a.i) ci++;
            if (ci == c) return a;
            a.i++;n++;
        }
        a.l = NULL_ADDR;
        a.i = NULL_ADDR;
    }
    /* works if nodes are sorted
    //skip past nodes of children of parents before our parent
    while (a.i < max && n->parenti < h.a.i) {
    a.i++;n++;
    }

    // if you pass in NULL_ADDR for the child,
    // this routine returns the last child address
    if (c == NULL_ADDR) {
    while (a.i < max && n->parenti == h.a.i) {
        a.i++;n++;
    }
    a.i--;
    if (a.i == -1) a.l = NULL_ADDR;
    }
    else {
    if (a.i+c > l->nodes) {
        raise_error("address too deep!");
    }
    a.i += c-1;
    }
*/

    return a;
}

/**
 * get the symbol of a given mtree node
 *
 * @param[in] h handle to the node
 * @returns node's Symbol
 */
Symbol _m_symbol(H h) {
    N *n = __m_get(h);
    return n->symbol;
}

/**
 * get matrix address of a node's next sibling
 *
 * @param[in] h handle to the node
 * @returns Maddr of next sibling child
 */
Maddr _m_next_sibling(H h) {
    L *l = GET_LEVEL(h);
    int i = h.a.i+1;
    Maddr r;
    N *n = GET_NODE(h,l);
    Mindex pi = n->parenti;
    while (i<l->nodes) {
        n++;
        if (n->parenti == pi) {
            r.l = h.a.l;
            r.i = i;
            return r;
        }
        i++;
    }
    return null_H.a;
}

/**
 * add an mtree into an existing tree
 *
 * @param[in] parent handle to the parent to be added to
 * @param[in] h handle to the tree to add in
 * @returns handle of newly added (sub) tree
 *
 * this function is destructive in that the task is accomplished by copying
 * the values from the tree at h, and then freeing it.
 *
 */
H _m_add(H parent,H h) {
    L *pl,*l;
    H r;
    int x = _m_children(parent)+1;
    int i,levels = h.m->levels;
    H p = parent;
    Mindex d = parent.a.i;
    for (i=0;i<levels;i++) {
        __m_new_init(p,&r,&pl);
        l = _GET_LEVEL(h,i);
        N *np = __m_add_nodes(r,pl,l->nodes);
        N *n = &l->nP[0];
        Mindex j = l->nodes;
        while (j--)  {
            *np = *n;
            if (np->parenti == NULL_ADDR) np->parenti = 0;
            np->parenti += d;
            np++; n++;
        }
        d = pl->nodes-l->nodes;
        p.a.l++;
    }
    r.a = _m_child(parent,x);

    __m_free(h,0);  //free h but not it's surfaces which were copied into the parent
    return r;
}

/**
 * walk an mtree
 *
 * @param[in] h mtree to walk
 * @param[in] walkfn function pointer to function to be executed on each node of the tree
 * @param[in] user_data data to pass to the waklfn
 *
 */
void _m_walk(H h,void (*walkfn)(H ,N *,void *,MwalkState *,Maddr),void *user_data) {
    int levels = h.m->levels;
    MwalkState state[levels];
    int i,root;
    //    for(i=0;i<levels;i++) {state[i]=0;};
    Maddr ap = _m_parent(h);
    root = h.a.l;
    int done = 0;
    /// @todo checks to make sure root isn't deleted or null?
    L *l = GET_LEVEL(h);
    N *n;
    int backup,nodes = h.a.i+1; // at root level pretend we are at last node
    //    raise(SIGINT);

    while(!done) {
    backup = 0;
    n = GET_NODE(h,l);
    // look for child of parent at this level (may be node at current handle address)
    while ((h.a.i < nodes)  && ((n->flags & TFLAG_DELETED) || (n->parenti != ap.i))) {
        n++;h.a.i++;
    };

    // if we got one, then call the walk function
    if (h.a.i != nodes) {
        (*walkfn)(h,n,user_data,state,ap);
        // and go down looking for children
        if (h.a.l+1 < levels) {
        state[h.a.l].i = h.a.i;
        ap = h.a;
        h.a.l++;
        h.a.i = 0;
        l = GET_LEVEL(h);
        nodes = l->nodes;
        }
        else {
        // if no more levels, then backup if no more nodes
        if (++h.a.i == nodes)
            backup = 1;
        }
    }
    else backup = 1;

    while(backup) {
        // if current node is at root level we are done
        if (h.a.l == root) {backup = 0;done = 1;}
        else {
        // otherwise move up a level
        h.a.l--;
        // new node index is next node at that level from stored state
        h.a.i = state[h.a.l].i+1;
        l = GET_LEVEL(h);
        nodes = l->nodes;
        // if we still have nodes to check then we have finished backing up otherwise
        // we'll loop to go back up another level
        if (h.a.i < nodes) {
            backup = 0;
            ap.l = h.a.l -1;
            ap.i = state[ap.l].i;
        }
        }
    }
    }
}

// walkfn used by _m_detach to detach a branch of a tree
void _m_detatchfn(H oh,N *on,void *data,MwalkState *s,Maddr ap) {
    struct {M *m;int l;} *d = data;

    H parent;
    H h;
    L *l;
    if (!d->m) {
        __m_new_root(&h,&l);
        parent.m = 0;
        d->m = h.m;
    }
    else {
        parent.m = d->m;
        parent.a.i = s[oh.a.l-1].user.pi;
        // we use d->l here to give us a level offset because the root of
        // the detached tree is not necessarily the same as the root of
        // the whole tree.
        parent.a.l = oh.a.l-1-d->l;
        __m_new_init(parent,&h,&l);
    }

    N *n,*nl;
    n = __m_add_nodes(h,l,1);

    // everything in the node is the same except the parenti
    *n = *on;
    on->flags = TFLAG_DELETED;
    on->surface = 0;
    on->size = 0;
    // which we got from the user portion of the state data
    n->parenti = parent.m ? parent.a.i : NULL_ADDR;
    s[oh.a.l].user.pi = l->nodes-1;

}

/**
 * detach a branch of a tree
 *
 * @param[in] oh handle of mtree node to detach from the mtree
 * @returns hande to newly detached tree
 *
 */
H _m_detatch(H oh) {
    struct {M *m;int l;} d = {NULL,oh.a.l};
    _m_walk(oh,_m_detatchfn,&d);
    H h = {d.m,{0,0}};
    return h;
}

/**
 * create a serialized version of an mtree
 *
 * @param[in] oh handle of mtree node to detach from the mtree
 * @param[inout] sizeP pointer to size_t value to return length of serialized data
 * @returns pointer to newly malloced buffer of serialized tree data
 */
S *_m_serialize(M *m) {

    uint32_t s_size = SERIALIZED_HEADER_SIZE(m->levels);
    uint32_t levels_size = 0;
    size_t blob_size = 0;

    Mindex i;
    H h = {m,{0,0}};

    // calculate level and blob sizes so we can allocate
    for(h.a.l=0; h.a.l<m->levels; h.a.l++) {
    L *l = GET_LEVEL(h);

    levels_size +=  SERIALIZED_LEVEL_SIZE(l);

    for(h.a.i=0;h.a.i < l->nodes;h.a.i++) {
        N *n = GET_NODE(h,l);
        blob_size+=n->size;
    }
    }

    size_t total_size = s_size+levels_size+blob_size;
    S *s = malloc(total_size);
    memset(s,0,total_size);
    s->magic = m->magic;
    s->total_size = total_size;
    s->levels = m->levels;
    s->blob_offset = s_size+levels_size;

    void *blob = s->blob_offset + (void *)s;

    levels_size = 0;
    blob_size = 0;

    for(h.a.l=0; h.a.l<m->levels; h.a.l++) {
    s->level_offsets[h.a.l] = levels_size;
    L *sl = (L *) (((void *)s) + s_size + levels_size);

    L *l = GET_LEVEL(h);
    levels_size +=  SERIALIZED_LEVEL_SIZE(l);

    sl->nodes = l->nodes;

    N *sn = sizeof(Mindex)+(void *)sl;
    for(h.a.i=0;h.a.i < l->nodes;h.a.i++) {
        N *n = GET_NODE(h,l);
        *sn = *n;
        //      raise(SIGINT);

        if (n->flags & TFLAG_ALLOCATED) {
            *(size_t *)&sn->surface = blob_size;
            memcpy(blob+blob_size,n->surface,n->size);
            blob_size+=n->size;
        }
        else {
            memcpy(&sn->surface,&n->surface,n->size);
        }

        sn = (N *) (SERIALIZED_NODE_SIZE + ((void*)sn));
    }
    }
    return s;
}


/**
 * build mtree from serialized mtree data
 *
 * @params[in] pointer to serialized data
 * @returns handle to new mtree
 *
 */
H _m_unserialize(S *s) {
    M *m = malloc(sizeof(M));
    m->magic = s->magic;
    m->levels = s->levels;
    m->lP = malloc(sizeof(L)*m->levels);
    H h = {m,{0,0}};
    void *blob = s->blob_offset + (void *)s;

    uint32_t s_size = SERIALIZED_HEADER_SIZE(m->levels);
    for(h.a.l=0; h.a.l<m->levels; h.a.l++) {
        L *sl = (L *) (((void *)s) + s_size + ((S *)s)->level_offsets[h.a.l]);
        L *l = GET_LEVEL(h);
        l->nodes = sl->nodes;
        l->nP = malloc(sizeof(N)*l->nodes);
        N *sn = sizeof(Mindex)+(void *)sl;
        for(h.a.i=0;h.a.i < l->nodes;h.a.i++) {
            N *n = GET_NODE(h,l);
            *n = *sn;
            void *surface = blob+*(size_t *)&sn->surface;
            if (n->flags & TFLAG_ALLOCATED) {
                n->surface = malloc(sn->size);
                memcpy(n->surface,surface,sn->size);
            }
            else {
                memcpy(&n->surface,&sn->surface,sn->size);
            }
            sn = (N *) (SERIALIZED_NODE_SIZE + ((void*)sn));
        }
    }
    h.a.i = h.a.l = 0;
    return h;
}


/** @}*/
