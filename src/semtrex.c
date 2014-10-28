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
    case SEMTREX_SYMBOL_LITERAL_ID:
	if (state_type == -1) state_type = StateSymbol;
    case SEMTREX_SYMBOL_EXCEPT_ID:
	if (state_type == -1) state_type = StateSymbolExcept;
    case SEMTREX_SYMBOL_ANY_ID:
	if (state_type == -1) state_type = StateAny;
	if (c > 1) return "Literal must have 0 or 1 children";
	if (state_type == StateValue) {
	    size_t l = _t_size(t);
	    s = _state(state_type,statesP,l-sizeof(Svalue));
	    // copy the value from the semtrex into the state
	    Svalue *sv = (Svalue *)_t_surface(t);
	    memcpy(&s->data.value,sv,l);
	}
	else if ((state_type == StateSymbol) || (state_type == StateSymbolExcept)) {
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
	s->data.groupo.match_count = 0;
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
	*out = o;
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
    free(s);
}

/**
 * free the memory allocated by an FSA
 */
void _stx_freeFA(SState *s) {
    __stx_freeFA(s,++free_id);
    __stx_freeFA2(s);
}

int G_debug_match = 0;
T *G_ts,*G_te;
#define DEBUG_MATCH
#ifdef DEBUG_MATCH
#define MATCH_DEBUG(s) 	if(G_debug_match){puts("IN:" #s "");__t_dump(0,t,0,buf);printf("  with:%s\n",buf);}
#else
#define MATCH_DEBUG(s)
#endif

int __symbol_match(SState *s,T *t) {
    return t && semeq(s->data.symbol,_t_symbol(t));
}

int __transition_match(SState *s,T *t,T *r) {
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

int _val_match(T *t,size_t *l) {
    int i;
    char *p1,*p2;
    if (*l != _t_size(t)) return 0;
    else {
	p1 = (char *)(l+1); // the value comes right after the length
	p2 = _t_surface(t);
	for(i=*l;i>0;i--) {
	    if (*p1++ != *p2++) {return 0;break;}
	}
    }
    return 1;
}
#define _next_pair(l)     l = (size_t *)(sizeof(size_t)+*l+(void *)l);

/**
 * Walk the FSA in s using a recursive backtracing algorithm to match the tree in t.
 *
 * @param[in] s FSA to use for matching a tree
 * @param[in] t tree to match against
 * @param[inout] r match results tree being built.  (nil if no results needed)
 * @returns 1 or 0 if matched or not
 */
int __t_match(SState *s,T *t,T *r) {
    int i,c,matched;
    SgroupOpen *o;
    T *m,*t1;
    char buf[2000];

    //printf("tm: s:%d t:%d\n",s->data.symbol,t ? _t_symbol(t) : -1);
    switch(s->type) {
    case StateValue:
	MATCH_DEBUG(Value);
	if (!t) return 0;
	else {
	    size_t *l = &s->data.value.length;
	    if (!__symbol_match(s,t)) return 0;
            int count = s->data.value.count;
	    if (s->data.value.flags & SEMTREX_VALUE_NOT_FLAG) {
		while(count-- && (matched = !_val_match(t,l))) {
		    _next_pair(l);
		}
	    }
	    else {
		while(count-- && !(matched = _val_match(t,l))) {
		    _next_pair(l);
		}
	    }

	    if (!matched) return 0;
	}
	return __transition_match(s,t,r);
	break;
    case StateSymbol:
	MATCH_DEBUG(Symbol);
	if (!__symbol_match(s,t)) return 0;
	return __transition_match(s,t,r);
	break;
    case StateSymbolExcept:
	MATCH_DEBUG(SymbolExcept);
	if (__symbol_match(s,t)) return 0;
	return __transition_match(s,t,r);
	break;
    case StateAny:
	MATCH_DEBUG(Any);
	return __transition_match(s,t,r);
	break;
    case StateSplit:
	MATCH_DEBUG(Split);
	if (__t_match(s->out,t,r)) return 1;
	else return __t_match(s->out1,t,r);
	break;
    case StateWalk:
	MATCH_DEBUG(Walk);
	if (__t_match(s->out,t,r)) return 1;
	t1 = _t_child(t,1);
	G_ts = t1;
	if (t1 && __t_match(s,t1,r)) return 1;
	t1 = _t_next_sibling(t);
	G_ts = t1;
	if (t1 && __t_match(s,t1,r)) return 1;
	return 0;
	break;
    case StateGroupOpen:
	MATCH_DEBUG(GroupOpen);
	if (!r)
	    // if we aren't collecting up match results simply follow groups through
	    return __t_match(s->out,t,r);
	else {
	    if (!t) return 0;
	    // add on tree node to the list of match points
	    s->data.groupo.matches[s->data.groupo.match_count++] = t;
	    matched = __t_match(s->out,t,r);
	    s->data.groupo.match_count--;
	    return matched;
	}
	break;
    case StateGroupClose:
	MATCH_DEBUG(GroupClose);
	// make sure that what follows the group also matches
	matched = __t_match(s->out,t,r);
	if (matched) {
	    // get the match structure from the GroupOpen state pointed to by this state
	    o = &s->data.groupc.openP->data.groupo;
	    c = o->match_count;
	    t1 = o->matches[c-1];

	    if (t1 == 0) {
		raise_error("couldn't find match for group: %d",o->uid);
	    }

	    SemanticID match_symbol = o->symbol;
	    int match_id = o->uid;

	    //	    printf("Matched:%s\n",_d_get_symbol_name(0,match_symbol));

	    T *m = _t_newi(r,SEMTREX_MATCH,match_id);
	    _t_news(m,SEMTREX_MATCH_SYMBOL,match_symbol);
	    int *p = _t_get_path(t1);
	    T *pp = _t_new(m,SEMTREX_MATCHED_PATH,p,sizeof(int)*(_t_path_depth(p)+1));

	    int* p_end;
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
		p_end = _t_get_path(t);
		i = p_end[d]-p[d];
		free(p_end);
	    }
	    free(p);

	    _t_newi(m,SEMTREX_MATCH_SIBLINGS_COUNT,i);
	}
	return matched;
	break;
    case StateDescend:
	MATCH_DEBUG(Descend);
	t = _t_child(t,1);
	G_te = t;
	return __t_match(s->out,t,r);
	break;
    case StateNot:
	MATCH_DEBUG(Not);
	if (!t) return 0;
	//	if (G_debug_match) raise(SIGINT);
	if (!__t_match(s->out,t,r)) return __t_match(s->out1,t,r);
	else return 0;

	break;
    case StateMatch:
	MATCH_DEBUG(Match);
	return 1;
	break;
    }
    raise_error("unimplemented state type: %d",s->type);
    return 0;
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
    int states;
    int m;
    G_ts = G_te = t;
    SState *fa = _stx_makeFA(semtrex,&states);
    T *r = 0;
    if (rP) {
	r = _t_new_root(SEMTREX_MATCH_RESULTS);
    }
    m = __t_match(fa,t,r);
    _stx_freeFA(fa);
    if (rP) {
	if (!m) {_t_free(r);*rP = NULL;}
	else *rP = r;
    }
    return m;
}

/**
 * Match a tree against a semtrex
 *
 * @param[in] semtrex the semtrex pattern tree
 * @param[in] t the tree to match against the pattern
 * @returns 1 or 0 if matched or not
 */
int _t_match(T *semtrex,T *t) {
    return _t_matchr(semtrex,t,NULL);
}

/**
 * extract the portion of a semtrex match results that corresponds with a given group symbol
 * @param[in] result match results from the _t_matchr call
 * @param[in] group the uid from the semtrex group that you want the result for
 * @returns T of the match or NULL if no such match found
 */
T *_t_get_match(T *result,Symbol group)
{
    if (!result) return 0;
    T *t;
    DO_KIDS(result,
	T *match = _t_child(result,i);
	T *symb = _t_child(match,1);
	if (semeq(*(Symbol *)_t_surface(symb),group))
	    return match;
	    );
    return 0;
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
    case SEMTREX_SYMBOL_EXCEPT_ID:
    case SEMTREX_VALUE_LITERAL_ID:
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
	if (semeq(sym, SEMTREX_VALUE_LITERAL)) {
	    Svalue *sv = (Svalue *)_t_surface(s);
	    Structure st = _d_get_symbol_structure(defs.symbols,sv->symbol);
	    size_t *l = &sv->length;
	    if (sv->flags & SEMTREX_VALUE_NOT_FLAG)
		sprintf(b+strlen(b),"!");
	    sprintf(b+strlen(b),"=");
	    if ((i = sv->count) > 1)
		sprintf(b+strlen(b),"[");
	    while(i--) {
		if (semeq(st,CSTRING))
		    sprintf(b+strlen(b),"\"%s\"",(char *)(l+1));
		else if (semeq(st,CHAR))
		    sprintf(b+strlen(b),"'%c'",*(int *)(l+1));
		else if (semeq(st,INTEGER))
		    sprintf(b+strlen(b),"%d",*(int *)(l+1));
		else sprintf(b+strlen(b),"???x");
		l = (size_t *)(*l+(void *)(l+1));
		if (i > 0)
		    sprintf(b+strlen(b),",");
	    }
	    if (i = sv->count > 1)
		sprintf(b+strlen(b),"]");
	}
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
	    sprintf(buf, "{%s}",__dump_semtrex(defs,_t_child(s,1),b));
	else
	    sprintf(buf, "{%s:%s}",sn,__dump_semtrex(defs,_t_child(s,1),b));
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

// helper to add a stx_char value litteral to a semtrex
T *__stxcv(T *stxx,char c) {
    Svalue sv;
    sv.symbol = ASCII_CHAR;
    sv.flags = 0;
    sv.count = 1;
    sv.length = sizeof(int);
    *(int *)&sv.value = c;
    return _t_new(stxx,SEMTREX_VALUE_LITERAL,&sv,sizeof(Svalue));
}

// helper to add a stx_char value litteral set to a semtrex
T *__stxcvm(T *stxx,int not,int count,...) {
    va_list chars;
    int i;
    T *t;
    size_t svl = sizeof(Svalue) + (sizeof(size_t) + sizeof(char))*count;
    Svalue *svP = malloc(svl);
    svP->flags = not ? SEMTREX_VALUE_NOT_FLAG:0;
    svP->count = count;
    svP->symbol = ASCII_CHAR;
    size_t *l = &svP->length;
    va_start(chars,count);
    for(i=0;i<count;i++) {
	*l = sizeof(char);
	*(char *)(l+1) = va_arg(chars,int);
	l = (size_t *)(1+(char *)(l+1));
    }
    va_end(chars);
    t = _t_new(stxx,SEMTREX_VALUE_LITERAL,svP,svl);

    free(svP);
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
    T *t,*s = _t_new_root(ASCII_CHARS);
    while(*stx) {
	_t_newi(s,ASCII_CHAR,*stx);
	stx++;
    }

    /////////////////////////////////////////////////////
    // build the token stream out of an ascii stream
    // EXPECTATION
    // /(TEST_STR_SYMBOL/(sy1/(sy11/sy111)),sy2,sy3)" but was "/{STX_TOKENS:(ASCII_CHARS/({STX_SL:ASCII_CHAR='/'})|(({STX_OP:ASCII_CHAR='('})|(({STX_CP:ASCII_CHAR=')'})|(({STX_PLUS:ASCII_CHAR='+'})|(({STX_COMMA:ASCII_CHAR=','})|((ASCII_CHAR='!',{STX_EXCEPT:[a-zA-Z0-9_]+})|(({STX_CG:ASCII_CHAR='}'})|(({STX_STAR:ASCII_CHAR='*'})|(({STX_LABEL:[a-zA-Z0-9_]+})|(ASCII_CHAR='{',{STX_OG:[a-zA-Z0-9_]+},ASCII_CHAR=':')))))))))+)}
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
    sq = _t_newr(o,SEMTREX_SEQUENCE);
    __stxcv(sq,'!');
    t = _t_news(sq,SEMTREX_GROUP,STX_EXCEPT);
    _stxl(t);

    o = _t_newr(o,SEMTREX_OR);
    t = _t_news(o,SEMTREX_GROUP,STX_CG);
    __stxcv(t,'}');
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
    t = _t_news(sq,SEMTREX_GROUP,STX_VAL_I);
    t = _t_newr(t,SEMTREX_ONE_OR_MORE);
    _stxcs(t,"0123456789");

    o = _t_newr(o,SEMTREX_OR);
    t = _t_news(o,SEMTREX_GROUP,STX_LABEL);
    _stxl(t);

    //  o = _t_newr(o,SEMTREX_OR);
    sq = _t_newr(o,SEMTREX_SEQUENCE);
    __stxcv(sq,'{');
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
	for(i=m;i>1;i--) {
	    T *c = _t_child(results,i);
	    T *sn = _t_child(c,1);
	    Symbol ts = *(Symbol *)_t_surface(sn);
	    if (semeq(ts,STX_VAL_S) || semeq(ts,STX_LABEL) || semeq(ts,STX_OG) || semeq(ts,STX_EXCEPT)  || semeq(ts,STX_EQ)){
		int sibs = *(int *)_t_surface(_t_child(c,3));
		int *path = (int *)_t_surface(_t_child(c,2));
		int j,d = _t_path_depth(path);
		for(j=0;j<sibs;j++) {
		    buf[j] = *(char *)_t_surface(_t_get(s,path));
		    path[d-1]++;
		}
		buf[j]=0;
		_t_new(tokens,ts,buf,j+1);
	    }
	    else if (semeq(ts,STX_VAL_C)) {
		int *path = (int *)_t_surface(_t_child(c,2));
		int c = *(int *)_t_surface(_t_get(s,path));
		_t_newi(tokens,ts,c);
	    }
	    else if (semeq(ts,STX_VAL_I)) {
		int sibs = *(int *)_t_surface(_t_child(c,3));
		int *path = (int *)_t_surface(_t_child(c,2));
		int j,d = _t_path_depth(path);
		for(j=0;j<sibs;j++) {
		    buf[j] = *(char *)_t_surface(_t_get(s,path));
		    path[d-1]++;
		}
		buf[j]=0;
		_t_newi(tokens,ts,atoi(buf));
	    }
	    else
		_t_newi(tokens,ts,0);
	}
	_t_free(results);

	dump_tokens("TOKENS:");
	T *sxx,*sq;

	/////////////////////////////////////////////////////
	// convert STX_EQ to SEMTREX_VALUE_LITERALS
	// EXPECTATION
	// /%{SEMTREX_VALUE_LITERAL:STX_EQ,STX_VAL_I|STX_VAL_S|STX_VAL_C}
	sxx = _t_new_root(SEMTREX_WALK);
	g = _t_news(sxx,SEMTREX_GROUP,SEMTREX_VALUE_LITERAL);
	sq = _t_newr(g,SEMTREX_SEQUENCE);
	_t_news(sq,SEMTREX_SYMBOL_LITERAL,STX_EQ);
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
	    T *v = _t_next_sibling(t);
	    T *p = _t_parent(v);
	    _t_detach_by_ptr(p,v);

	    size_t l = _t_size(v);  // value size

	    char *symbol_name = (char *)_t_surface(t);
	    Svalue *sv = malloc(sizeof(Svalue)+l);
	    sv->symbol = get_symbol(symbol_name,d);
	    sv->count = 1;
	    memcpy(&sv->value,_t_surface(v),l);

	    __t_morph(t,SEMTREX_VALUE_LITERAL,sv,sizeof(Svalue)+l,1);
	    _t_free(v);
	    free(sv);

	    _t_free(results);
	}
	_t_free(sxx);

	dump_tokens("TOKENS_AFTER_VALUE_LITERAL:");

	/////////////////////////////////////////////////////
	// replace paren groups with STX_SIBS list
	// EXPECTATION
	// /STX_TOKENS/.*,{STX_OP:STX_OP,{STX_SIBS:!STX_CP+},STX_CP}
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
	// /%,{STX_OG:STX_OG,{SEMTREX_GROUP:!STX_CG+},STX_CG}
	sxx = _t_new_root(SEMTREX_WALK);
	g = _t_news(sxx,SEMTREX_GROUP,STX_OG);
	sq = _t_newr(g,SEMTREX_SEQUENCE);
	_t_news(sq,SEMTREX_SYMBOL_LITERAL,STX_OG);
	gg = _t_news(sq,SEMTREX_GROUP,SEMTREX_GROUP);
	any = _t_newr(gg,SEMTREX_ONE_OR_MORE);
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
	// /*{STX_POSTFIX:.,STX_PLUS|STX_STAR|STX_Q)
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
	// /%{SEMTREX_WALK:STX_WALK,.}
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
	// /%{SEMTREX_NOT:STX_NOT,.}
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
	// /%.*,{STX_CHILD:STX_LABEL,STX_SL,!STX_SL}
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
	// /*{SEMTREX_SEQUENCE:(!STX_COMMA,STX_COMMA)+,!STX_COMMA}  ->  SEMTREX_SEQUENCE
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
	// /%{SEMTREX_OR:!STX_OR,STX_OR,!STX_OR}
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
	// /%STX_LABEL|STX_EXCEPT
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
	// /%STX_SIBS
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
