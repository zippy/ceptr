/**
 * @ingroup semtrex
 *
 * @{
 * @file semtrex.c
 * @brief semtrex implementation
 *
 * This file implements a regular expression type language usefull for searching semantic trees.
 *
 * The code to generate finite state automata to match the trees is based heavily on Russ Cox's
 * great work see https://github.com/zippy/ceptr/blob/master/articles/cox2007.pdf
 *
 * @copyright Copyright (C) 2013-2016, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 */

#include "semtrex.h"
#include "def.h"
#include "debug.h"

/// the final matching state in the FSA can be declared statically and globally
SState matchstate = {NULL,0,StateMatch}; /* only one instance of the match state*/

char *G_s_str[]={"StateSymbol","StateAny","StateValue","StateSplit","StateMatch","StateGroupOpen","StateGroupClose","StateDescend","StateWalk","StateNot"};


/**
 * Since the out pointers in the list are always
 * uninitialized, we use the pointers themselves
 * as storage for the Ptrlists.
 */
typedef union Ptrlist Ptrlist;
union Ptrlist
{
    Ptrlist *next;
    SState *s;
};

/**
 * Create singleton list containing just outp.
*/
Ptrlist*
list1(SState **outp)
{
    Ptrlist *l;

    l = (Ptrlist*)outp;
    l->next = NULL;
    return l;
}

/**
 * Patch the list of states at out to point to start.
 */
void
patch(Ptrlist *l, SState *s,int level)
{
    Ptrlist *next;

    for(; l; l=next){
        next = l->next;

        // This is a wicked trick:  The Ptrlist will always be the address of the
        // SState *out element of the SState structure.  Thus, we can get access to the
        // transition by simply adding 1 to the pointer.  Then we can adjust the the transition
        // pop value by adding in the current level.
        TransitionType *tr = (TransitionType *)(l+1);
        // @todo fix this bogus pointer math (maybe by converting TransitionType to a struct?)
        StateType type = *(StateType *)(sizeof(TransitionType)+(char *)tr);
debug(D_STX_BUILD,"patching %s to %s with input:%d states tr:%d\n",G_s_str[type],G_s_str[s->type],(int)level,(int)*tr);
        if (*tr != TransitionNone) {
        //        if (isTransitionUp(transition)) {
            // adjust the level so we pop to the right place
            *tr += -level;
            // if transition is to next, make sure it's because the source state actually
            // consumed a node
            if (*tr==0  && ((type != StateSymbol) && (type != StateAny) && (type != StateValue)))
                *tr = TransitionNone;
        }
        l->s = s;
    }
}

/**
 * Join the two lists l1 and l2, returning the combination.
 */
Ptrlist*
append(Ptrlist *l1, Ptrlist *l2)
{
    Ptrlist *oldl1;

    oldl1 = l1;
    while(l1->next)
        l1 = l1->next;
    l1->next = l2;
    return oldl1;
}


//#define state(t,sP) _state(t,sP,0)
/**
 * utility routine to initialize a state struct
 */
SState *state(StateType type,int *statesP,int level) {
    SState *s = malloc(sizeof(SState));
    s->out = NULL;
    s->out1 = NULL;
    s->transition = level;
    s->transition1 = level;
    s->type_ = s->type = type;
    s->_did = 0;
    (*statesP)++;
    return s;
}

int G_group_id;
/**
 * Given a Semtrex tree, build a partial FSA (returned via in as a pointer to the starting state, a list of output states, and a count of the total number of states created).
 */
char * __stx_makeFA(T *t,SState **in,Ptrlist **out,int level,int *statesP) {
    SState *s,*i,*last,*s1,*s2;
    Ptrlist *o,*o1;
    char *err;
    int state_type = -1;
    int x;
    SemanticID group_symbol;
    int group_id;
    T *v;

    TransitionType tr;

    int c = _t_children(t);
    Symbol sym = _t_symbol(t);
    switch(sym.id) {
    case SEMTREX_VALUE_LITERAL_ID:
    case SEMTREX_VALUE_LITERAL_NOT_ID:
        debug(D_STX_BUILD,"SYM=\n");
        state_type = StateValue;
        s = state(state_type,statesP,level);
        s->data.value.flags = (sym.id == SEMTREX_VALUE_LITERAL_NOT_ID) ? LITERAL_NOT : 0;
        // copy the value set (which must be the first child) from the semtrex into the state
        v = _t_child(t,1);
        if (!v) {
            raise_error("expecting value or SEMTREX_VALUE_SET as first child of SEMTREX_VALUE_LITERAL");
        }
        if (semeq(_t_symbol(v),SEMTREX_VALUE_SET)) s->data.value.flags |= LITERAL_SET;

        s->data.value.values = _t_clone(v);
        *in = s;
        *out = list1(&s->out);
        break;
    case SEMTREX_SYMBOL_LITERAL_ID:
    case SEMTREX_SYMBOL_LITERAL_NOT_ID:
        debug(D_STX_BUILD,"SYM\n");
        state_type = StateSymbol;

        v = _t_child(t,1);
        int is_set;
        Symbol vsym = _t_symbol(v);
        if (!v || !((is_set = semeq(SEMTREX_SYMBOL_SET,vsym)) || semeq(SEMTREX_SYMBOL,vsym))) {
            raise_error("expecting SEMTREX_SYMBOL_SET or SEMTREX_SYMBOL as first child of SEMTREX_SYMBOL_LITERAL");
        }
        if (c > 2) return "Symbol literal must have 0 or 1 children other than the symbol/set";
        s = state(state_type,statesP,level);
        s->data.symbol.flags = (sym.id == SEMTREX_SYMBOL_LITERAL_NOT_ID) ? LITERAL_NOT : 0;
        if (is_set) s->data.symbol.flags |= LITERAL_SET;
        s->data.symbol.symbols = _t_clone(v);
        *in = s;
        if (c > 1) {
            err = __stx_makeFA(_t_child(t,2),&i,&o,level-1,statesP);
            if (err) return err;
            s->out = i;
            s->transition = TransitionDown;
            *out = o;
        }
        else {
            *out = list1(&s->out);
        }
        break;
    case SEMTREX_SYMBOL_ANY_ID:
        debug(D_STX_BUILD,"ANY\n");
        state_type = StateAny;
        if (c > 1) return "Symbol any must have 0 or 1 children";

        s = state(state_type,statesP,level);

        *in = s;
        if (c > 0) {
            err = __stx_makeFA(_t_child(t,1),&i,&o,level-1,statesP);
            if (err) return err;
            s->out = i;
            s->transition = TransitionDown;
            *out = o;
        }
        else {
            *out = list1(&s->out);
        }
        break;
    case SEMTREX_SEQUENCE_ID:
        debug(D_STX_BUILD,"SEQ\n");
        if (c == 0) return "Sequence must have children";
        last = 0;
        for(x=c;x>=1;x--) {
            err = __stx_makeFA(_t_child(t,x),&i,&o,level,statesP);
            if (err) return err;
            if (last) patch(o,last,level);
            else *out = o;
            last = i;
            *in = i;
        }
        break;
    case SEMTREX_OR_ID:
        debug(D_STX_BUILD,"OR\n");
        if (c != 2) return "Or must have 2 children";
        s = state(StateSplit,statesP,TransitionNone);
        *in = s;
        err = __stx_makeFA(_t_child(t,1),&i,&o,level,statesP);
        if (err) return err;
        s->out = i;
        err = __stx_makeFA(_t_child(t,2),&i,&o1,level,statesP);
        if (err) return err;
        s->out1 = i;
        *out = append(o,o1);
        break;
    case SEMTREX_ZERO_OR_MORE_ID:
        debug(D_STX_BUILD,"*\n");
        if (c != 1) raise_error("expecting 1 child for SEMTREX_ZERO_OR_MORE got %d, %s\n",c,t2s(t));

        if (c != 1) return "Star must have 1 child";
        s = state(StateSplit,statesP,level);
        *in = s;
        err = __stx_makeFA(_t_child(t,1),&i,&o,level,statesP);
        if (err) return err;
        s->out = i;
        s->transition = TransitionNone;
        patch(o,s,level);
        *out = list1(&s->out1);
        break;
    case SEMTREX_ONE_OR_MORE_ID:
        debug(D_STX_BUILD,"+\n");
        if (c != 1) return "Plus must have 1 child";
        s = state(StateSplit,statesP,level);
        err = __stx_makeFA(_t_child(t,1),&i,&o,level,statesP);
        if (err) return err;
        *in = i;
        s->out = i;
        s->transition = TransitionNone;
        patch(o,s,level);
        *out = list1(&s->out1);
        break;
    case SEMTREX_ZERO_OR_ONE_ID:
        debug(D_STX_BUILD,"?\n");
        if (c != 1) return "Question must have 1 child";
        s = state(StateSplit,statesP,level);
        *in = s;
        err = __stx_makeFA(_t_child(t,1),&i,&o,level,statesP);
        if (err) return err;
        s->out = i;
        s->transition = TransitionNone;
        *out = append(o,list1(&s->out1));
        break;
    case SEMTREX_GROUP_ID:
        debug(D_STX_BUILD,"GROUP\n");
        if (c != 1) return "Group must have 1 child";
        s = state(StateGroupOpen,statesP,TransitionNone);
        *in = s;
        group_symbol = *(SemanticID *)_t_surface(t);
        group_id = ++G_group_id;
        s->data.groupo.symbol = group_symbol;
        s->data.groupo.uid = group_id;
        err = __stx_makeFA(_t_child(t,1),&i,&o,level,statesP);
        if (err) return err;
        s->out = i;
        s1 = state(StateGroupClose,statesP,TransitionNone);
        patch(o,s1,level);
        s1->data.groupc.openP = s;
        *out = list1(&s1->out);
        break;
    case SEMTREX_DESCEND_ID:
        debug(D_STX_BUILD,"DESCEND\n");
        if (c != 1) return "Descend must have 1 child";
        s = state(StateDescend,statesP,TransitionDown);
        *in = s;
        err = __stx_makeFA(_t_child(t,1),&i,&o,level-1,statesP);
        if (err) return err;
        s->out = i;
        *out = o;
        break;
    case SEMTREX_NOT_ID:
        debug(D_STX_BUILD,"NOT\n");
        if (c != 1) return "Not must have 1 child";
        s = state(StateNot,statesP,TransitionNone);
        *in = s;
        err = __stx_makeFA(_t_child(t,1),&i,&o,level,statesP);
        if (err) return err;
        s->out = i;
        *out = append(o,list1(&s->out1));
        break;
    case SEMTREX_WALK_ID:
        debug(D_STX_BUILD,"WALK\n");
        if (c != 1) return "Walk must have 1 child";
        s = state(StateWalk,statesP,level);
        *in = s;
        err = __stx_makeFA(_t_child(t,1),&i,&o,level,statesP);
        if (err) return err;
        s->out = i;
        *out = o;
        break;
    default:
        return "Unknown SEMTREX SYMBOL";
    }
    if (debugging(D_STX_BUILD)) {
        char buf[20000];
        int x = level*-1;
        debug(D_STX_BUILD,"%d:%.*s%s\n",x,x,"_______________________",_stx_dump(*in,buf));
    }

    return 0;
}

/**
 * wrapper function for building the finite state automata recursively and patching it to the final match state
 */
SState * _stx_makeFA(T *t,int *statesP) {
    SState *in;
    Ptrlist *o;
    G_group_id = 0;
    char *err = __stx_makeFA(t,&in,&o,0,statesP);
    if (err != 0) {raise_error("%s",err);}
    patch(o,&matchstate,0);
    //    printf("\n");_stx_dump(in);
    return in;
}

static int free_id = 0;

/**
 * walks through the FSA clearing pointers that create loops
 */
int __stx_freeFA(SState *s,int id) {
    if ((s->_did != id) && (s != &matchstate)) {
        s->_did = id;
        if (s->out) if (__stx_freeFA(s->out,id)) s->out = 0;
        if (s->out1) if (__stx_freeFA(s->out1,id)) s->out1 = 0;
        return 0;
    }
    return 1;
}

/**
 * walk through FSA freeing the states, assumes __stx_freeFA has been called first so as not to go into loops!
 */
void __stx_freeFA2(SState *s) {
    if (s->out) __stx_freeFA2(s->out);
    if (s->out1) __stx_freeFA2(s->out1);
    if (s->type == StateValue) {
        _t_free(s->data.value.values);
    }
    if (s->type == StateSymbol) {
        _t_free(s->data.symbol.symbols);
    }
    free(s);
}

/**
 * free the memory allocated by an FSA
 */
void _stx_freeFA(SState *s) {
    __stx_freeFA(s,++free_id);
    __stx_freeFA2(s);
}

/**
 * check that a SEMTREX_SYMBOL_SET contains the given symbol
 * @param[in] s symbol
 * @param[in] t SEMTREX_SYMBOL_SET tree
 * @returns 1 or 0
 */
int __symbol_set_contains(T *s,T *t) {
    if (!t) return 0;
    int i,c = _t_children(s);
    Symbol sym = _t_symbol(t);
    for (i=1;i<=c;i++) {
        if (semeq(sym,*(Symbol *)_t_surface(_t_child(s,i)))) return 1;
    }
    return 0;
}

/**
 * check that a SEMTREX_SYMBOL_SET does not contain the given symbol
 * @param[in] s symbol
 * @param[in] t SEMTREX_SYMBOL_SET tree
 * @returns 1 or 0
 */
int __symbol_set_does_not_contain(T *s,T *t) {
    if (!t) return 0;
    int i,c = _t_children(s);
    Symbol sym = _t_symbol(t);
    for (i=1;i<=c;i++) {
        if (semeq(sym,*(Symbol *)_t_surface(_t_child(s,i)))) return 0;
    }
    return 1;
}

/* advance the cursor according to the instructions in the state*/
T * __transition(TransitionType transition,T *t) {
    int i;
    if (!t) debug(D_STX_MATCH,"transition: t is null\n");
    if (!t) return 0;
    if (transition == TransitionDown) {
        debug(D_STX_MATCH,"transition: down\n");
        t = _t_child(t,1);
    }
    else if (isTransitionPop(transition)) {
        debug(D_STX_MATCH,"transition: popping %d\n",transition);
        for(i=transition;i<0;i++) {
            t = _t_parent(t);
        }
        // popping always means also moving to next child after the pop
        t = _t_next_sibling(t);
    }
    else if (isTransitionNext(transition)) {
        debug(D_STX_MATCH,"transition: next\n");
        t = _t_next_sibling(t);
    }

    debug(D_STX_MATCH,"transition: result %s\n",!t ? "NULL":t2s(t));
    return t;
}

// helper to see if the surface of given tree nodes matche
/// @todo move this to tree.c
int _val_match(T *t,T *t1) {

    int i;
    char *p1,*p2;
    size_t l = _t_size(t1);
    debug(D_STX_MATCH,"comparing sizes %ld,%ld\n",l,_t_size(t));
    if (l != _t_size(t)) return 0;
    i = memcmp(_t_surface(t),_t_surface(t1),l);
    debug(D_STX_MATCH,"compare result: %d\n",i);
    return i==0;
}

// convert cpointer SEMTREX_MATCH_CURSOR elements to MATCHED_PATH and SIBLING COUNT elements
void __fix(T *source_t,T *r) {
    T *m1,*m2;

    // get the start and end cursors
    T *start_c = *(T **)_t_surface(m1 = _t_child(r,2));
    T *end_c = *(T **)_t_surface(m2 = _t_child(r,3));

    // morph the start cursor in the match path
    int *p = _t_get_path(start_c);
    __t_morph(m1,SEMTREX_MATCH_PATH,p,sizeof(int)*(_t_path_depth(p)+1),1);

    // now figure out how many children were matched
    int d = _t_path_depth(p);
    int i;

    d--;
    if (d < 0) { i = 1;}
    else if (!end_c) {
        T *parent = _t_parent(start_c);
        if (!parent) i = 1;
        else {
            int pc = _t_children(parent);
            i = pc - p[d] + 1;
        }
    }
    else {
        int* p_end;
        p_end = _t_get_path(end_c);
        if (_t_path_depth(p_end) < d) {
            raise_error("whoa!  Mismatched path depths!");
        }
        if (debugging(D_STX_MATCH)) {
            char buf[255];
            _t_sprint_path(p,buf);
            debug(D_STX_MATCH,"start path:%s\n",buf);
            _t_sprint_path(p_end,buf);
            debug(D_STX_MATCH,"  end path:%s\n",buf);
        }
        i = p_end[d]- p[d];
        free(p_end);
    }
    free(p);
    __t_morph(m2,SEMTREX_MATCH_SIBLINGS_COUNT,&i,sizeof(int),0);
    int c = _t_children(r);
    for(i=4;i<=c;i++) {
        end_c = _t_child(r,i);
        __fix(source_t,end_c);
    }
}

#define MAX_BRANCH_DEPTH 5000

// structure to hold backtracking data for match algorithm
typedef struct BranchPoint {
    T *walk;
    SState *s;
    TransitionType transition;
    T *cursor;
    T *cursor_prev;
    T *match;
    int *r_path;
} BranchPoint;

char * __stx_dump_state(SState *s,char *buf);
char G_stx_debug_buf[1000];
#define _PUSH_BRANCH(state,t,c,cp,w) {                                    \
        G_stx_debug_buf[0]=0;debug(D_STX_MATCH,"pushing split branch for backtracking to state %s\n    with cursor:%s and cur_prev:%s\n",__stx_dump_state(state,G_stx_debug_buf),c?t2s(c):"NULL",cp?t2s(cp):"NULL"); \
    if((depth+1)>=MAX_BRANCH_DEPTH) {raise_error("MAX branch depth exceeded");} \
    stack[depth].s = state;                                             \
    stack[depth].transition = t;                                        \
    stack[depth].cursor = c;                                            \
    stack[depth].cursor_prev = cp;                                      \
    stack[depth].walk = w;                                              \
    if (rP) {                                                           \
        if (*rP) {                                                      \
            stack[depth].match = _t_clone(*rP);                         \
            stack[depth].r_path = _t_get_path(r);                       \
        }                                                               \
        else stack[depth].match = 0;                                    \
    }                                                                   \
    depth++;                                                            \
}

#define PUSH_BRANCH(state,t,c,cp) _PUSH_BRANCH(state,t,c,cp,0)
#define PUSH_WALK_POINT(state,t,c,cp) _PUSH_BRANCH(state,t,c,cp,c)

#define FAIL {s=0;break;}
#define TRANSITION(x) if (!t) {FAIL;}; if (!x) {FAIL;}; t = __transition(s->transition,t); s = s->out;

/**
 * build an FSA from semtrex tree and walk it using a recursive backtracing algorithm to match the tree in t.
 *
 * @param[in] semtrex tree to use for matching a tree
 * @param[in] source_t tree to match against
 * @param[inout] rP match results tree being built.  (nil if no results needed)
 * @returns 1 or 0 if matched or not
 */
int __t_match(T *semtrex,T *source_t,T **rP) {
    int states;
    char buf[5000];
    BranchPoint stack[MAX_BRANCH_DEPTH];

    int depth = 0;
    T *t = source_t,*prev_t;
    int matched;
    T *r = 0,*x;
    if (rP) *rP = 0;

    SgroupOpen *o;

    SState *fa = _stx_makeFA(semtrex,&states);
    SState *s = fa;

    while (s && s != &matchstate) {
        debug(D_STX_MATCH,"IN:%s\n",G_s_str[s->type]);
        if (s->type == StateGroupOpen) {
            o = &s->data.groupo;
            debug(D_STX_MATCH,"   for %s\n",_sem_get_name(G_sem,o->symbol));
        }
        if (s->type == StateGroupClose) {
            // get the match structure from the GroupOpen state pointed to by this state
            o = &s->data.groupc.openP->data.groupo;
            debug(D_STX_MATCH,"   for %s\n",_sem_get_name(G_sem,o->symbol));
        }
        if (debugging(D_STX_MATCH)) {G_cursor=t;G_cur_stx_state=s;debug(D_STX_MATCH,"  FSA:%s\n",_stx_dump(fa,G_stx_dump_buf));debug(D_STX_MATCH,"  tree:%s\n",!t ? "NULL" : _t2s(G_sem,_t_root(t)));}
        if (rP && *rP) {debug(D_STX_MATCH,"MATCH:\n%s\n",__t2s(G_sem,*rP,INDENT));}


        switch(s->type) {
        case StateValue:
            if (!t) {FAIL;}
            else {
                T *v = s->data.value.values;

                if (!t) FAIL;
                int count = _t_children(v);
                int i;
                debug(D_STX_MATCH,"  seeking:%s%s\n",s->data.value.flags & LITERAL_NOT ? " ~":"",__t_dump(G_sem,v,0,buf));
                Symbol ts = _t_symbol(t);
                if (s->data.value.flags & LITERAL_NOT) {
                    if (s->data.value.flags & LITERAL_SET) {
                        // all in the set must not match
                        matched = 1;
                        for(i=1;i<=count && matched;i++) {
                            x = _t_child(v,i);
                            matched = !(semeq(ts,_t_symbol(x)) && _val_match(t,x));
                        }
                    }
                    else {
                        matched = !(semeq(ts,_t_symbol(v)) && _val_match(t,v));
                    }
                }
                else {
                    if (s->data.value.flags & LITERAL_SET) {
                        // at least one in the set much match
                        matched = 0;
                        for(i=1;i<=count && !matched; i++) {
                            x = _t_child(v,i);
                            matched = semeq(ts,_t_symbol(x)) && _val_match(t,x);
                        }
                    }
                    else {
                        matched = semeq(ts,_t_symbol(v)) && _val_match(t,v);
                    }
                }

                if (!matched) FAIL;
            }
            t = __transition(s->transition,t);
            s = s->out;
            break;
        case StateSymbol:
            if (s->data.symbol.flags & LITERAL_SET) {
                TRANSITION((s->data.symbol.flags & LITERAL_NOT) ?
                           __symbol_set_does_not_contain(s->data.symbol.symbols,t) :
                           __symbol_set_contains(s->data.symbol.symbols,t));
            }
            else {
                if (!t) FAIL;
                int matched = semeq(_t_symbol(t),*(Symbol *)_t_surface(s->data.symbol.symbols));
                TRANSITION(s->data.symbol.flags & LITERAL_NOT ? !matched : matched);
            }
            break;
        case StateAny:
            TRANSITION(1);
            break;
        case StateSplit:
            PUSH_BRANCH(s->out1,s->transition1,t,prev_t);
            s = s->out;
            break;
        case StateWalk:
            s = s->out;
            PUSH_WALK_POINT(s,s->transition,t,prev_t);
            break;
        case StateGroupOpen:
            o = &s->data.groupo;
            if (!rP) {
                // if we aren't collecting up match results simply follow groups through
                s = s->out;
            }
            else {
                if (!t) FAIL;

                r = _t_newi(r,SEMTREX_MATCH,o->uid);
                if (!*rP) *rP = r; // save the root match
                T *x = _t_news(r,SEMTREX_MATCH_SYMBOL,o->symbol);
                // save the current cursor as a c cpointer.  This will get converted to
                // an actual MATCH_PATH later in __fix if it turns out that this particular
                // part of the tree actually does match.
                _t_new(r,SEMTREX_MATCH_CURSOR,&t,sizeof(t));
                s = s->out;
            }
            break;
        case StateGroupClose:
            if (rP) {

                int pt[2] = {3,TREE_PATH_TERMINATOR};
//                if (prev_t != t) raise_error("BOING");
                T *x = _t_new(0,SEMTREX_MATCH_CURSOR,&t,sizeof(t));
                _t_insert_at(r, pt, x);

                T *pp = _t_parent(r);
                if (pp) r = pp;
            }
            s = s->out;
            break;
        case StateDescend:
            t = _t_child(t,1);
            s = s->out;
            break;
        case StateMatch:
            break;
        }
        prev_t = t;
        // if we just had a fail see if there is some backtracking we can do
        if (!s && depth) {
            --depth;
            debug(D_STX_MATCH,"Fail & backtracking possible\n");
            if (rP) {
                if (*rP) _t_free(*rP);
                if ((*rP = stack[depth].match)) {
                    r = _t_get(*rP,stack[depth].r_path);
                    free(stack[depth].r_path);
                }
                else r = 0;
            }

            // pop back to the state in the FSA where we failed
            s = stack[depth].s;
            // reset the saved cursor
            t = stack[depth].cursor;
            prev_t = stack[depth].cursor_prev;
            debug(D_STX_MATCH,"     popping to--%s\n", t ? t2s(t) : "NULL");
            debug(D_STX_MATCH,"     prev_t:%s\n",prev_t ? t2s(prev_t) : "NULL");
            debug(D_STX_MATCH,"     running tranistion:%d\n",stack[depth].transition);
            // and run the transition that we saved for moving to that state that
            // normally would have been run in the TRANSITION macro
            t = __transition(stack[depth].transition,t);

            T *walk = stack[depth].walk;
            if(walk) {
                t = _t_child(walk,1);
                if (!t) {
                    t = _t_next_sibling(walk);
                    if (!t) {
                        T *p = walk;
                        T *root = stack[depth].cursor;
                        while(1) {
                            p = _t_parent(p);
                            if (!p || p == root) {t = 0;break;}
                            if ((t = _t_next_sibling(p))) break;
                        }
                    }
                }
                if (t) {stack[depth++].walk = t;}
                else s = 0;
            }
        }
    }
    if (rP) {
        if (s) {
            debug(D_STX_MATCH,"FIXING RESULTS:\n%s\n",__t2s(G_sem,*rP,INDENT));
            // convert the cursor pointers to matched paths/sibling counts
            __fix(source_t,*rP);
        }
        else if(*rP) {
            _t_free(*rP);
        }
    }
    // clean up any remaining stack frames
    while (depth--) {
        if (rP) {
            if ((r = stack[depth].match)) {
                _t_free(r);
                free(stack[depth].r_path);
            }
        }
    }
    _stx_freeFA(fa);
    if (s == &matchstate) {
        debug(D_STX_MATCH,"Matched!\n");
        return true;
    }
    return false;
}

/**
 * Match a tree against a semtrex and get back match results
 *
 * @param[in] semtrex the semtrex pattern tree
 * @param[in] t the tree to match against the pattern
 * @param[inout] rP a pointer to a T to be filled with a match results tree
 * @returns 1 or 0 if matched or not
 */
int _t_matchr(T *semtrex,T *t,T **rP) {
    return __t_match(semtrex,t,rP);
}

/**
 * Match a tree against a semtrex
 *
 * @param[in] semtrex the semtrex pattern tree
 * @param[in] t the tree to match against the pattern
 * @returns 1 or 0 if matched or not
 */
int _t_match(T *semtrex,T *t) {
    return __t_match(semtrex,t,NULL);
}

T *_stx_get_matched_node(Symbol s,T *match_results,T *match_tree,int *sibs) {
    T *m = _t_get_match(match_results,s);
    if (!m) {
        raise_error("expected to have match!");
    }
    int *path = (int *)_t_surface(_t_child(m,SemtrexMatchPathIdx));
    if (sibs)
        *sibs = *(int*)_t_surface(_t_child(m,SemtrexMatchSibsIdx));
    T *x = _t_get(match_tree,path);

    if (!x) {
        raise_error("expecting to get a value from match!!");
    }
    return x;
}

void _stx_replace(T *semtrex,T *t,T *replace){
    T *r;
    Symbol sym = _t_symbol(replace);
    while(_t_matchr(semtrex,t,&r)) {
        int sibs;
        T *x = _stx_get_matched_node(sym,r,t,&sibs);
        if (sibs > 1) raise_error("not implemented for sibs > 1");
        _t_replace_node(x,_t_clone(replace));
        _t_free(r);
    }
}

/**
 * extract the portion of a semtrex match results that corresponds with a given group symbol
 *
 * @param[in] result match results from the _t_matchr call
 * @param[in] group the uid from the semtrex group that you want the result for
 * @returns T of the match or NULL if no such match found
 */
T *_t_get_match(T *match,Symbol group)
{
    if (!match) return 0;
    T *s = _t_child(match,1);
    if (semeq(*(Symbol *)_t_surface(s),group)) {
        return match;
    };
    int i = 4,c = _t_children(match);
    for(i=4;i<=c;i++) {
        s =_t_child(match,i);
        s = _t_get_match(s,group);
        if (s) return s;
    }
    return 0;
}

/**
 * create a new tree based on the matched elements from a semtrex match
 *
 * @param[in] sem semantic context
 * @param[in] match a match results from a call to _t_matchr
 * @param[in] group the symbol of the group in the match to embody
 * @param[in] t the matching tree
 *
 */
T *_t_embody_from_match(SemTable *sem,T *match,Symbol group,T *t) {
    return __t_embody_from_match(sem,_t_get_match(match,group),t);
}

T *__t_embody_from_match(SemTable *sem,T *match,T *t) {
    Symbol s = *(Symbol *)_t_surface(_t_child(match,1));
    if (semeq(s,NULL_SYMBOL)) return 0;
    T *e;
    int i,j = _t_children(match);
    if (j > 3) {
        e = _t_new_root(s);
        for(i=4;i<=j;i++) {
            T *c = _t_child(match,i);
            if (c) {
                T *r = __t_embody_from_match(sem,c,t);
                if (r) _t_add(e,r);
            }
        }
    }
    else {
        int *p;
        int children = *(int *)_t_surface(_t_child(match,3));
        Structure st = _sem_get_symbol_structure(sem,s);
        T *x;
        switch(st.id) {
        case CSTRING_ID:
            return asciiT_tos(t,match,0,s);
        case INTEGER_ID:
            return asciiT_toi(t,match,0, s);
        case INTEGER64_ID:
            return asciiT_tol(t,match,0, s);
        case FLOAT_ID:
            return asciiT_tof(t,match,0,s);
        case CHAR_ID:
            return asciiT_toc(t,match,0,s);
        default:
            p = (int *)_t_surface(_t_child(match,2));
            x = _t_get(t,p);
            e = _t_clone(x);
        }
    }
    return e;
}

// semtrex dumping code
char * __dump_semtrex(SemTable *sem,T *s,char *buf);

void __stxd_multi(SemTable *sem,char *x,T *s,char *buf) {
    char b[4000];
    T *sub = _t_child(s,1);
    Symbol ss = _t_symbol(sub);
    int has_child = (semeq(ss,SEMTREX_SYMBOL_LITERAL_NOT) || semeq(ss,SEMTREX_SYMBOL_LITERAL)) ? 2 : 1;
    sprintf(buf,(_t_children(s)>has_child || _t_symbol(sub).id==SEMTREX_SEQUENCE_ID) ? "(%s)%s" : "%s%s",__dump_semtrex(sem,sub,b),x);
}
void __stxd_descend(SemTable *sem,T *s,char *v,char *buf,int skip) {
    if((_t_children(s)-skip)>0) {
        char b[4000];
        T *sub = _t_child(s,1+skip);
        Symbol ss = _t_symbol(sub);
        int has_child = (semeq(ss,SEMTREX_SYMBOL_LITERAL_NOT) || semeq(ss,SEMTREX_SYMBOL_LITERAL)) ? 2 : 1;
        sprintf(buf,_t_children(sub)>has_child?"%s/(%s)":"%s/%s",v,__dump_semtrex(sem,sub,b));
    }
    else sprintf(buf,"%s",v);
}

char * __dump_semtrex(SemTable *sem,T *s,char *buf) {
    Symbol sym = _t_symbol(s);
    char b[5000];
    char b1[5000];
    char *sn,*bx;
    T *t,*v,*v1;
    int i,c,count;
    SemanticID sid;
    switch(sym.id) {
    case SEMTREX_VALUE_LITERAL_ID:
    case SEMTREX_VALUE_LITERAL_NOT_ID:
        v = _t_child(s,1); //get the value or set
        if (semeq(_t_symbol(v),SEMTREX_VALUE_SET)) {
            count = _t_children(v);
            v1 = _t_child(v,1);
            if (!v1) {raise_error("no values in set!");}
        }
        else {
            count = 1;
            v1 = v;
            v = s;
        }
        sid = _t_symbol(v1);  // if set assume values are all the same type
        sn = _sem_get_name(sem,sid);
        if (*sn=='<')
            sprintf(b,"%d.%d.%d",sid.context,sid.semtype,sid.id);
        else
            sprintf(b,"%s",sn);
        Structure st = _sem_get_symbol_structure(sem,sid);
        if (sym.id == SEMTREX_VALUE_LITERAL_NOT_ID) {
            sprintf(b+strlen(b),"!");
        }
        sprintf(b+strlen(b),"=");
        if (count > 1)
            sprintf(b+strlen(b),"{");
        for(i=1;i<=count;i++) {
            T *x = _t_child(v,i);
            if (semeq(st,CSTRING))
                sprintf(b+strlen(b),"\"%s\"",(char *)(_t_surface(x)));
            else if (semeq(st,CHAR))
                sprintf(b+strlen(b),"'%c'",*(char *)(_t_surface(x)));
            else if (semeq(st,INTEGER))
                sprintf(b+strlen(b),"%d",*(int *)(_t_surface(x)));
            else if (semeq(st,FLOAT))
                sprintf(b+strlen(b),"%f",*(float *)(_t_surface(x)));
            else sprintf(b+strlen(b),"???x");
            if (i < count)
                sprintf(b+strlen(b),",");
        }
        if (count > 1)
            sprintf(b+strlen(b),"}");
        sprintf(buf,"%s",b);
        break;
    case SEMTREX_SYMBOL_LITERAL_NOT_ID:
    case SEMTREX_SYMBOL_LITERAL_ID:

        if (semeq(sym, SEMTREX_SYMBOL_LITERAL_NOT)) {
            b[0] = '!';
            b[1] = 0;
        }
        else b[0] = 0;

        v = _t_child(s,1); //get the symbol value or set

        if (semeq(_t_symbol(v),SEMTREX_SYMBOL_SET)) {
            count = _t_children(v);
            v1 = _t_child(v,1);
            if (!v1) {raise_error("no symbols in set!");}
        }
        else {
            count = 1;
            v1 = v;
            v = s;
        }

        if (count > 1) {
            sprintf(b+strlen(b),"{");
        }
        for(i=1;i<=count;i++) {
            sid = *(Symbol *)_t_surface(v1);
            sn = _sem_get_name(sem,sid);
            // ignore "<unknown symbol"
            if (*sn=='<')
                sprintf(b+strlen(b),"%d.%d.%d",sid.context,sid.semtype,sid.id);
            else
                sprintf(b+strlen(b),"%s",sn);
            v1 = _t_next_sibling(v1);
            if (i!=count)
                sprintf(b+strlen(b),",");
        }
        if (count > 1)
            sprintf(b+strlen(b),"}");

        __stxd_descend(sem,s,b,buf,1);
        break;
    case SEMTREX_SYMBOL_ANY_ID:
        sprintf(b,".");
        __stxd_descend(sem,s,b,buf,0);
        break;
    case SEMTREX_SEQUENCE_ID:
        sn = buf;
        DO_KIDS(s,
            sprintf(sn,i<_c ? "%s,":"%s",__dump_semtrex(sem,_t_child(s,i),b));
            sn += strlen(sn);
                );
        break;
    case SEMTREX_OR_ID:
        t = _t_child(s,1);
        sn = __dump_semtrex(sem,t,b);
        Symbol ss = _t_symbol(t);
        int has_child = (semeq(ss,SEMTREX_SYMBOL_LITERAL_NOT) || semeq(ss,SEMTREX_SYMBOL_LITERAL)) ? 2 : 1;
        sprintf(buf,(_t_children(t) > has_child) ? "(%s)|":"%s|",sn);
        t = _t_child(s,2);
        sn = __dump_semtrex(sem,t,b);
        ss = _t_symbol(t);
        has_child = (semeq(ss,SEMTREX_SYMBOL_LITERAL_NOT) || semeq(ss,SEMTREX_SYMBOL_LITERAL)) ? 2 : 1;
        sprintf(buf+strlen(buf),(_t_children(t) > has_child) ? "(%s)":"%s",sn);
        break;
    case SEMTREX_NOT_ID:
        t = _t_child(s,1);
        sn = __dump_semtrex(sem,t,b);
        sprintf(buf,"~%s",sn);
        break;
    case SEMTREX_ZERO_OR_MORE_ID:
        __stxd_multi(sem,"*",s,buf);
        break;
    case SEMTREX_ONE_OR_MORE_ID:
        __stxd_multi(sem,"+",s,buf);
        break;
    case SEMTREX_ZERO_OR_ONE_ID:
        __stxd_multi(sem,"?",s,buf);
        break;
    case SEMTREX_GROUP_ID:
        sn = _sem_get_name(sem,*(Symbol *)_t_surface(s));
        // ignore "<unknown symbol"
        if  (*sn=='<')
            sprintf(buf, "<%s>",__dump_semtrex(sem,_t_child(s,1),b));
        else
            sprintf(buf, "<%s:%s>",sn,__dump_semtrex(sem,_t_child(s,1),b));
        break;
    case SEMTREX_DESCEND_ID:
        sprintf(buf, "/%s",__dump_semtrex(sem,_t_child(s,1),b));
        break;
    case SEMTREX_WALK_ID:
        sprintf(buf, "(%%%s)",__dump_semtrex(sem,_t_child(s,1),b));
        break;
    }
    return buf;
}

/**
 * convert a semtrex tree into linear text format
 *
 * @param[in] sem the semantic contexts
 * @param[in] s the semtrex tree
 * @param[in] buf the string buffer to fill
 * @returns the buffer
 */
char * _dump_semtrex(SemTable *sem,T *s,char *buf) {
    buf[0] = '/';
    __dump_semtrex(sem,s,buf+1);
    return buf;
}

// helper to add a stx_char value literal to a semtrex
T *__stxcv(T *p,char c) {
    T *t =  _t_newr(p,SEMTREX_VALUE_LITERAL);
    _t_newc(t,ASCII_CHAR,c);
    return t;
}

// helper to to add a semtrex literal value set of ascii chars to a semtrex
T *__stxcvm(T *p,int not,int count,...) {
    va_list chars;
    T *t =  _t_newr(p,not?SEMTREX_VALUE_LITERAL_NOT:SEMTREX_VALUE_LITERAL);
    T *v = _t_newr(t,SEMTREX_VALUE_SET);

    va_start(chars,count);
    int i;
    for(i=0;i<count;i++) {
        _t_newc(v,ASCII_CHAR,va_arg(chars,int));
    }
    va_end(chars);

    return t;
}

// helper to add a bunch of semtrex ors that match a character set
void _stxcs(T *stxx,char *an) {
    T *label = _t_newr(stxx,SEMTREX_ONE_OR_MORE);
    label = _t_newr(label,SEMTREX_OR);
    while(*an) {
        __stxcv(label,*an);
        an++;
        if (*an) label = _t_newr(label,SEMTREX_OR);
    }
    __stxcv(label,'_');
}

void _stxl(T *stxx) {
    _stxcs(stxx,"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789._");
}

// temporary function until we get system label table operational
Symbol get_symbol(char *symbol_name,SemTable *sem) {
    int ctx;
    for (ctx=0;ctx<sem->contexts;ctx++) {
        ContextStore *cs = __sem_context(sem,ctx);
        if (!cs->definitions) continue;
        T *symbols = __sem_get_defs(sem,SEM_TYPE_SYMBOL,ctx);
        int i,c = _t_children(symbols);
        for(i=1;i<=c;i++) {
            T *t = _t_child(symbols,i);
            T *c = _t_child(_t_child(t,DefLabelIdx),1);
            if (!strcmp(symbol_name,(char *)_t_surface(c))) {
                Symbol r = {ctx,SEM_TYPE_SYMBOL,i};
                return r;
            }
        }
    }
    return NULL_SYMBOL;
}

//#define DUMP_TOKENS
#ifdef DUMP_TOKENS
#define dump_tokens(str) puts(str);puts(_t2s(G_sem,tokens));
#else
#define dump_tokens(str)
#endif

/*
   a utility function to move the contents of paren/group tokens as children of the
   open token.
   Assumes the semtrex results was from a semtrex of the form:
     ...  <STX_OP:STX_OP,<STX_SIBS:!STX_CP+>,STX_CP>
     where the contents is marked by one group (in the case above STX_SIBS) and the
     the whole thing is marked by an "open" group

 */
T *wrap(T *tokens,T *results, Symbol contents_s, Symbol open_s) {
    T *m = _t_get_match(results,contents_s);
    T *om = _t_get_match(results,open_s);

    // transfer the contents nodes to the open node
    int count = *(int *)_t_surface(_t_child(m,3));
    int *cpath = (int *)_t_surface(_t_child(m,2));
    int *opath = (int *)_t_surface(_t_child(om,2));
    T *o = _t_get(tokens,opath);
    T *parent = _t_parent(o);
    int x = cpath[_t_path_depth(cpath)-1];
    T *t;
    while(count--) {
        t = _t_child(parent,x);
        _t_detach_by_ptr(parent,t);
        _t_add(o,t);
    }
    // free the close token
    t = _t_child(parent,x);
    _t_detach_by_ptr(parent,t);
    _t_free(t);
    return o;
}

/**
 * convert a cstring to an ASCII_CHARS tree
 * @param[in] c string
 * @returns T ASCII_CHARS tree
 */
T *makeASCIITree(char *c) {
    T *s = _t_new_root(ASCII_CHARS);
    while(*c) {
        _t_newc(s,ASCII_CHAR,*c);
        c++;
    }
    return s;
}

char *_asciiT2str(T* asciiT,T* match,T *t,char *buf) {
    int path[100];
    int sibs = *(int *)_t_surface(_t_child(match,SemtrexMatchSibsIdx));
    int *p = (int *)_t_surface(_t_child(match,SemtrexMatchPathIdx));
    int j,d = _t_path_depth(p);
    if (d>=100) {raise_error("path too deep!");}
    memcpy(path,p,sizeof(int)*(d+1));
    for(j=0;j<sibs;j++) {
        buf[j] = *(char *)_t_surface(_t_get(asciiT,path));
        path[d-1]++;
    }
    buf[j]=0;
    return buf;
}

/**
 * convert ascii tokens from a match to an integer and add them to the given tree
 */
T *asciiT_toi(T* asciiT,T* match,T *t,Symbol s) {
    char buf[10];
    _asciiT2str(asciiT,match,t,buf);
    return _t_newi(t,s,atoi(buf));
}

/**
 * convert ascii tokens from a match to a 64 bit integer and add them to the given tree
 */
T *asciiT_tol(T* asciiT,T* match,T *t,Symbol s) {
    char buf[12];
    _asciiT2str(asciiT,match,t,buf);
    return _t_newi64(t,s,atol(buf));
}

/**
 * convert ascii tokens from a match to an float and add them to the given tree
 */
T *asciiT_tof(T* asciiT,T* match,T *t,Symbol s) {
    char buf[10];
    _asciiT2str(asciiT,match,t,buf);
    float f = atof(buf);
    return _t_new(t,s,&f,sizeof(float));
}

/**
 * convert ascii tokens from a match to a string and add them to the given tree
 */
T *asciiT_tos(T* asciiT,T* match,T *t,Symbol s) {
    char buf[255];
    _asciiT2str(asciiT,match,t,buf);
    return _t_new_str(t,s,buf);
}

/**
 * convert ascii tokens from a match to a char and add them to the given tree
 */
T *asciiT_toc(T* asciiT,T* match,T *t,Symbol s) {
    int *path = (int *)_t_surface(_t_child(match,2));
    int c = *(int *)_t_surface(_t_get(asciiT,path));
    return _t_newc(t,s,c);
}

/**
 * utility function to create a semtrex litteral symbol set
 */
T *__sl(T *p, bool not,int count, ...) {
    va_list symbols;
    T *t = _t_newr(p,not ? SEMTREX_SYMBOL_LITERAL_NOT : SEMTREX_SYMBOL_LITERAL);
    T *ss = count > 1 ?  _t_newr(t,SEMTREX_SYMBOL_SET) : t;
    va_start(symbols,count);
    int i;
    for(i=0;i<count;i++) {
        _t_news(ss,SEMTREX_SYMBOL,va_arg(symbols,Symbol));
    }
    va_end(symbols);
    return t;
}

/**
 * convert a cstring to semtrex tree
 * @param[in] r Receptor context for searching for symbols
 * @param[in] stx the cstring representation of a semtrex tree
 * @returns T semtrex tree
 */
T *parseSemtrex(SemTable *sem,char *stx) {
    // convert the string into a tree
    #ifdef DUMP_TOKENS
    printf("\nPARSING:%s\n",stx);
    #endif
    T *t,*s = makeASCIITree(stx);

    /////////////////////////////////////////////////////
    // build the token stream out of an ascii stream
    // PATTERN
    // "/{STX_TOKENS:(ASCII_CHARS/({STX_SL:ASCII_CHAR='/'})|(({STX_OP:ASCII_CHAR='('})|(({STX_CP:ASCII_CHAR=')'})|(({STX_PLUS:ASCII_CHAR='+'})|(({STX_COMMA:ASCII_CHAR=','})|((ASCII_CHAR='!',{STX_EXCEPT:[a-zA-Z0-9_]+})|(({STX_CG:ASCII_CHAR='}'})|(({STX_STAR:ASCII_CHAR='*'})|(({STX_LABEL:[a-zA-Z0-9_]+})|(ASCII_CHAR='{',{STX_OG:[a-zA-Z0-9_]+},ASCII_CHAR=':')))))))))+)}
    T *ts = _t_news(0,SEMTREX_GROUP,STX_TOKENS);
    T *g = _sl(ts,ASCII_CHARS);
    T *sq = _t_newr(g,SEMTREX_SEQUENCE);
    T *p = _t_newr(sq,SEMTREX_ONE_OR_MORE);
    T *o = _t_newr(p,SEMTREX_OR);
    t = _t_news(o,SEMTREX_GROUP,STX_WALK);
    __stxcv(t,'%');
    o = _t_newr(o,SEMTREX_OR);
    t = _t_news(o,SEMTREX_GROUP,STX_SL);
    __stxcv(t,'/');
    o = _t_newr(o,SEMTREX_OR);
    t = _t_news(o,SEMTREX_GROUP,STX_OP);
    __stxcv(t,'(');
    o = _t_newr(o,SEMTREX_OR);
    t = _t_news(o,SEMTREX_GROUP,STX_CP);
    __stxcv(t,')');

    o = _t_newr(o,SEMTREX_OR);
    t = _t_news(o,SEMTREX_GROUP,STX_OR);
    __stxcv(t,'|');
    o = _t_newr(o,SEMTREX_OR);
    t = _t_news(o,SEMTREX_GROUP,STX_COMMA);
    __stxcv(t,',');

    o = _t_newr(o,SEMTREX_OR);
    t = _t_news(o,SEMTREX_GROUP,STX_CG);
    __stxcv(t,'>');
    o = _t_newr(o,SEMTREX_OR);
    t = _t_news(o,SEMTREX_GROUP,SEMTREX_SYMBOL_ANY);
    __stxcv(t,'.');
    o = _t_newr(o,SEMTREX_OR);
    t = _t_news(o,SEMTREX_GROUP,STX_STAR);
    __stxcv(t,'*');
    o = _t_newr(o,SEMTREX_OR);
    t = _t_news(o,SEMTREX_GROUP,STX_PLUS);
    __stxcv(t,'+');
    o = _t_newr(o,SEMTREX_OR);
    t = _t_news(o,SEMTREX_GROUP,STX_Q);
    __stxcv(t,'?');
    o = _t_newr(o,SEMTREX_OR);
    t = _t_news(o,SEMTREX_GROUP,STX_NOT);
    __stxcv(t,'~');

    o = _t_newr(o,SEMTREX_OR);
    sq = _t_newr(o,SEMTREX_SEQUENCE);
    t = _t_news(sq,SEMTREX_GROUP,STX_EQ);
    _stxl(t);
    __stxcv(sq,'=');

    o = _t_newr(o,SEMTREX_OR);
    sq = _t_newr(o,SEMTREX_SEQUENCE);
    t = _t_news(sq,SEMTREX_GROUP,STX_NEQ);
    _stxl(t);
    __stxcv(sq,'!');
    __stxcv(sq,'=');

    o = _t_newr(o,SEMTREX_OR);
    sq = _t_newr(o,SEMTREX_SEQUENCE);
    __stxcv(sq,'\'');
    t = _t_news(sq,SEMTREX_GROUP,STX_VAL_C);
    _sl(t,ASCII_CHAR);
    __stxcv(sq,'\'');

    o = _t_newr(o,SEMTREX_OR);
    sq = _t_newr(o,SEMTREX_SEQUENCE);
    __stxcv(sq,'"');
    t = _t_news(sq,SEMTREX_GROUP,STX_VAL_S);
    _stxl(t);
    __stxcv(sq,'"');

    o = _t_newr(o,SEMTREX_OR);
    t = _t_news(o,SEMTREX_GROUP,STX_OS);
    __stxcv(t,'{');
    o = _t_newr(o,SEMTREX_OR);
    t = _t_news(o,SEMTREX_GROUP,STX_CS);
    __stxcv(t,'}');

    o = _t_newr(o,SEMTREX_OR);
    sq = _t_newr(o,SEMTREX_SEQUENCE);
    __stxcv(sq,'!');
    t = _t_news(sq,SEMTREX_GROUP,STX_EXCEPT);
    _stxl(t);

    o = _t_newr(o,SEMTREX_OR);
    sq = _t_newr(o,SEMTREX_SEQUENCE);
    t = _t_news(sq,SEMTREX_GROUP,STX_EXCEPT);
    __stxcv(t,'!');

    o = _t_newr(o,SEMTREX_OR);
    sq = _t_newr(o,SEMTREX_SEQUENCE);
    t = _t_news(sq,SEMTREX_GROUP,STX_VAL_F);
    T *sq2 = _t_newr(t,SEMTREX_SEQUENCE);
    t = _t_newr(sq2,SEMTREX_ZERO_OR_MORE);
    _stxcs(t,"0123456789");
    __stxcv(sq2,'.');
    t = _t_newr(sq2,SEMTREX_ONE_OR_MORE);
    _stxcs(t,"0123456789");

    o = _t_newr(o,SEMTREX_OR);
    sq = _t_newr(o,SEMTREX_SEQUENCE);
    t = _t_news(sq,SEMTREX_GROUP,STX_VAL_I);
    t = _t_newr(t,SEMTREX_ONE_OR_MORE);
    _stxcs(t,"0123456789");

    o = _t_newr(o,SEMTREX_OR);
    t = _t_news(o,SEMTREX_GROUP,STX_LABEL);
    _stxl(t);

    //  o = _t_newr(o,SEMTREX_OR);
    sq = _t_newr(o,SEMTREX_SEQUENCE);
    __stxcv(sq,'<');
    t = _t_news(sq,SEMTREX_GROUP,STX_OG);
    _stxl(t);
    __stxcv(sq,':');


    T *results,*tokens;
    if (_t_matchr(ts,s,&results)) {
        char buf[10000];

        //----------------
        // ACTION
        /// @todo, this should be implemented using template filling?
        tokens = _t_new_root(STX_TOKENS);
        int i,m = _t_children(results);
        for(i=4;i<=m;i++) {
            T *c = _t_child(results,i);
            T *sn = _t_child(c,1);
            Symbol ts = *(Symbol *)_t_surface(sn);
            if (semeq(ts,STX_VAL_S) || semeq(ts,STX_LABEL) || semeq(ts,STX_OG) || semeq(ts,STX_EXCEPT) || semeq(ts,STX_EQ) || semeq(ts,STX_NEQ)){
                asciiT_tos(s,c,tokens,ts);
            }
            else if (semeq(ts,STX_VAL_C)) {
                asciiT_toc(s,c,tokens,ts);
            }
            else if (semeq(ts,STX_VAL_I)) {
                asciiT_toi(s,c,tokens,ts);
            }
            else if (semeq(ts,STX_VAL_F)) {
                asciiT_tof(s,c,tokens,ts);
            }
            else
                _t_newi(tokens,ts,0);
        }
        _t_free(results);

        dump_tokens("TOKENS:");
        T *sxx,*sq;

        /////////////////////////////////////////////////////
        // convert STX_OS/STX_CS to STX_SET groups
        // /%<STX_OS:STX_OS,<STX_SET:!{STX_OS,STX_CS}+>,STX_CS>

        sxx = _t_new_root(SEMTREX_WALK);
        g = _t_news(sxx,SEMTREX_GROUP,STX_OS);
        sq = _t_newr(g,SEMTREX_SEQUENCE);
        _sl(sq,STX_OS);
        t = _t_news(sq,SEMTREX_GROUP,STX_SET);
        t = _t_newr(t,SEMTREX_ONE_OR_MORE);

        __sl(t,1,2,STX_OS,STX_CS);
        _sl(sq,STX_CS);

        //----------------
        // ACTION
        while (_t_matchr(sxx,tokens,&results)) {
            g = wrap(tokens,results,STX_SET,STX_OS);
            // convert the STX_OS to STX_SET and free the STX_CS
            g->contents.symbol = STX_SET;
            // zap STX_COMMAs
            int cc = _t_children(g);
            T *v = _t_child(g,1);
            while(cc--) {
                T *x = _t_next_sibling(v);
                Symbol is = _t_symbol(v);
                if (semeq(is,STX_COMMA)) {
                    _t_detach_by_ptr(g,v);
                    _t_free(v);
                }
                v = x;
            }
            _t_free(results);
        }
        _t_free(sxx);

        dump_tokens("TOKENS_AFTER_SETS:");

        /////////////////////////////////////////////////////
        // convert STX_EQ/STX_NEQ to SEMTREX_VALUE_LITERALS
        // PATTERN
        // /%<SEMTREX_VALUE_LITERAL:STX_EQ|STX_NEQ,<SEMTREX_VALUE_SET:STX_VAL_I|STX_VAL_F|STX_VAL_S|STX_VAL_C|STX_SET)>>
        sxx = _t_new_root(SEMTREX_WALK);
        g = _t_news(sxx,SEMTREX_GROUP,SEMTREX_VALUE_LITERAL);
        sq = _t_newr(g,SEMTREX_SEQUENCE);
        o = _t_newr(sq,SEMTREX_OR);
        _sl(o,STX_EQ);
        _sl(o,STX_NEQ);

        g = _t_news(sq,SEMTREX_GROUP,SEMTREX_VALUE_SET);
        o = _t_newr(g,SEMTREX_OR);
        _sl(o,STX_VAL_I);
        o = _t_newr(o,SEMTREX_OR);
        _sl(o,STX_VAL_F);
        o = _t_newr(o,SEMTREX_OR);
        _sl(o,STX_VAL_S);
        o = _t_newr(o,SEMTREX_OR);
        _sl(o,STX_VAL_C);
        _sl(o,STX_SET);

        //----------------
        // ACTION
        while (_t_matchr(sxx,tokens,&results)) {

            T *m = _t_get_match(results,SEMTREX_VALUE_LITERAL);
            int *path = (int *)_t_surface(_t_child(m,2));
            t = _t_get(tokens,path);
            Symbol val_type = _t_symbol(t);
            t->contents.symbol = semeq(val_type,STX_EQ) ? SEMTREX_VALUE_LITERAL : SEMTREX_VALUE_LITERAL_NOT;

            T *p = _t_parent(t);
            T *v = _t_next_sibling(t);
            _t_detach_by_ptr(p,v);

            int set_count;
            T *set;

            if (semeq(_t_symbol(v),STX_SET)) {
                set = v;
                v->contents.symbol = SEMTREX_VALUE_SET;
                set_count = _t_children(v);
                v = _t_child(v,1);
                while(set_count--) {
                    char *symbol_name = (char *)_t_surface(t);
                    Symbol vs = get_symbol(symbol_name,sem);
                    // convert the STX_VAL structure token to the semantic type specified by the value literal
                    v->contents.symbol = vs;
                    v = _t_next_sibling(v);
                }
                _t_add(t,set);
            }
            else {
                //              set = _t_newr(t,SEMTREX_VALUE_SET);
                char *symbol_name = (char *)_t_surface(t);
                Symbol vs = get_symbol(symbol_name,sem);
                // convert the STX_VAL structure token to the semantic type specified by the value literal
                v->contents.symbol = vs;
                _t_add(t,v);
            }


            _t_free(results);
        }
        _t_free(sxx);

        dump_tokens("TOKENS_AFTER_VALUE_LITERAL:");

        /////////////////////////////////////////////////////
        // replace paren groups with STX_SIBS list
        // PATTERN
        // /STX_TOKENS/.*,<STX_OP:STX_OP,<STX_SIBS:!{STX_CP,STX_OP}+>,STX_CP>
        sxx = _sl(0,STX_TOKENS);
        sq = _t_newr(sxx,SEMTREX_SEQUENCE);
        T *st = _t_newr(sq,SEMTREX_ZERO_OR_MORE);
        _t_newr(st,SEMTREX_SYMBOL_ANY);
        T *gg = _t_news(sq,SEMTREX_GROUP,STX_OP);
        T *sq1 = _t_newr(gg,SEMTREX_SEQUENCE);
        _sl(sq1,STX_OP);
        T *g = _t_news(sq1,SEMTREX_GROUP,STX_SIBS);
        T *any = _t_newr(g,SEMTREX_ONE_OR_MORE);
        __sl(any,1,2,STX_OP,STX_CP);
        _sl(sq1,STX_CP);

        //----------------
        // ACTION
        while (_t_matchr(sxx,tokens,&results)) {
            g = wrap(tokens,results,STX_SIBS,STX_OP);
            // convert the STX_OP to STX_SIBS and free the STX_CP
            g->contents.symbol = STX_SIBS;
            _t_free(results);
        }
        _t_free(sxx);

        dump_tokens("TOKENS_AFTER_SIBS:");

        /////////////////////////////////////////////////////
        // find groups
        // PATTERN
        // /%,<STX_OG:STX_OG,<SEMTREX_GROUP:!{STX_CG,STX_OG}+>,STX_CG>
        sxx = _t_new_root(SEMTREX_WALK);
        g = _t_news(sxx,SEMTREX_GROUP,STX_OG);
        sq = _t_newr(g,SEMTREX_SEQUENCE);
        _sl(sq,STX_OG);
        gg = _t_news(sq,SEMTREX_GROUP,SEMTREX_GROUP);
        any = _t_newr(gg,SEMTREX_ONE_OR_MORE);

        __sl(any,1,2,STX_OG,STX_CG);
        _sl(sq,STX_CG);

        //----------------
        // ACTION
        while (_t_matchr(sxx,tokens,&results)) {
            g = wrap(tokens,results,SEMTREX_GROUP,STX_OG);

            // convert the STX_OG to SEMTREX_GROUP children and free the STX_CG
            char *symbol_name = (char *)_t_surface(g);
            Symbol sy = get_symbol(symbol_name,sem);
            __t_morph(g,SEMTREX_GROUP,&sy,sizeof(Symbol),1);

            _t_free(results);
        }
        _t_free(sxx);

        dump_tokens("TOKENS_AFTER_GROUPS:");

        /////////////////////////////////////////////////////
        // if there are any parens left we raise mismatch!
        // PATTERN
        // /(STX_TOKENS/.*,(STX_OP)|(STX_CP))
        sxx = _sl(0,STX_TOKENS);
        sq = _t_newr(sxx,SEMTREX_SEQUENCE);
        st = _t_newr(sq,SEMTREX_ZERO_OR_MORE);
        _t_newr(st,SEMTREX_SYMBOL_ANY);
        o = _t_newr(sq,SEMTREX_OR);
        _sl(o,STX_OP);
        _sl(o,STX_CP);

        //----------------
        // ACTION
        if (_t_match(sxx,tokens)) {
            raise_error("mismatched parens! [tokens:%s]",_t2s(G_sem,tokens));
        }
        _t_free(sxx);

        /////////////////////////////////////////////////////
        // convert postfix groups
        // PATTERN
        // /*<STX_POSTFIX:.,STX_PLUS|STX_STAR|STX_Q>
        sxx = _t_new_root(SEMTREX_WALK);
        g = _t_news(sxx,SEMTREX_GROUP,STX_POSTFIX);
        sq = _t_newr(g,SEMTREX_SEQUENCE);
        _t_newr(sq,SEMTREX_SYMBOL_ANY);
        o = _t_newr(sq,SEMTREX_OR);
        _sl(o,STX_PLUS);
        o = _t_newr(o,SEMTREX_OR);
        _sl(o,STX_STAR);
        _sl(o,STX_Q);

        //----------------
        // ACTION
        while (_t_matchr(sxx,tokens,&results)) {
            T *m = _t_get_match(results,STX_POSTFIX);
            int *path = (int *)_t_surface(_t_child(m,2));
            int x = path[_t_path_depth(path)-1];
            t = _t_get(tokens,path);
            T *parent = _t_parent(t);
            t = _t_child(parent,x);
            _t_detach_by_ptr(parent,t);
            T *c = _t_get(tokens,path);
            _t_add(c,t);
            if (semeq(_t_symbol(c),STX_PLUS ))
                c->contents.symbol = SEMTREX_ONE_OR_MORE;
            else if (semeq(_t_symbol(c),STX_STAR ))
                c->contents.symbol = SEMTREX_ZERO_OR_MORE;
            else if (semeq(_t_symbol(c),STX_Q ))
                c->contents.symbol = SEMTREX_ZERO_OR_ONE;

            _t_free(results);
        }
        _t_free(sxx);

        dump_tokens("TOKENS_AFTER_POSTFIX:");

        /////////////////////////////////////////////////////
        // convert not
        // PATTERN
        // /%<SEMTREX_NOT:STX_NOT,.>
        sxx = _t_new_root(SEMTREX_WALK);
        g = _t_news(sxx,SEMTREX_GROUP,SEMTREX_NOT);
        sq = _t_newr(g,SEMTREX_SEQUENCE);
        _sl(sq,STX_NOT);
        _t_newr(sq,SEMTREX_SYMBOL_ANY);

        //----------------
        // ACTION
        while (_t_matchr(sxx,tokens,&results)) {
            T *m = _t_get_match(results,SEMTREX_NOT);
            int *path = (int *)_t_surface(_t_child(m,2));
            int x = path[_t_path_depth(path)-1];
            t = _t_get(tokens,path);
            T *parent = _t_parent(t);
            // detach the node to be negated
            T *c = _t_child(parent,x+1);
            _t_detach_by_ptr(parent,c);
            // reatach it to to the morphed STX_NOT
            T *n = _t_child(parent,x);
            n->contents.symbol = SEMTREX_NOT;
            _t_add(n,c);
            _t_free(results);
        }
        _t_free(sxx);

        dump_tokens("TOKENS_AFTER_NOT:");

        /////////////////////////////////////////////////////
        // convert things following slashes to children of things preceeding slashes
        // PATTERN
        // /%.*,<STX_CHILD:STX_LABEL,STX_SL,!STX_SL>
        sxx = _t_new_root(SEMTREX_WALK);
        sq = _t_newr(sxx,SEMTREX_SEQUENCE);
        any = _t_newr(sq,SEMTREX_ZERO_OR_MORE);
        _t_newr(any,SEMTREX_SYMBOL_ANY);
        g = _t_news(sq,SEMTREX_GROUP,STX_CHILD);
        sq = _t_newr(g,SEMTREX_SEQUENCE);
        _sl(sq,STX_LABEL);
        _sl(sq,STX_SL);
        _sln(sq,STX_SL);

        //----------------
        // ACTION
        while (_t_matchr(sxx,tokens,&results)) {
            T *m = _t_get_match(results,STX_CHILD);
            int *path = (int *)_t_surface(_t_child(m,2));
            int x = path[_t_path_depth(path)-1];
            t = _t_get(tokens,path);
            T *parent = _t_parent(t);
            // detach and free the slash token
            T *c = _t_child(parent,++x);
            _t_detach_by_ptr(parent,c);
            _t_free(c);
            // detach and add the element following the slash as a child
            c = _t_child(parent,x);
            _t_detach_by_ptr(parent,c);
            _t_add(t,c);

            _t_free(results);
        }
        _t_free(sxx);

        dump_tokens("TOKENS_AFTER_SLASH:");


        /////////////////////////////////////////////////////
        // convert STX_SET to SEMTREX_LITERALS
        // PATTERN
        // /%<SEMTREX_SYMBOL_LITERAL:STX_EXCEPT="!"?,STX_SET>

        sxx = _t_new_root(SEMTREX_WALK);
        g = _t_news(sxx,SEMTREX_GROUP,SEMTREX_SYMBOL_LITERAL);
        sq = _t_newr(g,SEMTREX_SEQUENCE);
        t = _t_newr(sq,SEMTREX_ZERO_OR_ONE);
        t =  _t_newr(t,SEMTREX_VALUE_LITERAL);
        t = _t_newr(t,SEMTREX_VALUE_SET);
        _t_new(t,STX_EXCEPT,"!",2);
        _sl(sq,STX_SET);
        //----------------
        // ACTION
        while (_t_matchr(sxx,tokens,&results)) {
            T *m = _t_get_match(results,SEMTREX_SYMBOL_LITERAL);
            int *path = (int *)_t_surface(_t_child(m,2));
            t = _t_get(tokens,path);
            int not = semeq(_t_symbol(t),STX_EXCEPT);
            T *parent = _t_parent(t);
            if (not) {
                // throw away the EXCEPT token
                T *x = t;
                t = _t_next_sibling(t);
                _t_detach_by_ptr(parent,x);
                _t_free(x);
            }
            DO_KIDS(t,
                    T *x = _t_child(t,i);
                    char *symbol_name = (char *)_t_surface(x);
                    Symbol sy = get_symbol(symbol_name,sem);
                    __t_morph(x,SEMTREX_SYMBOL,&sy,sizeof(Symbol),1);
                    );
            t->contents.symbol = SEMTREX_SYMBOL_SET;
            T *x = _t_new_root(not?SEMTREX_SYMBOL_LITERAL_NOT:SEMTREX_SYMBOL_LITERAL);
            _t_detach_by_ptr(parent,t);
            _t_add(x,t);
            _t_insert_at(tokens,path,x);

            _t_free(results);
        }
        _t_free(sxx);
        dump_tokens("TOKENS_AFTER_LITERAL_STX_SET:");

        /////////////////////////////////////////////////////
        // convert labels to SEMTREX_LITERALS
        // PATTERN
        // /%<SEMTREX_SYMBOL_LITERAL:STX_LABEL|STX_EXCEPT>
        sxx = _t_new_root(SEMTREX_WALK);
        g = _t_news(sxx,SEMTREX_GROUP,SEMTREX_SYMBOL_LITERAL);
        o = _t_newr(g,SEMTREX_OR);
        _sl(o,STX_LABEL);
        _sl(o,STX_EXCEPT);
        //----------------
        // ACTION
        while (_t_matchr(sxx,tokens,&results)) {
            T *m = _t_get_match(results,SEMTREX_SYMBOL_LITERAL);
            int *path = (int *)_t_surface(_t_child(m,2));
            t = _t_get(tokens,path);
            char *symbol_name = (char *)_t_surface(t);
            Symbol sy = get_symbol(symbol_name,sem);
            t->contents.symbol = semeq(t->contents.symbol,STX_LABEL)?SEMTREX_SYMBOL_LITERAL:SEMTREX_SYMBOL_LITERAL_NOT;
            T *ss = _t_news(0,SEMTREX_SYMBOL,sy);
            int pp[2] = {1,TREE_PATH_TERMINATOR};
            _t_insert_at(t,pp,ss);

            _t_free(results);
        }
        _t_free(sxx);

        dump_tokens("TOKENS_AFTER_LITERAL:");

        /////////////////////////////////////////////////////
        // convert comma tokens to sequences
        // PATTERN
        // /*<SEMTREX_SEQUENCE:(!STX_COMMA,STX_COMMA)+,!STX_COMMA>  ->  SEMTREX_SEQUENCE
        sxx = _t_new_root(SEMTREX_WALK);
        g = _t_news(sxx,SEMTREX_GROUP,SEMTREX_SEQUENCE);
        sq = _t_newr(g,SEMTREX_SEQUENCE);
        o = _t_newr(sq,SEMTREX_ONE_OR_MORE);
        _sln(sq,STX_COMMA);
        sq = _t_newr(o,SEMTREX_SEQUENCE);
        _sln(sq,STX_COMMA);
        _sl(sq,STX_COMMA);

        //----------------
        // ACTION
        while (_t_matchr(sxx,tokens,&results)) {
            T *m = _t_get_match(results,SEMTREX_SEQUENCE);
            int count = *(int *)_t_surface(_t_child(m,3));
            int *path = (int *)_t_surface(_t_child(m,2));
            T *seq = _t_new_root(SEMTREX_SEQUENCE);
            T *parent = _t_parent(_t_get(tokens,path));
            int x = path[_t_path_depth(path)-1];
            while(count--) {
                t =  _t_child(parent,x);
                _t_detach_by_ptr(parent,t);
                if (semeq(STX_COMMA,_t_symbol(t)))
                    _t_free(t);
                else
                    _t_add(seq,t);
            }
            if (_t_children(parent) == 0) {
                _t_add(parent,seq);
            }
            else {
                _t_insert_at(tokens,path,seq);
            }
            _t_free(results);
        }
        _t_free(sxx);

        dump_tokens("TOKENS_AFTER_COMMA:");


        /////////////////////////////////////////////////////
        // convert ors
        // PATTERN
        // /%<SEMTREX_OR:!STX_OR,STX_OR,!STX_OR>
        sxx = _t_new_root(SEMTREX_WALK);
        g = _t_news(sxx,SEMTREX_GROUP,SEMTREX_OR);
        sq = _t_newr(g,SEMTREX_SEQUENCE);
        _sln(sq,STX_OR);
        _sl(sq,STX_OR);
        _sln(sq,STX_OR);

        //----------------
        // ACTION
        while (_t_matchr(sxx,tokens,&results)) {
            T *m = _t_get_match(results,SEMTREX_OR);
            int *path = (int *)_t_surface(_t_child(m,2));
            int x = path[_t_path_depth(path)-1];
            t = _t_get(tokens,path);
            T *parent = _t_parent(t);
            // detach the or's children
            T *c1 = _t_child(parent,x);
            _t_detach_by_ptr(parent,c1);
            T *c2 = _t_child(parent,x+1);
            _t_detach_by_ptr(parent,c2);
            o = _t_child(parent,x);
            _t_add(o,c1);
            _t_add(o,c2);
            o->contents.symbol = SEMTREX_OR;
            _t_free(results);
        }
        _t_free(sxx);

        dump_tokens("TOKENS_AFTER_ORS:");

        /////////////////////////////////////////////////////
        // fixup STX_WALK
        // PATTERN
        // /%<SEMTREX_WALK:STX_WALK,.>
        sxx = _t_new_root(SEMTREX_WALK);
        g = _t_news(sxx,SEMTREX_GROUP,SEMTREX_WALK);
        sq = _t_newr(g,SEMTREX_SEQUENCE);
        _sl(sq,STX_WALK);
        _t_newr(sq,SEMTREX_SYMBOL_ANY);
        //----------------
        // ACTION
        while (_t_matchr(sxx,tokens,&results)) {
            T *m = _t_get_match(results,SEMTREX_WALK);
            int *path = (int *)_t_surface(_t_child(m,2));
            t = _t_get(tokens,path);
            T *parent = _t_parent(t);
            int x = path[_t_path_depth(path)-1];
            T *c = _t_child(parent,x+1);
            _t_detach_by_ptr(parent,c);
            _t_add(t,c);
            t->contents.symbol = SEMTREX_WALK;

            _t_free(results);
        }
        _t_free(sxx);

        dump_tokens("TOKENS_AFTER_WALK:");

        /////////////////////////////////////////////////////
        // remove stray STX_SIBS
        // PATTERN
        // /%<STX_SIBS:STX_SIBS>
        sxx = _t_new_root(SEMTREX_WALK);
        g = _t_news(sxx,SEMTREX_GROUP,STX_SIBS);
        _sl(g,STX_SIBS);
        //----------------
        // ACTION
        while (_t_matchr(sxx,tokens,&results)) {
            T *m = _t_get_match(results,STX_SIBS);
            int *path = (int *)_t_surface(_t_child(m,2));
            t = _t_get(tokens,path);
            T *parent = _t_parent(t);
            if (_t_children(t) > 1) {
                __t_dump(G_sem,tokens,0,buf);
                raise_error("sibs with more than one child! [tokens:%s]",buf);
            }
            int x = path[_t_path_depth(path)-1];
            T *c = _t_child(t,1);
            _t_detach_by_ptr(t,c);
            _t_replace(parent,x,c);
            _t_free(results);
        }
        _t_free(sxx);

        dump_tokens("TOKENS_FINAL:");

        int c = _t_children(tokens);
        t =_t_child(tokens,1);
        Symbol sy = _t_symbol(t);
        if (c == 2 && (semeq(STX_SL,sy))) {
            t = _t_child(tokens,2);
            _t_detach_by_ptr(tokens,t);
        }
        else {
            __t_dump(G_sem,tokens,0,buf);
            raise_error("unexpected tokens! [tokens:%s]",buf);
        }
        _t_free(tokens);

    }
    _t_free(ts);
    _t_free(s);

    return t;
}

// recursable implementation of _stx_results2sem_map
void __stx_r2fi(SemTable *sem,T *mr,T *mt, T *sem_map) {
    T *t = _t_newr(sem_map,SEMANTIC_LINK);
    T *match_symbol =  _t_child(mr,SemtrexMatchSymbolIdx);
    Symbol msym = *(Symbol*)_t_surface(match_symbol);
    _t_news(t,USAGE,msym);
    T *r = _t_newr(t,REPLACEMENT_VALUE);

    int *path = (int *)_t_surface(_t_child(mr,SemtrexMatchPathIdx));
    // will just use the first sibling... *sibs = *(int*)_t_surface(_t_child(m,SemtrexMatchSibsIdx));
    T *x = _t_get(mt,path);
    if (!x) {
        raise_error("expecting to get a value from match!!");
    }
    x = _t_clone(x);

    int c = _t_children(mr);

    if (!semeq(msym,NULL_SYMBOL)) {
        Symbol xsym = _t_symbol(x);
        Structure mrs = _sem_get_symbol_structure(sem,msym);
        Structure xs = _sem_get_symbol_structure(sem,xsym);

        // the structures are the same then we can just set the symbol type
        if (semeq(mrs,xs)) {
            x->contents.symbol = msym;
        }
        else {
            // otherwise try embody from match
            T *e = _t_embody_from_match(sem,mr,msym,mt);
            if (!e)
            raise_error("unable to embody from match: %s(%s) -> %s(%s)",
                        _sem_get_name(sem,xsym),_sem_get_name(sem,xs),
                        _sem_get_name(sem,msym),_sem_get_name(sem,msym)
                        );
            _t_free(x);
            x = e;
        }
    }
    _t_add(r,x);
    int i;
    for (i=SemtrexMatchSibsIdx+1;i<=c;i++) {
        __stx_r2fi(sem,_t_child(mr,i),mt,sem_map);
    }
}

/**
 * create SEMANTIC_MAP tree from a SEMTREX_MATCH tree for use in filling templates
 *
 * @param[in] match_results results tree from _t_matchr
 * @param[in] match_tree the tree the semtrex was matched against
 * @results a SEMANTIC_MAP tree
 */
T *_stx_results2sem_map(SemTable *sem,T *match_results,T *match_tree) {
    T *sem_map = _t_new_root(SEMANTIC_MAP);
    __stx_r2fi(sem,match_results,match_tree,sem_map);
    return sem_map;
}

char trbuf[255];
char *transition2Str(TransitionType transition) {
    trbuf[0]=0;
    if (transition == TransitionDown) sprintf(trbuf,"Down");
    else if (isTransitionPop(transition)) sprintf(trbuf,"Up%d",transition*-1);
    if (isTransitionNext(transition)) {
        sprintf(trbuf+strlen(trbuf),"%sNext",trbuf[0]?"+":"");
    }
    return trbuf;
}
// debugging code to dump out an ascii representation of the stx fsa
#include "ansicolor.h"
static int dump_id = 99;
SState *G_cur_stx_state = NULL;
char G_stx_dump_buf[10000];
#define pbuf(...) sprintf(buf+strlen(buf),__VA_ARGS__)

char * __stx_dump_state(SState *s,char *buf) {
    switch (s->type) {
    case StateMatch:
        pbuf("(M)");
        break;
    case StateGroupOpen:
        pbuf("{%d:%s",s->data.groupo.uid,_sem_get_name(G_sem,s->data.groupo.symbol));
        break;
    case StateGroupClose:
        pbuf("%d:%s}",s->data.groupc.openP->data.groupo.uid,_sem_get_name(G_sem,s->data.groupc.openP->data.groupo.symbol));
        break;
    case StateSymbol:
        {
            Symbol sym;
            T *x;
            x = s->data.symbol.symbols;
            if (semeq(_t_symbol(x),SEMTREX_SYMBOL))
                sym = *(Symbol *)_t_surface(x);
            else raise_error("unimplemented state data type in stx_dump\n");
            pbuf("(%s%s)",(s->data.symbol.flags & LITERAL_NOT) ? "!" : "",
                 _sem_get_name(G_sem,sym));
        }
        break;
    case StateValue:
        pbuf("(%sV",(s->data.value.flags & LITERAL_NOT) ? "!" : "");
        if (s->data.value.flags & LITERAL_SET) {
            T *x = s->data.value.values;
            int c = _t_children(x);
            int i;

            for(i=1;i<=c;i++) {
                pbuf("%s;",t2s(_t_child(x,i)));
            }
        }
        else {
            pbuf("%s",t2s(_t_child(_t_child(s->data.value.values,1),1)));
        }
        pbuf(")");
        break;
    case StateAny:
        pbuf("(.)");
        break;
    case StateDescend:
        printf("(/)");
        break;
    case StateNot:
        pbuf("(~)");
        break;
    case StateSplit:
        pbuf("(S)");
        break;
    case StateWalk:
        pbuf("(%%)");
        break;
    default:
             raise_error("unknown state");
        pbuf("(\?\?)");
    }
    return buf;
}

void __stx_dump(SState *s,char *buf) {
    if (s->_did == dump_id) {pbuf("X");return;}
    s->_did = dump_id;
    if (s == G_cur_stx_state)
        pbuf(KRED);
    __stx_dump_state(s,buf);
    if (s == G_cur_stx_state)
        pbuf(KNRM);
    if (s->out) {
        pbuf("-%s->",transition2Str(s->transition));
        __stx_dump(s->out,buf);
    }
    if (s->out1) {
        pbuf("[-%s->",transition2Str(s->transition1));
        __stx_dump(s->out1,buf);
        pbuf("]");
    }
    //        printf("\n");
}

char * _stx_dump(SState *s,char *buf) {
    ++dump_id;
    buf[0] = 0;
    __stx_dump(s,buf);
    return buf;
}

void stx_dump(T *s) {
    int l;

    SState *f = _stx_makeFA(s,&l);    _stx_dump(f,G_stx_dump_buf);
    puts(G_stx_dump_buf);
    _stx_freeFA(f);
}

/**@}*/
