#ifndef _CEPTR_DEF_H
#define _CEPTR_DEF_H
#include "tree.h"

enum Symbols {
    CSPEC = -1, CSPEC_NOUN = -2, XADDR_NOUN = -3,
    CSTRING_NOUN = -4, PATTERN_SPEC_DATA_NOUN = -5,
    NOUN_NOUN = -6
};

enum {RUNTREE_NOUN=-199,INSTRUCTION_NOUN=-200,INTEGER_NOUN=-201};
enum {FLOW_NOUN = -31410,PATH_NOUN,CONTEXT_TREE_NOUN,BOOLEAN_NOUN,FLOW_STATE_NOUN};

enum {F_IF}; //flows

char dump_buf[1000];
#define DEFS_ARRAY_NOUN -799
#define META_NOUN -800
#define DEF_NOUN -888
#define DEF_DUMP_FUNC_NOUN -801
#define DEF_PARSEVAL_FUNC_NOUN -802

enum {FALSE_VALUE = 0,TRUE_VALUE = 1};

int strcicmp(char const *a, char const *b);

Tnode *G_sys_defs;

Tnode *__d_get_def(Symbol noun) {
    if (noun < 0) {
	for(int i=1;i<=_t_children(G_sys_defs);i++){
	    Tnode *d = _t_get_child(G_sys_defs,i);
	    if (noun == *(int *)_t_surface(d))
		return d;
	}
    }
    return NULL;
}

Tnode *_d_get_def(Tnode *t) {
    Symbol noun = _t_noun(t);
    return __d_get_def(noun);
}

typedef char * (*dumpFn)(void *);
typedef int (*parseValFn)(char *,int,void *);

void __d_dump_surface(Tnode *t,int level) {
    __lspc(level);
    Tnode *d = _d_get_def(t);
    if (d == NULL) {
	printf("<noun: %d> sfc: %p; children: %d\n",_t_noun(t),t->surface,_t_children(t));
    }
    else {
	char *label = (char *)_t_get_child_surface(d,1);
	printf("%s_NOUN:",label);
	dumpFn f = *(dumpFn *) _t_get_child_surface(d,2);
	void *s = _t_surface(t);
	printf(" ");
	if (f) {
	    char *text = (*f)(s);
	    if (text)
		printf("%s\n",text);
	    else
		printf("\n");
	}
	else printf("%p\n",s);
    }
    for(int i=1;i<=_t_children(t);i++) __d_dump_surface(_t_get_child(t,i),level+1);
}

void _d_dump_surface(Tnode *surface) {
    __d_dump_surface(surface,0);
}

char *_dump_cstring(void *surface) {
    sprintf(dump_buf,"\"%s\"",(char*)surface);
    return dump_buf;
}

char *_dump_xaddr(void *surface) {
    struct {int key;int noun;} *x = surface;
    sprintf(dump_buf,"key:%d,noun:%d",x->key,x->noun);
    return dump_buf;
}

char *_dump_def(void *surface) {
    Symbol noun = *(Symbol *)surface;
    Tnode *d = __d_get_def(noun);
    if (d)
	return (char *)_t_get_child_surface(d,1);
    else {
	sprintf(dump_buf,"<%d>",noun);
	return dump_buf;
    }
   return 0;
}

int _d_parse_int(char *v,int l,void *s) {
    *(int *)s = atoi(v);
    return 0;
}

int _d_parse_cstring(char *v,int l,void *s) {
    char *c = malloc(l+1);
    memcpy(c,v,l);
    c[l] = 0;
    *(char **)s = c;
    return 1;
}

int _d_parse_bool(char *v,int l,void *s) {
    if (!strcicmp(v,"true")) {
	*(int *)s =  TRUE_VALUE;
	return 0;
    }
    else if (!strcicmp(v,"false")) {
	*(int *)s =  FALSE_VALUE;
	return 0;
    }
    return -1;
}

int _d_parse_flow(char *v,int l,void *s) {
    if (!strcicmp(v,"if")) {
	*(int *)s = F_IF;
	return 0;
    }
    return -1;
}


Tnode *_d_def(Tnode *t,char *label,Symbol noun,dumpFn df,parseValFn pf) {
    Tnode *d;
    d = _t_newi(t,DEF_NOUN,noun);
    _t_new(d,CSTRING_NOUN,label,strlen(label)+1);
    _t_newi(d,DEF_DUMP_FUNC_NOUN,(long)df);
    _t_newi(d,DEF_PARSEVAL_FUNC_NOUN,(long)pf);
    return _t_newi(d,META_NOUN,noun);
}

Tnode *_d_sys_def(char *label,Symbol noun,dumpFn df,parseValFn pf) {
    return _d_def(G_sys_defs,label,noun,df,pf);
}

void sys_defs_init() {
    G_sys_defs = _t_new_root(DEFS_ARRAY_NOUN);

    Tnode *d;
    _d_sys_def("DEF",DEF_NOUN,_dump_def,0);

    _d_sys_def("META",META_NOUN,_dump_def,_d_parse_int);

    _d_sys_def("DEF_DUMP_FUNC",DEF_DUMP_FUNC_NOUN,0,0);

    _d_sys_def("DEF_PARSEVAL_FUNC",DEF_PARSEVAL_FUNC_NOUN,0,0);

    _d_sys_def("CSTRING",CSTRING_NOUN,_dump_cstring,_d_parse_cstring);

    _d_sys_def("XADDR",XADDR_NOUN,_dump_xaddr,0);

    d = _d_sys_def("NOUN",NOUN_NOUN,0,0);
    _t_newi(d,META_NOUN,XADDR_NOUN);
    _t_newi(d,META_NOUN,CSTRING_NOUN);

    _d_sys_def("BOOLEAN",BOOLEAN_NOUN,0,_d_parse_bool);
    _d_sys_def("INTEGER",INTEGER_NOUN,0,_d_parse_int);
    _d_sys_def("FLOW",FLOW_NOUN,0,_d_parse_flow);

}

void sys_defs_free() {
    _t_free(G_sys_defs);
}

#endif
