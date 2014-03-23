#ifndef _PARSE_H
#define _PARSE_H

#include "flow.h"

enum {P_START,P_IGNOREWS,P_READ_NOUN,P_READ_VALUE,P_CHILDREN};

int strcicmp(char const *a, char const *b)
{
    for (;; a++, b++) {
        int d = tolower(*a) - tolower(*b);
        if (d != 0 || !*a)
            return d;
    }
}
int __t_parse_noun(char *n) {
    if (!strcicmp(n,"flow"))
	return FLOW_NOUN;
    if (!strcicmp(n,"boolean"))
	return BOOLEAN_NOUN;
    if (!strcicmp(n,"integer"))
	return INTEGER_NOUN;
    raise_error("unknown noun %s\n",n);
}

int __t_parse_value(Tnode *t,char *v) {
    switch(_t_noun(t)) {
    case FLOW_NOUN:
	if (!strcicmp(v,"if"))
	    return F_IF;
	break;
    case BOOLEAN_NOUN:
	if (!strcicmp(v,"true"))
	    return TRUE_VALUE;
	if (!strcicmp(v,"false"))
	    return FALSE_VALUE;
	break;
    case INTEGER_NOUN:
	return atoi(v);
    }
    raise_error2("unparseable value %s for noun %d\n",v,_t_noun(t));
}

Tnode *_t_parse(char *s) {
    char word[255];
    char val[255];
    int i;
    Tnode *t = 0,*r;
    int state = P_IGNOREWS;
    int next = P_START;
    char c;
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
	    t = _t_newi(t,__t_parse_noun(word),0);
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
		raise_error("unexpected token: %c\n",c);
	    }
	    break;
	case P_READ_VALUE:
	    if (isalnum(c) || c == '_') {
		word[i++] = c;
		break;
	    }
	    word[i] = 0;
	    i = 0;
	    *(int *)&t->surface = __t_parse_value(t,word);
	    state = P_IGNOREWS;
	    next = P_CHILDREN;
	    s--;
	    break;
	}
    }
    if (t!=0) {raise_error0("parens didn't balance: too few\n");}
    return r;
}


#endif
