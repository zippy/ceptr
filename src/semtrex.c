#include "semtrex.h"

// based on: http://swtch.com/~rsc/regexp/regexp1.html

SState matchstate = {StateMatch}; /* only one instance of the match state*/

/*
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

/* Create singleton list containing just outp. */
Ptrlist*
list1(SState **outp)
{
    Ptrlist *l;

    l = (Ptrlist*)outp;
    l->next = NULL;
    return l;
}

/* Patch the list of states at out to point to start. */
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

/* Join the two lists l1 and l2, returning the combination. */
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

// utility routine to initialize a state struct
SState *state(StateType type,int *statesP) {
    SState *s = malloc(sizeof(SState));
    s->out = NULL;
    s->out1 = NULL;
    s->type = type;
    s->_did = 0;
    (*statesP)++;
    return s;
}

// Given a Semtrex tree, build a partial FSA (returned via in as a pointer to the starting state, a list of output states, and a count of the total number of states created).
char * __s_makeFA(Tnode *t,SState **in,Ptrlist **out,int level,int *statesP) {
    SState *s,*i,*last;
    Ptrlist *o,*o1;
    char *err;
    int state_type = -1;
    int x;

    int c = _t_children(t);
    Symbol sym = _t_symbol(t);
    switch(sym) {
    case SEMTREX_VALUE_LITERAL:
	state_type = StateValue;
    case SEMTREX_SYMBOL_LITERAL:
	if (state_type == -1) state_type = StateSymbol;
    case SEMTREX_SYMBOL_ANY:
	if (state_type == -1) state_type = StateAny;
	if (c > 1) return "Literal must have 0 or 1 children";
	s = state(state_type,statesP);
	if (state_type == StateValue) {
	    // copy the value from the semtrex into the state
	    Svalue *sv = (Svalue *)_t_surface(t);
	    s->data.value.symbol = sv->symbol;
	    s->data.value.length = sv->length;
	    s->data.value.value = malloc(sv->length);
	    memcpy(s->data.value.value,&sv->value,sv->length);
	}
	else if (state_type == StateSymbol) {
	    s->data.symbol = *(Symbol *)_t_surface(t);
	}
	*in = s;
	if (c > 0) {
	    s->transition = TransitionDown;
	    err = __s_makeFA(_t_child(t,1),&i,&o,level-1,statesP);
	    if (err) return err;
	    s->out = i;
	    *out = o;
	}
	else {
	    s->transition = level;
	    *out = list1(&s->out);
	}
	break;
    case SEMTREX_SEQUENCE:
	if (c == 0) return "Sequence must have children";
	last = 0;
	for(x=c;x>=1;x--) {
	err = __s_makeFA(_t_child(t,x),&i,&o,level,statesP);
	if (err) return err;

	// if (o1->transition < 0) o1->transition += -level;
	if (last) patch(o,last,level);
	else *out = o;
	last = i;
    *in = i;
	}
	break;
    case SEMTREX_OR:
	if (c != 2) return "Or must have 2 children";
	s = state(StateSplit,statesP);
	*in = s;
	err = __s_makeFA(_t_child(t,1),&i,&o,level,statesP);
	if (err) return err;
	s->out = i;
	err = __s_makeFA(_t_child(t,2),&i,&o1,level,statesP);
	if (err) return err;
	s->out1 = i;
	*out = append(o,o1);
	break;
    case SEMTREX_ZERO_OR_MORE:
	if (c != 1) return "Star must have 1 child";
	s = state(StateSplit,statesP);
	*in = s;
	err = __s_makeFA(_t_child(t,1),&i,&o,level,statesP);
	if (err) return err;
	s->out = i;
	patch(o,s,level);
	*out = list1(&s->out1);
	break;
    case SEMTREX_ONE_OR_MORE:
	if (c != 1) return "Plus must have 1 child";
	s = state(StateSplit,statesP);
	err = __s_makeFA(_t_child(t,1),&i,&o,level,statesP);
	if (err) return err;
	*in = i;
	s->out = i;
	patch(o,s,level);
	*out = list1(&s->out1);
	break;
    case SEMTREX_ZERO_OR_ONE:
	if (c != 1) return "Question must have 1 child";
	s = state(StateSplit,statesP);
	*in = s;
	err = __s_makeFA(_t_child(t,1),&i,&o,level,statesP);
	if (err) return err;
	s->out = i;
	*out = append(o,list1(&s->out1));
	break;
    case SEMTREX_GROUP:
	if (c != 1) return "Group must have 1 child";
	s = state(StateGroup,statesP);
	*in = s;
	x = *(int *)_t_surface(t);
	s->data.group.id = x;
	s->data.group.type = GroupOpen;
	err = __s_makeFA(_t_child(t,1),&i,&o,level,statesP);
	if (err) return err;
	s->out = i;
	s = state(StateGroup,statesP);
	patch(o,s,level);
	s->data.group.id = x;
	s->data.group.type = GroupClose;
	*out = list1(&s->out);
	break;
    default:
	return "Unknown SEMTREX SYMBOL";
    }
    return 0;
}

void _s_dump(SState *s);

// wrapper for building the finite state automata recursively and patching it to the final match state
SState * _s_makeFA(Tnode *t,int *statesP) {
    SState *in;
    Ptrlist *o;
    char *err = __s_makeFA(t,&in,&o,0,statesP);
    if (err != 0) {raise_error0(err);}
    patch(o,&matchstate,0);
    //        _s_dump(in);
    return in;
}

static int free_id = 0;

// walk through the FSA clearing pointers to marked states
int __s_freeFA(SState *s,int id) {
    if ((s->_did != id) && (s != &matchstate)) {
	s->_did = id;
	if (s->out) if (__s_freeFA(s->out,id)) s->out = 0;
	if (s->out1) if (__s_freeFA(s->out1,id)) s->out1 = 0;
	return 0;
    }
    return 1;
}

// walk through again freeing the states
__s_freeFA2(SState *s) {
    if (s->out) __s_freeFA2(s->out);
    if (s->out1) __s_freeFA2(s->out1);
    if (s->type == StateValue) {
	free(s->data.value.value);
    }
    free(s);
}

// walks through a state diagram freeing states
// and malloced values of StateValue states
void _s_freeFA(SState *s) {
    __s_freeFA(s,++free_id);
    __s_freeFA2(s);
}

// Walk the FSA in s using a recursive backtracing algorithm to match the tree in t.  Returns matched portions in a match results tree if one is provided in r.
int __t_match(SState *s,Tnode *t,Tnode *r) {
    char *p1,*p2;
    int i;
    //printf("tm: s:%d t:%d\n",s->data.symbol,t ? _t_symbol(t) : -1);
    switch(s->type) {
    case StateValue:
	if (!t) return 0;
	else {
	    size_t i;
	    if (s->data.value.length != _t_size(t));
	    p1 = s->data.value.value;
	    p2 = _t_surface(t);
	    for(i=s->data.value.length;i>0;i--) {
		if (*p1++ != *p2++) return 0;
	    }
	}
	// no break to fall through and check symbol
    case StateSymbol:
	if (!t || (s->data.symbol != _t_symbol(t))) return 0;
	// no break to fall through and handle transition and recursive calls
    case StateAny:
	if (!t) return 0;
	if (s->out == &matchstate) return 1;
	switch(s->transition) {
	case TransitionNextChild:
	    return __t_match(s->out,_t_next_sibling(t),r);
	    break;
	case TransitionDown:
	    return __t_match(s->out,_t_child(t,1),r);
	    break;
	default:
	    for(i=s->transition;i<0;i++) {
		t = _t_parent(t);
	    }
	    t = _t_next_sibling(t);
	    return __t_match(s->out,t,r);
	    break;
	}
	break;
    case StateSplit:
	if (__t_match(s->out,t,r)) return 1;
	else return __t_match(s->out1,t,r);
	break;
    case StateGroup:
	if (!r)
	    // if we aren't collecting up match results simply follow groups through
	    return __t_match(s->out,t,r);
	else {
	    int match_id = s->data.group.id;
	    int matched;
	    if (s->data.group.type == GroupOpen) {
		if (!t) return 0;
		// add on the semtrex match nodes and path to list of matches.
		Tnode *m = _t_newi(r,SEMTREX_MATCH,match_id);
		int *p = _t_get_path(t);
		Tnode *pp = _t_new(m,TREE_PATH,p,sizeof(int)*(_t_path_depth(p)+1));
		free(p);
		matched = __t_match(s->out,t,r);
		if (!matched) {
		    _t_remove(r,m);
		    _t_free(m);
		}
		return matched;
	    }
	    else {
		// make sure that what follows the group also matches
		int matched = __t_match(s->out,t,r);
		if (matched) {
		    // use the match_id find the Match item that this is a CloseGroup of this item
		    int* p_start;
		    int* p_end;
		    int d;
		    Tnode *m =0;
		    int i;
		    for(i=1;i<=_t_children(r);i++) {
			Tnode *n = _t_child(r,i);
			if (*(int *)_t_surface(n) == match_id) {
			    m = n;
			    break;
			}
		    }

		    if (m == 0) {
			raise_error("couldn't find match for %d",match_id);
		    }

		    if (!t) i=1;
		    else {
			p_start = (int *)_t_surface(_t_child(m,1));
			p_end = _t_get_path(t);
			d = _t_path_depth(p_start);
			d--;
			i = p_end[d]-p_start[d];
			free(p_end);
		    }
		    _t_newi(m,SEMTREX_MATCH_SIBLINGS_COUNT,i);
		}
		return matched;
	    }
	}
	break;
    case StateMatch:
	return 1;
	break;
    }
    raise_error("unimplemented state type: %d",s->type);
    return 0;
}

// semtrex matching where you care about the matched results
int _t_matchr(Tnode *semtrex,Tnode *t,Tnode **rP) {
    int states;
    int m;
    SState *fa = _s_makeFA(semtrex,&states);
    Tnode *r = 0;
    if (rP) {
	r = _t_new_root(SEMTREX_MATCH_RESULTS);
    }
    m = __t_match(fa,t,r);
    _s_freeFA(fa);
    if (rP) {
	if (!m) {_t_free(r);*rP = NULL;}
	else *rP = r;
    }
    return m;
}

// semtrex matching where you just care about whether a match exists
int _t_match(Tnode *semtrex,Tnode *t) {
    return _t_matchr(semtrex,t,NULL);
}

// return the semtrex match that matches the symbol
Tnode *_t_get_match(Tnode *result,Symbol group)
{
    int i,c=_t_children(result);
    Tnode *t;
    for (i=1;i<=c;i++) {
	if (*(int *)_t_surface(t = _t_child(result,i)) == group)
	    return t;
    }
    return 0;
}
