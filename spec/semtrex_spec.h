#include "../src/ceptr.h"
#include "../src/hashfn.h"
#include "../src/semtrex.h"
#define S1 1
#define S2 2
#define S3 3

Tnode *_makeTestTree(); //defined it tree_spec.h

void testMatchTrees() {
    Tnode *t = _makeTestTree();
    Tnode *s = _t_newi(0,SEMTREX_SYMBOL_LITERAL,S1);
    Tnode *s1 = _t_newi(s,SEMTREX_SYMBOL_LITERAL,S2);

    spec_is_true(_t_match(s,t));

    Tnode *s2 = _t_newi(s,SEMTREX_SYMBOL_LITERAL,S2);

    spec_is_true(!_t_match(s,t));

    _t_free(t);
}

void testSemtrex() {
    testMatchTrees();
}
