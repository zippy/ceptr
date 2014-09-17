#ifndef _CEPTR_WORD_H
#define _CEPTR_WORD_H
#include "tree.h"

Tnode *G_sys_words;
Symbol _w_new(Tnode *p,Symbol noun,char *label) {
    Tnode *w = _t_newi(p,WORD_NOUN,noun);
    _t_new(w,CSTRING_NOUN,label,strlen(label)+1);
    return _t_children(w);
}

Symbol _w_sys_new(Symbol noun,char *label) {
    if (!G_sys_words) G_sys_words = _t_new_root(WORDS_ARRAY_NOUN);
    return _w_new(G_sys_words,noun,label);
}

Tnode *_w_get_def(Tnode *wt,Symbol word) {
    Tnode *w = _t_get_child(wt,word);
    return __d_get_def(*(int *)_t_surface(w));
}

char *_w_get_label(Tnode *wt,Symbol word) {
    return (char *)_t_get_child_surface(_t_get_child(wt,word),1);
}

#endif
