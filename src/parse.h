#ifndef _PARSE_H
#define _PARSE_H

#include "flow.h"

enum {P_START,P_IGNOREWS,P_READ_NOUN,P_READ_VALUE,P_CHILDREN,P_READ_STR,P_READ_ESCAPE};

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
    if (!strcicmp(n,"cstring"))
	return CSTRING_NOUN;
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
	break;
    case CSTRING_NOUN:
	return 0;
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
	    if (t->noun == CSTRING_NOUN) {
		t->surface = malloc(i);
		memcpy(t->surface,word,i);
		t->flags |= TFLAG_ALLOCATED;
	    }
	    else *(int *)&t->surface = __t_parse_value(t,word);

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
