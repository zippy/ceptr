#include "../src/ceptr.h"

void testWordNew() {
    Symbol new_word = _w_sys_new(INTEGER_NOUN,"AGE");
    spec_is_ptr_equal(_w_get_def(G_sys_words,new_word),__d_get_def(INTEGER_NOUN));
    spec_is_str_equal(_w_get_label(G_sys_words,new_word),"AGE");
}

void testWord() {
    sys_defs_init();
    testWordNew();
    sys_defs_free();
}
