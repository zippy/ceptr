#ifndef _PARSE_H
#define _PARSE_H

#include "flow.h"
#include "def.h"
#include "word.h"

enum {P_START,P_IGNOREWS,P_READ_NOUN,P_READ_VALUE,P_CHILDREN,P_READ_STR,P_READ_ESCAPE};

T *_t_parse(char *s) {
    char word[255];
    char val[255];
    int i,parse_result,n;
    T *t = 0,*r,*d;
    int state = P_IGNOREWS;
    int next = P_START;
    char c;
    parseValFn pf;

    while (c = *s++) {
	switch(state) {
	case P_IGNOREWS:
	    if (isspace(c)) break;
	    state = next;
	    s--;
	    break;
	case P_START:
	    if (c == '(') {
		i = 0;
		state = P_IGNOREWS;
		next = P_READ_NOUN;
		break;
	    }
	    if (!isspace(c)) {raise_error("expecting '(' got: %c\n",c);}
	    break;
	case P_READ_NOUN:
	    if (isalnum(c) || c == '_') {
		word[i++] = c;
		break;
	    }
	    word[i] = 0;
	    i = 0;

	    if (__d_parse_noun(word,&n)) {
		int r = 1;
	        if (G_sys_words) {
		    int c = _t_children(G_sys_words);
		    for(int l = 1;l<=c;l++) {
			if(!strcicmp(word,_w_get_label(G_sys_words,l))) {
			    T *d = _w_get_def(G_sys_words,l);
			    n = *(int *)_t_surface(d);
			    r = 0;break;
			}
		    }
		}
		if (r) {raise_error("unknown noun %s\n",word);}
	    }



	    t = _t_newi(t,n,0);
	    if (c == ':') {
		state = P_READ_VALUE;
		break;
	    }
	    else {
		s--;
		state = P_IGNOREWS;
		next = P_CHILDREN;
	    }
	    break;
	case P_CHILDREN:
	    if (c == '(') {
		s--;
		state = P_IGNOREWS;
		next = P_START;
		break;
	    } else if (c==')') {
		if (t == 0) {raise_error0("parens didn't balance: too many\n");}
		r = t;
		t = _t_parent(t);
		state = P_IGNOREWS;
		next = P_CHILDREN;
		break;
	    }
	    else {
		raise_error("expecting open or close paren, got: %c\n",c);
	    }
	    break;
	case P_READ_ESCAPE:
	    word[i++] = c;
	    state = P_READ_STR;
	    break;
	case P_READ_STR:
	    if (c == '\\') {
		state = P_READ_ESCAPE;
		break;
	    }
	    if (c == '"') {
		state = P_READ_VALUE;
		break;
	    }
	    else word[i++] = c;
	    break;
	case P_READ_VALUE:
	    if (i == 0 && c == '"') {
		state = P_READ_STR;
		break;
	    }
	    if (isalnum(c) || c == '_') {
		word[i++] = c;
		break;
	    }
	    word[i] = 0;

	    d = _d_get_def(t);
	    pf = *(parseValFn*)_t_get_child_surface(d,DEF_PARSE_CHILD);
	    if (pf == 0) {
		t->surface = 0;
	    }
	    else {
		parse_result = (*pf)(word,i,&t->surface);

		if (parse_result > 0) {
		    t->flags |= TFLAG_ALLOCATED;
		}
		if (parse_result < 0) {
		    raise_error2("unparseable value %s for noun %d\n",word,_t_noun(t));
		}
	    }

	    i = 0;
	    state = P_IGNOREWS;
	    next = P_CHILDREN;
	    s--;
	    break;
	}
    }
    if (state == P_READ_STR || state == P_READ_ESCAPE) {raise_error0("incomplete string\n");}
    if (t!=0) {raise_error0("parens didn't balance: too few\n");}
    return r;
}


#endif
