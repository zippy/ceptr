#include "semtrex.h"


// based on: http://swtch.com/~rsc/regexp/regexp1.html

SState matchstate = {StateMatch}; /* only one instance of the match state*/

char * __s_makeFA(Tnode *t,SState **in, SState **out,int level,int *statesP) {
    SState *s,*i,*o,*prev_o;
    char *err;

    int c = _t_children(t);
    switch(_t_symbol(t)) {
    case SEMTREX_SYMBOL_LITERAL:
	if (c > 1) return "Literal must have 0 or 1 children";
	s = malloc(sizeof(SState));
	(*statesP)++;
	s->out1 = NULL;
	*in = s;
	s->type = StateSymbol;
	s->symbol = *(Symbol *)_t_surface(t);
	if (c > 0) {
	    s->transition = TransitionDown;
	    err = __s_makeFA(_t_child(t,1),&i,&o,level-1,statesP);
	    s->out = i;
	    *out = o;
	    o->out = &matchstate;
	}
	else {
	    *out = s;
	    s->transition = level;
	    s->out = &matchstate;
	}
	break;
    case SEMTREX_SEQUENCE:
	if (c == 0) return "Sequence must have children";
	prev_o = 0;
	for(int x=1;x<=c;x++) {
	    err = __s_makeFA(_t_child(t,x),&i,&o,level,statesP);
	    if (o->transition < 0) o->transition += -level;
	    if (err) return err;
	    if (!prev_o) {*in = i;}
	    else {prev_o->out = i;}
	    prev_o = o;
	}
	*out = o;
	break;
    default:
	return "SEMTREX SYMBOL UNIMPLEMENTED";
    }
    return 0;
}

SState * _s_makeFA(Tnode *t,int *statesP) {
    SState *in,*out;
    char *err = __s_makeFA(t,&in,&out,0,statesP);
    if (err != 0) {raise_error0(err);}
    return in;
}

void _s_freeFA(SState *s) {
}

int __t_match(SState *s,Tnode *t) {
    printf("tm: s:%d t:%d\n",s->symbol,_t_symbol(t));
    switch(s->type) {
    case StateSymbol:
	if (s->symbol != _t_symbol(t)) return 0;
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
    case StateMatch:
	return 1;
	break;
    }
    raise_error("unimplemented!%d",s->type);
}

int _t_match(Tnode *semtrex,Tnode *t) {
    int states;
    SState *fa = _s_makeFA(semtrex,&states);
    int m = __t_match(fa,t);
    _s_freeFA(fa);
    return m;
}
//int _t_match(Tnode *semtrex,Tnode *t) {return 0;}
