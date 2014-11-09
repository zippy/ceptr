/**
 * @ingroup semtrex
 *
 * @{
 * @file semtrex.c
 * @brief semtrex implementation
 *
 * This file implements a regular expression type language usefull for searching semantic trees.
 *
 * The code to generate finite state automata to match the trees is based heavily on
 * based on Russ Cox's great work, see: http://swtch.com/~rsc/regexp/regexp1.html
 *
 * @copyright Copyright (C) 2013-2014, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 */

#include "semtrex.h"
#include "def.h"

/// the final matching state in the FSA can be declared statically and globally
SState matchstate = {StateMatch}; /* only one instance of the match state*/

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
	(*(int *)(l+1)) += -level;
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


#define state(t,sP) _state(t,sP,0)
/**
 * utility routine to initialize a state struct
 */
SState *_state(StateType type,int *statesP,int extra) {
    SState *s = malloc(sizeof(SState)+extra);
    s->out = NULL;
    s->out1 = NULL;
    s->type = type;
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

    int c = _t_children(t);
    Symbol sym = _t_symbol(t);
    switch(sym.id) {
    case SEMTREX_VALUE_LITERAL_ID:
	state_type = StateValue;
	s = state(state_type,statesP);
	s->data.value.symbol = *(Symbol *)_t_surface(t);

	// copy the value from the semtrex into the state
	s->data.value.value = _t_clone(t);
	*in = s;
	s->transition = level;
	*out = list1(&s->out);
	break;
    case SEMTREX_SYMBOL_LITERAL_ID:
	if (state_type == -1) state_type = StateSymbol;
    case SEMTREX_SYMBOL_EXCEPT_ID:
	if (state_type == -1) state_type = StateSymbolExcept;
    case SEMTREX_SYMBOL_ANY_ID:
	if (state_type == -1) state_type = StateAny;
	if (c > 1) return "Literal must have 0 or 1 children";
	if ((state_type == StateSymbol) || (state_type == StateSymbolExcept)) {
	    s = state(state_type,statesP);
	    s->data.symbol = *(Symbol *)_t_surface(t);
	}
	else
	    s = state(state_type,statesP);
	*in = s;
	if (c > 0) {
	    s->transition = TransitionDown;
	    err = __stx_makeFA(_t_child(t,1),&i,&o,level-1,statesP);
	    if (err) return err;
	    s->out = i;
	    *out = o;
	}
	else {
	    s->transition = level;
	    *out = list1(&s->out);
	}
	break;
    case SEMTREX_SEQUENCE_ID:
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
	if (c != 2) return "Or must have 2 children";
	s = state(StateSplit,statesP);
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
	if (c != 1) return "Star must have 1 child";
	s = state(StateSplit,statesP);
	*in = s;
	err = __stx_makeFA(_t_child(t,1),&i,&o,level,statesP);
	if (err) return err;
	s->out = i;
	patch(o,s,level);
	*out = list1(&s->out1);
	break;
    case SEMTREX_ONE_OR_MORE_ID:
	if (c != 1) return "Plus must have 1 child";
	s = state(StateSplit,statesP);
	err = __stx_makeFA(_t_child(t,1),&i,&o,level,statesP);
	if (err) return err;
	*in = i;
	s->out = i;
	patch(o,s,level);
	*out = list1(&s->out1);
	break;
    case SEMTREX_ZERO_OR_ONE_ID:
	if (c != 1) return "Question must have 1 child";
	s = state(StateSplit,statesP);
	*in = s;
	err = __stx_makeFA(_t_child(t,1),&i,&o,level,statesP);
	if (err) return err;
	s->out = i;
	*out = append(o,list1(&s->out1));
	break;
    case SEMTREX_GROUP_ID:
	if (c != 1) return "Group must have 1 child";
	s = state(StateGroupOpen,statesP);
	*in = s;
	group_symbol = *(SemanticID *)_t_surface(t);
	group_id = ++G_group_id;
	s->data.groupo.symbol = group_symbol;
	s->data.groupo.uid = group_id;
	err = __stx_makeFA(_t_child(t,1),&i,&o,level,statesP);
	if (err) return err;
	s->out = i;
	s1 = state(StateGroupClose,statesP);
	patch(o,s1,level);
	s1->data.groupc.openP = s;
	*out = list1(&s1->out);
	break;
    case SEMTREX_DESCEND_ID:
	if (c != 1) return "Descend must have 1 child";
	s = state(StateDescend,statesP);
	*in = s;
	err = __stx_makeFA(_t_child(t,1),&i,&o,level-1,statesP);
	if (err) return err;
	s->out = i;
	*out = o;
	break;
    case SEMTREX_NOT_ID:
	if (c != 1) return "Not must have 1 child";
	s = state(StateNot,statesP);
	*in = s;
	err = __stx_makeFA(_t_child(t,1),&i,&o,level,statesP);
	if (err) return err;
	s->out = i;
	*out = append(o,list1(&s->out1));
	break;
    case SEMTREX_WALK_ID:
	if (c != 1) return "Walk must have 1 child";
	s = state(StateWalk,statesP);
	*in = s;
	err = __stx_makeFA(_t_child(t,1),&i,&o,level,statesP);
	if (err) return err;
	s->out = i;
	*out = o;
	break;
    default:
	return "Unknown SEMTREX SYMBOL";
    }
    return 0;
}

void _stx_dump(SState *s);

/**
 * wrapper function for building the finite state automata recursively and patching it to the final match state
 */
SState * _stx_makeFA(T *t,int *statesP) {
    SState *in;
    Ptrlist *o;
    G_group_id = 0;
    char *err = __stx_makeFA(t,&in,&o,0,statesP);
    if (err != 0) {raise_error0(err);}
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
__stx_freeFA2(SState *s) {
    if (s->out) __stx_freeFA2(s->out);
    if (s->out1) __stx_freeFA2(s->out1);
    if (s->type == StateValue) {
	_t_free(s->data.value.value);
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

Defs *G_d = 0;
int G_debug_match = 0;
T *G_ts,*G_te;
#define DEBUG_MATCH
#ifdef DEBUG_MATCH
#define MATCH_DEBUGG(s,x) 	if(G_debug_match){printf("IN:" #s " for %s\n",x);__t_dump(G_d,t,0,buf);printf("  cursor:%s\n",buf);}
#define MATCH_DEBUG(s) 	if(G_debug_match){puts("IN:" #s "");__t_dump(G_d,t,0,buf);printf("  cursor:%s\n",!t ? "NULL" : buf);}
//#define MATCH_DEBUGG(s,x) 	if(G_debug_match){printf("IN:" #s "%s with %p \n",x,t);}
//#define MATCH_DEBUG(s) 	if(G_debug_match){printf("IN:" #s " with %p\n",t);}

#else
#define MATCH_DEBUG(s)
#endif

int __symbol_match(SState *s,T *t) {
    return t && semeq(s->data.symbol,_t_symbol(t));
}

int __transition_match(SState *s,T *t,T **r) {
    int i;
    if (!t) return 0;
    if (s->out == &matchstate) return 1;
    switch(s->transition) {
    case TransitionNextChild:
	t = _t_next_sibling(t);
	G_te =t;
	return __t_match(s->out,t,r);
	break;
    case TransitionDown:
	t = _t_child(t,1);
	G_te = t;
	return __t_match(s->out,t,r);
	break;
    default:
	for(i=s->transition;i<0;i++) {
	    t = _t_parent(t);
	}
	t = _t_next_sibling(t);
	G_te = t;
	return __t_match(s->out,t,r);
	break;
    }
}

T * __transition(SState *s,T *t) {
    int i;
    if (!t) return 0;
    switch(s->transition) {
    case TransitionNextChild:
	t = _t_next_sibling(t);
	break;
    case TransitionDown:
	t = _t_child(t,1);
	break;
    default:
	for(i=s->transition;i<0;i++) {
	    t = _t_parent(t);
	}
	t = _t_next_sibling(t);
	break;
    }
    return t;
}

int _val_match(T *t,T *t1) {

    int i;
    char *p1,*p2;
    size_t l = _t_size(t1);
    if (l != _t_size(t)) return 0;

    return memcmp(_t_surface(t),_t_surface(t1),l)==0;
}
#define _next_pair(l)     l = (size_t *)(sizeof(size_t)+*l+(void *)l);

#define FAIL {s=0;break;}

typedef struct BranchPoint {
    T *walk;
    SState *s;
    T *cursor;
    T *match;
    int *r_path;
} BranchPoint;

#define _PUSH_BRANCH(state,c,w) {					\
	if(G_debug_match)puts("pushing");				\
	if((depth+1)>=MAX_BRANCH_DEPTH) {raise_error0("MAX branch depth exceeded");} \
	stack[depth].s = state;						\
	stack[depth].cursor = c;					\
	stack[depth].walk = w;						\
	if (rP) {							\
	    if (*rP) {							\
		stack[depth].match = _t_clone(*rP);			\
		stack[depth].r_path = _t_get_path(r);			\
	    }								\
	    else stack[depth].match = 0;				\
	}								\
	depth++;							\
}

// convert cpointer SEMTREX_MATCH_CURSOR elements to MATCHED_PATH and SIBLING COUNT elements
void __fix(T *source_t,T *r) {
    int pt[2] = {2,TREE_PATH_TERMINATOR};
    T *m1,*m2;
    T *t1 = *(T **)_t_surface(m1 = _t_get(r,pt));
    pt[0] = 3;
    T *t = *(T **)_t_surface(m2 = _t_get(r,pt));

    int *p = _t_get_path(t1);
    __t_morph(m1,SEMTREX_MATCHED_PATH,p,sizeof(int)*(_t_path_depth(p)+1),1);

    int d = _t_path_depth(p);
    int i;

    d--;
    if (!t) {
	T *parent = _t_parent(t1);
	if (!parent) i = 1;
	else {
	    int pc = _t_children(parent);
	    i = pc - p[d] + 1;
	}
    }
    else {
	int* p_end;
	p_end = _t_get_path(t);
	i = p_end[d]-p[d];
	if (!p_end) {
	    raise(SIGINT);
	}
	free(p_end);
    }
    free(p);
    __t_morph(m2,SEMTREX_MATCH_SIBLINGS_COUNT,&i,sizeof(int),0);
    int c = _t_children(r);
    for(i=4;i<=c;i++) {
	t = _t_child(r,i);
	__fix(source_t,t);
    }
}

#define MAX_BRANCH_DEPTH 500

#define PUSH_BRANCH(state,c) _PUSH_BRANCH(state,c,0)
#define PUSH_WALK_POINT(state,c) _PUSH_BRANCH(state,c,c)

#define TRANSITION(x) if (!t) {FAIL;}; if (x) {FAIL;}; t = __transition(s,t); s = s->out;

/**
 * Walk the FSA in s using a recursive backtracing algorithm to match the tree in t.
 *
 * @param[in] s FSA to use for matching a tree
 * @param[in] t tree to match against
 * @param[inout] r match results tree being built.  (nil if no results needed)
 * @returns 1 or 0 if matched or not
 */
int __t_match(T *semtrex,T *source_t,T **rP) {
    int states;
    char buf[5000];
    BranchPoint stack[MAX_BRANCH_DEPTH];

    int depth = 0;
    T *t1,*t = source_t;
    int matched;
    T *r = 0;
    if (rP) *rP = 0;

    SgroupOpen *o;

    SState *fa = _stx_makeFA(semtrex,&states);
    SState *s = fa;

    while (s && s != &matchstate) {
	switch(s->type) {
	case StateValue:
	    MATCH_DEBUG(Value);
	    if (!t) {FAIL;}
	    else {
		T *v = s->data.value.value;
		T *n = _t_child(v,1);
		int not;
		not = semeq(_t_symbol(n),SEMTREX_VALUE_LITERAL_NOT);

		if (!__symbol_match(s,t)) FAIL;
		int count = _t_children(v);
		int i;
		if (G_debug_match) {
		    __t_dump(G_d,v,0,buf);printf("  seeking:%s\n",buf);
		}
		if (not) {
		    for(i=2;i<=count && (matched = !_val_match(t,_t_child(v,i)));i++);
		}
		else {
		    for(i=1;i<=count && !(matched = _val_match(t,_t_child(v,i)));i++);
		}

		if (!matched) FAIL;
	    }
	    t = __transition(s,t);
	    s = s->out;
	    break;
	case StateSymbol:
	    MATCH_DEBUG(Symbol);
	    TRANSITION(!__symbol_match(s,t));
	    break;
	case StateSymbolExcept:
	    MATCH_DEBUG(SymbolExcept);
	    TRANSITION(__symbol_match(s,t));
	    break;
	case StateAny:
	    MATCH_DEBUG(Any);
	    TRANSITION(0);
	    break;
	case StateSplit:
	    MATCH_DEBUG(Split);
	    PUSH_BRANCH(s->out1,t);
	    s = s->out;
	    break;
	case StateWalk:
	    MATCH_DEBUG(Walk);
	    s = s->out;
	    PUSH_WALK_POINT(s,t);
	    break;
	case StateGroupOpen:
	    o = &s->data.groupo;
	    MATCH_DEBUGG(GroupOpen,_d_get_symbol_name(G_d?G_d->symbols:0,o->symbol));
	    if (!rP) {
		// if we aren't collecting up match results simply follow groups through
		s = s->out;
	    }
	    else {
		if (!t) FAIL;

		r = _t_newi(r,SEMTREX_MATCH,o->uid);
		if (!*rP) *rP = r;
		T *x = _t_news(r,SEMTREX_MATCH_SYMBOL,o->symbol);
		_t_new(r,SEMTREX_MATCH_CURSOR,&t,sizeof(t));
		s = s->out;
	    }
	    break;
	case StateGroupClose:
	    // get the match structure from the GroupOpen state pointed to by this state
	    o = &s->data.groupc.openP->data.groupo;
	    MATCH_DEBUGG(GroupClose,_d_get_symbol_name(G_d?G_d->symbols:0,o->symbol));
	    if (rP) {

		int pt[2] = {3,TREE_PATH_TERMINATOR};
		T *x = _t_new(0,SEMTREX_MATCH_CURSOR,&t,sizeof(t));
		_t_insert_at(r, pt, x);

		T *pp = _t_parent(r);
		if (pp) r = pp;
	    }
	    s = s->out;
	    break;
	case StateDescend:
	    MATCH_DEBUG(Descend);
	    t = _t_child(t,1);
	    G_te = t;
	    s = s->out;
	    break;
	case StateMatch:
	    MATCH_DEBUG(Match);
	    break;
	}
	// if we just had a fail see if there is some backtracking we can do
	if (!s && depth) {
	    --depth;
	    if (rP) {
		if (*rP) _t_free(*rP);
		if (*rP = stack[depth].match) {
		    r = _t_get(*rP,stack[depth].r_path);
		    free(stack[depth].r_path);
		}
		else r = 0;
	    }
	    if (G_debug_match) {
		puts("Fail: so popping to--");
		__t_dump(G_d,r,0,buf);
		puts(buf);
	    }
	    s = stack[depth].s;
	    t = stack[depth].cursor;
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
			    if (t = _t_next_sibling(p)) break;
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
	    if (G_debug_match) {
		puts("FIXING:");
		__t_dump(G_d,*rP,0,buf);
		puts(buf);
		//	raise(SIGINT);
	    }

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
	    if (r = stack[depth].match) {
		_t_free(r);
		free(stack[depth].r_path);
	    }
	}
    }
    _stx_freeFA(fa);
    return s == &matchstate;
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

/**
 * extract the portion of a semtrex match results that corresponds with a given group symbol
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

T *_t_embody_from_match(Defs *defs,T *match,T *t) {
    Symbol s = *(Symbol *)_t_surface(_t_child(match,1));
    T *e;
    int i,j = _t_children(match);
    if (j > 3) {
	e = _t_new_root(s);
	for(i=4;i<=j;i++) {
	    T *c = _t_child(match,i);
	    if (c) {
		_t_add(e,_t_embody_from_match(defs,c,t));
	    }
	}
    }
    else {
	int *p;
	int children = *(int *)_t_surface(_t_child(match,3));
	T *structures = defs ? defs->structures : 0;
	T *symbols = defs ? defs->symbols : 0;
	Structure st = _d_get_symbol_structure(symbols,s);
	T *x;
	switch(st.id) {
	case CSTRING_ID:
	    return asciiT_tos(t,match,0,s);
	case INTEGER_ID:
	    return asciiT_toi(t,match,0,s);
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
char * __dump_semtrex(Defs defs,T *s,char *buf);

void __stxd_multi(Defs defs,char *x,T *s,char *buf) {
    char b[4000];
    T *sub = _t_child(s,1);
    sprintf(buf,(_t_children(s)>1 || _t_symbol(sub).id==SEMTREX_SEQUENCE_ID) ? "(%s)%s" : "%s%s",__dump_semtrex(defs,sub,b),x);
}
void __stxd_descend(Defs defs,T *s,char *v,char *buf) {
    if(_t_children(s)>0) {
	char b[4000];
	T *sub = _t_child(s,1);
	sprintf(buf,_t_children(sub)>1?"%s/(%s)":"%s/%s",v,__dump_semtrex(defs,sub,b));
    }
    else sprintf(buf,"%s",v);
}

char * __dump_semtrex(Defs defs,T *s,char *buf) {
    Symbol sym = _t_symbol(s);
    char b[5000];
    char b1[5000];
    char *sn,*bx;
    T *t;
    int i,c;
    SemanticID sem;
    switch(sym.id) {
    case SEMTREX_VALUE_LITERAL_ID:
	sem = *(SemanticID *)_t_surface(s);
	sn = _d_get_symbol_name(defs.symbols,sem);
	if (*sn=='<')
	    sprintf(b,"%d.%d.%d",sem.context,sem.flags,sem.id);
	else
	    sprintf(b,"%s",sn);
	Structure st = _d_get_symbol_structure(defs.symbols,sem);
	T *n = _t_child(s,1);
	int not;
	if (not = semeq(_t_symbol(n),SEMTREX_VALUE_LITERAL_NOT)) {
	    sprintf(b+strlen(b),"!");
	}
	sprintf(b+strlen(b),"=");
	int count = _t_children(s);
	if ((count-not) > 1)
	    sprintf(b+strlen(b),"{");
	for(i=1+not;i<=count;i++) {
	    T *x = _t_child(s,i);
	    if (semeq(st,CSTRING))
		sprintf(b+strlen(b),"\"%s\"",(char *)(_t_surface(x)));
	    else if (semeq(st,CHAR))
		sprintf(b+strlen(b),"'%c'",*(char *)(_t_surface(x)));
	    else if (semeq(st,INTEGER))
		sprintf(b+strlen(b),"%d",*(int *)(_t_surface(x)));
	    else sprintf(b+strlen(b),"???x");
	    if (i < count)
		sprintf(b+strlen(b),",");
	}
	if ((count - not) > 1)
	    sprintf(b+strlen(b),"}");
	sprintf(buf,"%s",b);
	break;
    case SEMTREX_SYMBOL_EXCEPT_ID:
    case SEMTREX_SYMBOL_LITERAL_ID:

	if (semeq(sym, SEMTREX_SYMBOL_EXCEPT)) {
	    b[0] = '!';
	    bx = &b[1];
	}
	else bx = b;
	sem = *(SemanticID *)_t_surface(s);
	sn = _d_get_symbol_name(defs.symbols,sem);
	// ignore "<unknown symbol"
	if (*sn=='<')
	    sprintf(bx,"%d.%d.%d",sem.context,sem.flags,sem.id);
	else
	    sprintf(bx,"%s",sn);
	__stxd_descend(defs,s,b,buf);
	break;
    case SEMTREX_SYMBOL_ANY_ID:
	sprintf(b,".");
	__stxd_descend(defs,s,b,buf);
	break;
    case SEMTREX_SEQUENCE_ID:
	sn = buf;
	DO_KIDS(s,
	    sprintf(sn,i<_c ? "%s,":"%s",__dump_semtrex(defs,_t_child(s,i),b));
	    sn += strlen(sn);
		);
	break;
    case SEMTREX_OR_ID:
	t = _t_child(s,1);
	sn = __dump_semtrex(defs,t,b);
	sprintf(buf,(_t_children(t) > 0) ? "(%s)|":"%s|",sn);
	t = _t_child(s,2);
	sn = __dump_semtrex(defs,t,b);
	sprintf(buf+strlen(buf),(_t_children(t) > 0) ? "(%s)":"%s",sn);
	break;
    case SEMTREX_NOT_ID:
	t = _t_child(s,1);
	sn = __dump_semtrex(defs,t,b);
	sprintf(buf,"~%s",sn);
	break;
    case SEMTREX_ZERO_OR_MORE_ID:
	__stxd_multi(defs,"*",s,buf);
	break;
    case SEMTREX_ONE_OR_MORE_ID:
	__stxd_multi(defs,"+",s,buf);
	break;
    case SEMTREX_ZERO_OR_ONE_ID:
	__stxd_multi(defs,"?",s,buf);
	break;
    case SEMTREX_GROUP_ID:
	sn = _d_get_symbol_name(defs.symbols,*(Symbol *)_t_surface(s));
	// ignore "<unknown symbol"
	if  (*sn=='<')
	    sprintf(buf, "<%s>",__dump_semtrex(defs,_t_child(s,1),b));
	else
	    sprintf(buf, "<%s:%s>",sn,__dump_semtrex(defs,_t_child(s,1),b));
	break;
    case SEMTREX_DESCEND_ID:
	sprintf(buf, "/%s",__dump_semtrex(defs,_t_child(s,1),b));
	break;
    case SEMTREX_WALK_ID:
	sprintf(buf, "%%%s",__dump_semtrex(defs,_t_child(s,1),b));
	break;
    }
    return buf;
}

char * _dump_semtrex(Defs defs,T *s,char *buf) {
    buf[0] = '/';
    __dump_semtrex(defs,s,buf+1);
    return buf;
}

// helper to add a stx_char value literal to a semtrex
T *__stxcv(T *p,char c) {
    T *t =  _t_news(p,SEMTREX_VALUE_LITERAL,ASCII_CHAR);
    _t_newi(t,ASCII_CHAR,c);
    return t;
}

T *__stxcvm(T *p,int not,int count,...) {
    va_list chars;
    T *t =  _t_news(p,SEMTREX_VALUE_LITERAL,ASCII_CHAR);
    if (not) {
	_t_newr(t,SEMTREX_VALUE_LITERAL_NOT);
    }
    va_start(chars,count);
    int i;
    for(i=0;i<count;i++) {
	_t_newi(t,ASCII_CHAR,va_arg(chars,int));
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
    _stxcs(stxx,"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");
}

// temporary function until we get system label table operational
Symbol _get_symbol(char *symbol_name,Defs *d,Context ctx) {
    if (d->symbols) {
	int i,c = _t_children(d->symbols);
	for(i=1;i<=c;i++) {
	    T *t = _t_child(d->symbols,i);
	    T *c = _t_child(t,1);
	    if (!strcmp(symbol_name,(char *)_t_surface(c))) {
		Symbol r = {ctx,SEM_TYPE_SYMBOL,i};
		return r;
	    }
	}
    }
    return NULL_SYMBOL;
}

Symbol get_symbol(char *symbol_name,Defs *d) {
    Symbol s = _get_symbol(symbol_name,&G_sys_defs,SYS_CONTEXT);
    if (!semeq(s,NULL_SYMBOL))
	return s;
    return _get_symbol(symbol_name,d,RECEPTOR_CONTEXT);
}

//#define DUMP_TOKENS
#ifdef DUMP_TOKENS
#define dump_tokens(str) puts(str);__t_dump(0,tokens,0,buf);puts(buf);
#else
#define dump_tokens(str)
#endif

/*
   a utility function to move the contents of paren/group tokens as children of the
   open token.
   Assumes the semtrex results was from a semtrex of the form:
     ...  {STX_OP:STX_OP,{STX_SIBS:!STX_CP+},STX_CP}
     where the contents is marked by one group (in the case above STX_SIBS) and the
     the whole thing is marked by an "open" group

 */
T *wrap(T *tokens,T *results, Symbol contents_s, Symbol open_s) {
    T *m = _t_get_match(results,contents_s);
    T *om = _t_get_match(results,open_s);

/*    char buf[2000];
    __t_dump(0,results,0,buf);
    puts("WRAP:");
    puts(buf);
*/
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
	_t_newi(s,ASCII_CHAR,*c);
	c++;
    }
    return s;
}

/**
 * convert ascii tokens from a match to an integer and add them to the given tree
 */
T *asciiT_toi(T* asciiT,T* match,T *t,Symbol s) {
    char buf[10];
    int sibs = *(int *)_t_surface(_t_child(match,3));
    int *path = (int *)_t_surface(_t_child(match,2));
    int j,d = _t_path_depth(path);
    for(j=0;j<sibs;j++) {
	buf[j] = *(char *)_t_surface(_t_get(asciiT,path));
	path[d-1]++;
    }
    buf[j]=0;
    return _t_newi(t,s,atoi(buf));
}

/**
 * convert ascii tokens from a match to a string and add them to the given tree
*/
T *asciiT_tos(T* asciiT,T* match,T *t,Symbol s) {
    char buf[255];
    int sibs = *(int *)_t_surface(_t_child(match,3));
    int *path = (int *)_t_surface(_t_child(match,2));
    int j,d = _t_path_depth(path);
    for(j=0;j<sibs;j++) {
	buf[j] = *(char *)_t_surface(_t_get(asciiT,path));
	path[d-1]++;
    }
    buf[j]=0;
    return _t_new(t,s,buf,j+1);
}

/**
 * convert ascii tokens from a match to a char and add them to the given tree
 */
T *asciiT_toc(T* asciiT,T* match,T *t,Symbol s) {
    int *path = (int *)_t_surface(_t_child(match,2));
    int c = *(int *)_t_surface(_t_get(asciiT,path));
    return _t_newi(t,s,c);
}

/**
 * convert a cstring to semtrex tree
 * @param[in] r Receptor context for searching for symbols
 * @param[in] stx the cstring representation of a semtrex tree
 * @returns T semtrex tree
 */
T *parseSemtrex(Defs *d,char *stx) {
    // convert the string into a tree
    #ifdef DUMP_TOKENS
    printf("\nPARSING:%s\n",stx);
    #endif
    T *t,*s = makeASCIITree(stx);

    /////////////////////////////////////////////////////
    // build the token stream out of an ascii stream
    // EXPECTATION
    // "/{STX_TOKENS:(ASCII_CHARS/({STX_SL:ASCII_CHAR='/'})|(({STX_OP:ASCII_CHAR='('})|(({STX_CP:ASCII_CHAR=')'})|(({STX_PLUS:ASCII_CHAR='+'})|(({STX_COMMA:ASCII_CHAR=','})|((ASCII_CHAR='!',{STX_EXCEPT:[a-zA-Z0-9_]+})|(({STX_CG:ASCII_CHAR='}'})|(({STX_STAR:ASCII_CHAR='*'})|(({STX_LABEL:[a-zA-Z0-9_]+})|(ASCII_CHAR='{',{STX_OG:[a-zA-Z0-9_]+},ASCII_CHAR=':')))))))))+)}
    T *ts = _t_news(0,SEMTREX_GROUP,STX_TOKENS);
    T *g = _t_news(ts,SEMTREX_SYMBOL_LITERAL,ASCII_CHARS);
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
    _t_news(t,SEMTREX_SYMBOL_LITERAL,ASCII_CHAR);
    __stxcv(sq,'\'');

    o = _t_newr(o,SEMTREX_OR);
    sq = _t_newr(o,SEMTREX_SEQUENCE);
    __stxcv(sq,'\"');
    t = _t_news(sq,SEMTREX_GROUP,STX_VAL_S);
    _stxl(t);
    __stxcv(sq,'"');

    o = _t_newr(o,SEMTREX_OR);
    sq = _t_newr(o,SEMTREX_SEQUENCE);
    __stxcv(sq,'!');
    t = _t_news(sq,SEMTREX_GROUP,STX_EXCEPT);
    _stxl(t);

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
	// @todo, this should be implemented using INTERPOLATE_MATCH
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
	    else
		_t_newi(tokens,ts,0);
	}
	_t_free(results);

	dump_tokens("TOKENS:");
	T *sxx,*sq;

	/////////////////////////////////////////////////////
	// convert STX_EQ/STX_NEQ to SEMTREX_VALUE_LITERALS
	// EXPECTATION
	// /%<SEMTREX_VALUE_LITERAL:STX_EQ|STX_NEQ,STX_VAL_I|STX_VAL_S|STX_VAL_C>
	sxx = _t_new_root(SEMTREX_WALK);
	g = _t_news(sxx,SEMTREX_GROUP,SEMTREX_VALUE_LITERAL);
	sq = _t_newr(g,SEMTREX_SEQUENCE);
        o = _t_newr(sq,SEMTREX_OR);
	_t_news(o,SEMTREX_SYMBOL_LITERAL,STX_EQ);
	_t_news(o,SEMTREX_SYMBOL_LITERAL,STX_NEQ);
	o = _t_newr(sq,SEMTREX_OR);
	_t_news(o,SEMTREX_SYMBOL_LITERAL,STX_VAL_I);
	o = _t_newr(o,SEMTREX_OR);
	_t_news(o,SEMTREX_SYMBOL_LITERAL,STX_VAL_S);
	_t_news(o,SEMTREX_SYMBOL_LITERAL,STX_VAL_C);
	//----------------
	// ACTION
	while (_t_matchr(sxx,tokens,&results)) {
	    T *m = _t_get_match(results,SEMTREX_VALUE_LITERAL);
	    int *path = (int *)_t_surface(_t_child(m,2));
	    t = _t_get(tokens,path);
	    Symbol sym = _t_symbol(t);
	    T *v = _t_next_sibling(t);
	    T *p = _t_parent(v);
	    _t_detach_by_ptr(p,v);

	    char *symbol_name = (char *)_t_surface(t);

	    Symbol vs = get_symbol(symbol_name,d);
	    int not =  semeq(sym,STX_EQ) ? 0 : 1;

	    __t_morph(t,SEMTREX_VALUE_LITERAL,&vs,sizeof(Symbol),1);
	    if (not) _t_newr(t,SEMTREX_VALUE_LITERAL_NOT);

	    // convert the STX_VAL structure token to the semantic type specified by the value literal
	    v->contents.symbol = vs;
	    _t_add(t,v);

	    _t_free(results);
	}
	_t_free(sxx);

	dump_tokens("TOKENS_AFTER_VALUE_LITERAL:");

	/////////////////////////////////////////////////////
	// replace paren groups with STX_SIBS list
	// EXPECTATION
	// /STX_TOKENS/.*,<STX_OP:STX_OP,<STX_SIBS:!STX_CP+>,STX_CP>
	sxx = _t_news(0,SEMTREX_SYMBOL_LITERAL,STX_TOKENS);
	sq = _t_newr(sxx,SEMTREX_SEQUENCE);
	T *st = _t_newr(sq,SEMTREX_ZERO_OR_MORE);
	_t_newr(st,SEMTREX_SYMBOL_ANY);
	T *gg = _t_news(sq,SEMTREX_GROUP,STX_OP);
	T *sq1 = _t_newr(gg,SEMTREX_SEQUENCE);
	_t_news(sq1,SEMTREX_SYMBOL_LITERAL,STX_OP);
	T *g = _t_news(sq1,SEMTREX_GROUP,STX_SIBS);
	T *any = _t_newr(g,SEMTREX_ONE_OR_MORE);
	_t_news(any,SEMTREX_SYMBOL_EXCEPT,STX_CP);
	_t_news(sq1,SEMTREX_SYMBOL_LITERAL,STX_CP);

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
	// EXPECTATION
	// /%,<STX_OG:STX_OG,<SEMTREX_GROUP:~(STX_CG|STX_OG)+>,STX_CG>
	sxx = _t_new_root(SEMTREX_WALK);
	g = _t_news(sxx,SEMTREX_GROUP,STX_OG);
	sq = _t_newr(g,SEMTREX_SEQUENCE);
	_t_news(sq,SEMTREX_SYMBOL_LITERAL,STX_OG);
	gg = _t_news(sq,SEMTREX_GROUP,SEMTREX_GROUP);
	any = _t_newr(gg,SEMTREX_ONE_OR_MORE);

	//	t = _t_newr(any,SEMTREX_NOT);
	//o=  _t_newr(t,SEMTREX_OR);
	//_t_news(o,SEMTREX_SYMBOL_LITERAL,STX_CG);
	//_t_news(o,SEMTREX_SYMBOL_LITERAL,STX_OG);
	_t_news(any,SEMTREX_SYMBOL_EXCEPT,STX_CG);
	_t_news(sq,SEMTREX_SYMBOL_LITERAL,STX_CG);

	//----------------
	// ACTION
	while (_t_matchr(sxx,tokens,&results)) {
	    g = wrap(tokens,results,SEMTREX_GROUP,STX_OG);

	    // convert the STX_OG to SEMTREX_GROUP children and free the STX_CG
	    char *symbol_name = (char *)_t_surface(g);
	    Symbol sy = get_symbol(symbol_name,d);
	    __t_morph(g,SEMTREX_GROUP,&sy,sizeof(Symbol),1);

	    _t_free(results);
	}
	_t_free(sxx);

	dump_tokens("TOKENS_AFTER_GROUPS:");

	/////////////////////////////////////////////////////
	// if there are any parens left we raise mismatch!
	// EXPECTATION
	// /(STX_TOKENS/.*,(STX_OP)|(STX_CP))
	sxx = _t_news(0,SEMTREX_SYMBOL_LITERAL,STX_TOKENS);
	sq = _t_newr(sxx,SEMTREX_SEQUENCE);
	st = _t_newr(sq,SEMTREX_ZERO_OR_MORE);
	_t_newr(st,SEMTREX_SYMBOL_ANY);
	o = _t_newr(sq,SEMTREX_OR);
	_t_news(o,SEMTREX_SYMBOL_LITERAL,STX_OP);
	_t_news(o,SEMTREX_SYMBOL_LITERAL,STX_CP);

	//----------------
	// ACTION
	if (_t_match(sxx,tokens)) {
	    __t_dump(0,tokens,0,buf);
	    raise_error("mismatched parens! [tokens:%s]",buf);
	}
	_t_free(sxx);

	/////////////////////////////////////////////////////
	// convert postfix groups
	// EXPECTATION
	// /*<STX_POSTFIX:.,STX_PLUS|STX_STAR|STX_Q>
	sxx = _t_new_root(SEMTREX_WALK);
	g = _t_news(sxx,SEMTREX_GROUP,STX_POSTFIX);
	sq = _t_newr(g,SEMTREX_SEQUENCE);
	_t_newr(sq,SEMTREX_SYMBOL_ANY);
	o = _t_newr(sq,SEMTREX_OR);
	_t_news(o,SEMTREX_SYMBOL_LITERAL,STX_PLUS);
	o = _t_newr(o,SEMTREX_OR);
	_t_news(o,SEMTREX_SYMBOL_LITERAL,STX_STAR);
	_t_news(o,SEMTREX_SYMBOL_LITERAL,STX_Q);

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
	// fixup STX_WALK
	// EXPECTATION
	// /%<SEMTREX_WALK:STX_WALK,.>
	sxx = _t_new_root(SEMTREX_WALK);
	g = _t_news(sxx,SEMTREX_GROUP,SEMTREX_WALK);
	sq = _t_newr(g,SEMTREX_SEQUENCE);
	_t_news(sq,SEMTREX_SYMBOL_LITERAL,STX_WALK);
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
	// convert not
	// EXPECTATION
	// /%<SEMTREX_NOT:STX_NOT,.>
	sxx = _t_new_root(SEMTREX_WALK);
	g = _t_news(sxx,SEMTREX_GROUP,SEMTREX_NOT);
	sq = _t_newr(g,SEMTREX_SEQUENCE);
	_t_news(sq,SEMTREX_SYMBOL_LITERAL,STX_NOT);
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
	// EXPECTATION
	// /%.*,<STX_CHILD:STX_LABEL,STX_SL,!STX_SL>
	sxx = _t_new_root(SEMTREX_WALK);
	sq = _t_newr(sxx,SEMTREX_SEQUENCE);
	any = _t_newr(sq,SEMTREX_ZERO_OR_MORE);
	_t_newr(any,SEMTREX_SYMBOL_ANY);
	g = _t_news(sq,SEMTREX_GROUP,STX_CHILD);
	sq = _t_newr(g,SEMTREX_SEQUENCE);
	_t_news(sq,SEMTREX_SYMBOL_LITERAL,STX_LABEL);
	_t_news(sq,SEMTREX_SYMBOL_LITERAL,STX_SL);
	_t_news(sq,SEMTREX_SYMBOL_EXCEPT,STX_SL);

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
	// convert comma tokens to sequences
	// EXPECTATION
	// /*<SEMTREX_SEQUENCE:(!STX_COMMA,STX_COMMA)+,!STX_COMMA>  ->  SEMTREX_SEQUENCE
	sxx = _t_new_root(SEMTREX_WALK);
	g = _t_news(sxx,SEMTREX_GROUP,SEMTREX_SEQUENCE);
	sq = _t_newr(g,SEMTREX_SEQUENCE);
	o = _t_newr(sq,SEMTREX_ONE_OR_MORE);
	_t_news(sq,SEMTREX_SYMBOL_EXCEPT,STX_COMMA);
	sq = _t_newr(o,SEMTREX_SEQUENCE);
	_t_news(sq,SEMTREX_SYMBOL_EXCEPT,STX_COMMA);
	_t_news(sq,SEMTREX_SYMBOL_LITERAL,STX_COMMA);

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
	// EXPECTATION
	// /%<SEMTREX_OR:!STX_OR,STX_OR,!STX_OR>
	sxx = _t_new_root(SEMTREX_WALK);
	g = _t_news(sxx,SEMTREX_GROUP,SEMTREX_OR);
	sq = _t_newr(g,SEMTREX_SEQUENCE);
	_t_news(sq,SEMTREX_SYMBOL_EXCEPT,STX_OR);
	_t_news(sq,SEMTREX_SYMBOL_LITERAL,STX_OR);
	_t_news(sq,SEMTREX_SYMBOL_EXCEPT,STX_OR);

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
	// convert labels to SEMTREX_LITERALS
	// EXPECTATION
	// /%<SEMTREX_SYMBOL_LITERAL:STX_LABEL|STX_EXCEPT>
	sxx = _t_new_root(SEMTREX_WALK);
	g = _t_news(sxx,SEMTREX_GROUP,SEMTREX_SYMBOL_LITERAL);
	o = _t_newr(g,SEMTREX_OR);
	_t_news(o,SEMTREX_SYMBOL_LITERAL,STX_LABEL);
	_t_news(o,SEMTREX_SYMBOL_LITERAL,STX_EXCEPT);
	//----------------
	// ACTION
	while (_t_matchr(sxx,tokens,&results)) {
	    T *m = _t_get_match(results,SEMTREX_SYMBOL_LITERAL);
	    int *path = (int *)_t_surface(_t_child(m,2));
	    t = _t_get(tokens,path);
	    char *symbol_name = (char *)_t_surface(t);
	    Symbol sy = get_symbol(symbol_name,d);
	    __t_morph(t,semeq(t->contents.symbol,STX_LABEL)?SEMTREX_SYMBOL_LITERAL:SEMTREX_SYMBOL_EXCEPT,&sy,sizeof(Symbol),1);

	    _t_free(results);
	}
	_t_free(sxx);

	dump_tokens("TOKENS_AFTER_LITERAL:");

	/////////////////////////////////////////////////////
	// remove stray STX_SIBS
	// EXPECTATION
	// /%<STX_SIBS:STX_SIBS>
	sxx = _t_new_root(SEMTREX_WALK);
	g = _t_news(sxx,SEMTREX_GROUP,STX_SIBS);
	_t_news(g,SEMTREX_SYMBOL_LITERAL,STX_SIBS);
	//----------------
	// ACTION
	while (_t_matchr(sxx,tokens,&results)) {
	    T *m = _t_get_match(results,STX_SIBS);
	    int *path = (int *)_t_surface(_t_child(m,2));
	    t = _t_get(tokens,path);
	    T *parent = _t_parent(t);
	    if (_t_children(t) > 1) {
		__t_dump(0,tokens,0,buf);
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
	    __t_dump(0,tokens,0,buf);
	    raise_error("unexpected tokens! [tokens:%s]",buf);
	}
	_t_free(tokens);

    }
    _t_free(ts);
    _t_free(s);

    return t;
}

/**@}*/
