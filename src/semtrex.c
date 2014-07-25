#include "semtrex.h"


// based on: http://swtch.com/~rsc/regexp/regexp1.HTML

SState matchstate = {StateMatch}; /* only one instance of the match state*/

SState * __s_makeFA(Tnode *t,SState *src) {
    SState *s = malloc(sizeof(SState));
    SState *prev,*next;
    if (src != 0) {
	src->out = s;
    }
    s->out1 = NULL;
    int c = _t_children(t);
    switch(_t_symbol(t)) {
    case SEMTREX_SYMBOL_LITERAL:
	s->type = StateSymbol;
	s->symbol = *(Symbol *)_t_surface(t);
	s->transition = c > 0 ? TransitionDown : TransitionNextChild;
	prev = s;
	for(int i=1;i<=c;i++) {
	    next = __s_makeFA(_t_child(t,i),prev);
	    if (i > 1) prev->out = next;
	    prev = next;
	}
	break;
    default:
	raise_error0("SEMTREX SYMBOL UNIMPLEMENTED ");
    }
    if (_t_parent(t) != 0) {
	prev->transition = TransitionUp;
    }
    else {
	prev->out = &matchstate;
    }
    return s;
}

SState * _s_makeFA(Tnode *t) {
    return __s_makeFA(t,0);
}

void _s_freeFA(SState *s) {
}

/* typedef struct List List; */
/* struct List */
/* { */
/* 	SState **s; */
/* 	int n; */
/* }; */
/* List l1, l2; */
/* static int listid; */

/* void addstate(List*, SState*); */
/* void step(List*, int, List*); */

/* /\* Compute initial state list *\/ */
/* List *startlist(SState *start, List *l) */
/* { */
/*     l->n = 0; */
/*     listid++; */
/*     addstate(l, start); */
/*     return l; */
/* } */

/* /\* Check whether state list contains a match. *\/ */
/* int ismatch(List *l) */
/* { */
/*     int i; */

/*     for(i=0; i<l->n; i++) */
/* 	if(l->s[i] == &matchstate) */
/* 	    return 1; */
/*     return 0; */
/* } */

/* /\* Add s to l, following unlabeled arrows. *\/ */
/* void addstate(List *l, State *s) */
/* { */
/*     if(s == NULL || s->lastlist == listid) */
/* 	return; */
/*     s->lastlist = listid; */
/*     if(s->c == Split){ */
/* 	/\* follow unlabeled arrows *\/ */
/* 	addstate(l, s->out); */
/* 	addstate(l, s->out1); */
/* 	return; */
/*     } */
/*     l->s[l->n++] = s; */
/* } */

/* /\* */
/*  * Step the NFA from the states in clist */
/*  * past the character c, */
/*  * to create next NFA state set nlist. */
/*  *\/ */
/* void step(List *clist, int c, List *nlist) */
/* { */
/*     int i; */
/*     State *s; */

/*     listid++; */
/*     nlist->n = 0; */
/*     for(i=0; i<clist->n; i++){ */
/* 	s = clist->s[i]; */
/* 	if(s->c == c) */
/* 	    addstate(nlist, s->out); */
/*     } */
/* } */


int _t_match(Tnode *semtrex,Tnode *t) {
    return 0;

}
