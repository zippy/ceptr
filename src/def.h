#ifndef _CEPTR_DEF_H
#define _CEPTR_DEF_H
#include "tree.h"

enum Symbols {
CSPEC = -1, CSPEC_NOUN = -2, XADDR_NOUN = -3,
    CSTRING_NOUN = -4, PATTERN_SPEC_DATA_NOUN = -5
};


char dump_buf[1000];
#define SYS_DEFS_NOUN -799
#define META_NOUN -800
#define DEF_NOUN -888
#define DEF_DUMP_FUNC_NOUN -801

Tnode *G_sys_defs;

Tnode *__d_get_def(Symbol noun) {
    if (noun < 0) {
	for(int i=1;i<=_t_children(G_sys_defs);i++){
	    Tnode *d = _t_get_child(G_sys_defs,i);
	    if (noun == *(int *)_t_surface(d))
		return d;
	}
	return NULL;
    }
    else {
	raise_error0("NOT IMPLEMENTED\n");
    }
}

Tnode *_d_get_def(Tnode *t) {
    Symbol noun = _t_noun(t);
    return __d_get_def(noun);
}

typedef char * (*dumpFn)(void *);

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
	if (f) printf("%s\n",(*f)(s));
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

char *_dump_def(void *surface) {
    Symbol noun = *(Symbol *)surface;
    Tnode *d = __d_get_def(noun);
    return (char *)_t_get_child_surface(d,1);
}

Tnode *__sys_def(char *label,Symbol noun,dumpFn fn) {
    Tnode *d;
    d = _t_newi(G_sys_defs,DEF_NOUN,noun);
    _t_new(d,CSTRING_NOUN,label,strlen(label)+1);
    _t_newi(d,DEF_DUMP_FUNC_NOUN,(long)fn);
    return d;
}

void sys_defs_init() {
    G_sys_defs = _t_new_root(SYS_DEFS_NOUN);

    Tnode *d;
    d = __sys_def("DEF",DEF_NOUN,_dump_def);
    _t_newi(d,META_NOUN,META_NOUN);

    d = __sys_def("META",META_NOUN,_dump_def);
    _t_newi(d,META_NOUN,META_NOUN);

    d = __sys_def("DEF_DUMP_FUNC",DEF_DUMP_FUNC_NOUN,0);
    _t_newi(d,META_NOUN,DEF_DUMP_FUNC_NOUN);

    d = __sys_def("CSTRING",CSTRING_NOUN,_dump_cstring);
    _t_newi(d,META_NOUN,CSTRING_NOUN);
}

void sys_defs_free() {
    _t_free(G_sys_defs);
}

#endif
