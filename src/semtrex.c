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

char * __s_makeFA(Tnode *t,SState **in,Ptrlist **out,int level,int *statesP) {
    SState *s,*i,*last;
    Ptrlist *o,*o1;
    char *err;

    int c = _t_children(t);
    Symbol sym = _t_symbol(t);
    switch(sym) {
    case SEMTREX_SYMBOL_LITERAL:
    case SEMTREX_SYMBOL_ANY:
	if (c > 1) return "Literal must have 0 or 1 children";
	s = malloc(sizeof(SState));
	(*statesP)++;
	s->out1 = NULL;
	*in = s;
	s->type = (sym == SEMTREX_SYMBOL_LITERAL) ? StateSymbol :StateAny;
	s->symbol = *(Symbol *)_t_surface(t);
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
	for(int x=c;x>=1;x--) {
	    err = __s_makeFA(_t_child(t,x),&i,&o,level,statesP);
	    if (err) return err;

	    // if (o1->transition < 0) o1->transition += -level;
	    if (last) patch(o,last,level);
	    else *out = o;
	    last = i;
	}
	*in = i;
	break;
    case SEMTREX_OR:
	if (c != 2) return "Or must have 2 children";
	s = malloc(sizeof(SState));
	(*statesP)++;
	*in = s;
	s->type = StateSplit;
	err = __s_makeFA(_t_child(t,1),&i,&o,level,statesP);
	if (err) return err;
	s->out = i;
	err = __s_makeFA(_t_child(t,2),&i,&o1,level,statesP);
	if (err) return err;
	s->out1 = i;
	*out = append(o,o1);
	break;
    case SEMTREX_STAR:
	if (c != 1) return "Star must have 1 child";
	s = malloc(sizeof(SState));
	(*statesP)++;
	*in = s;
	s->type = StateSplit;
	err = __s_makeFA(_t_child(t,1),&i,&o,level,statesP);
	if (err) return err;
	s->out = i;
	patch(o,s,level);
	*out = list1(&s->out1);
	break;
    case SEMTREX_PLUS:
	if (c != 1) return "Plus must have 1 child";
	s = malloc(sizeof(SState));
	(*statesP)++;
	s->type = StateSplit;
	err = __s_makeFA(_t_child(t,1),&i,&o,level,statesP);
	if (err) return err;
	*in = i;
	s->out = i;
	patch(o,s,level);
	*out = list1(&s->out1);
	break;
    case SEMTREX_QUESTION:
	if (c != 1) return "Question must have 1 child";
	s = malloc(sizeof(SState));
	(*statesP)++;
	*in = s;
	s->type = StateSplit;
	err = __s_makeFA(_t_child(t,1),&i,&o,level,statesP);
	if (err) return err;
	s->out = i;
	*out = append(o,list1(&s->out1));
	break;
    default:
	return "Unknown SEMTREX SYMBOL";
    }
    return 0;
}
void _s_dump(SState *s);
SState * _s_makeFA(Tnode *t,int *statesP) {
    SState *in;
    Ptrlist *o;
    char *err = __s_makeFA(t,&in,&o,0,statesP);
    if (err != 0) {raise_error0(err);}
    patch(o,&matchstate,0);
    _s_dump(in);
    return in;
}

void _s_freeFA(SState *s) {
}

int __t_match(SState *s,Tnode *t) {
    printf("tm: s:%d t:%d\n",s->symbol,t ? _t_symbol(t) : -1);
    switch(s->type) {
    case StateSymbol:
	if (!t || (s->symbol != _t_symbol(t))) return 0;
    case StateAny:
	if (!t) return 0;
	if (s->out == &matchstate) return 1;
	switch(s->transition) {
	case TransitionNextChild:
	    return __t_match(s->out,_t_next_sibling(t));
	    break;
	case TransitionDown:
	    return __t_match(s->out,_t_child(t,1));
	    break;
	default:
	    for(int i=s->transition;i<0;i++) {
		t = _t_parent(t);
	    }
	    t = _t_next_sibling(t);
	    return __t_match(s->out,t);
	    break;
	}
	break;
    case StateSplit:
	if (__t_match(s->out,t)) return 1;
	else return __t_match(s->out1,t);
	break;
    case StateMatch:
	return 1;
	break;
    }
    raise_error("unimplemented state type: %d",s->type);
}

int _t_match(Tnode *semtrex,Tnode *t) {
    int states;
    SState *fa = _s_makeFA(semtrex,&states);
    int m = __t_match(fa,t);
    _s_freeFA(fa);
    return m;
}
