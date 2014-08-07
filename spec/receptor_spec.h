#include "../src/ceptr.h"
#include "../src/receptor.h"

void testCreateReceptor() {
    Receptor *r;
    r = _r_new();
    spec_is_equal(_t_symbol(r->root),RECEPTOR);

    // test that the flux is set up correctly
    Tnode *t = _t_child(r->root,1);
    spec_is_equal(_t_symbol(r->flux),FLUX);
    spec_is_ptr_equal(t,r->flux);
    t = _t_child(r->flux,1);
    spec_is_equal(_t_symbol(t),ASPECT);

    // test that listeners are set up correctly
    t = _t_child(r->root,2);
    spec_is_equal(_t_symbol(r->listeners),LISTENERS);
    spec_is_ptr_equal(t,r->listeners);

    // test that you can add a listener to a receptor
    Tnode *s = _t_new_root(EXPECTATION);
    _t_newi(s,SEMTREX_SYMBOL_LITERAL,0);
    Tnode *a = _t_new_root(ACTION);
    _r_add_expect(r,TEST_SYMBOL,s,a);

    Tnode *l = _t_child(r->listeners,1);      // listener should have been added as first child of listeners
    spec_is_equal(_t_symbol(l),LISTENER);
    spec_is_equal(*(int *)_t_surface(l),TEST_SYMBOL); // carrier should be TEST_SYMBOL
    spec_is_ptr_equal(_t_child(l,1),s);       // our expectation semtrex should be first child of the listener
    spec_is_ptr_equal(_t_child(l,2),a);       // our action code tree should be the second child of the listener

    _r_free(r);
}

void testSignalReceptor() {
    Receptor *r = _r_new();
    Tnode *signal = _t_newi(0,TEST_SYMBOL,314);
    _r_send(r,r,DEFAULT_ASPECT,signal);

    // the first node on the default aspect should be the signal
    Tnode *t = _t_child(_t_child(r->flux,1),1);
    spec_is_equal(_t_symbol(t),TEST_SYMBOL);
    spec_is_equal(*(int *)_t_surface(t),314);
    _r_free(r);
    _t_free(signal);
}

void testReceptor() {
    testCreateReceptor();
    testSignalReceptor();
}
