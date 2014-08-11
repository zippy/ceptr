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
    spec_is_equal(*(int *)_t_surface(t),DEFAULT_ASPECT);

    // test that listeners and signals are set up correctly on the default aspect
    t = __r_get_listeners(r,DEFAULT_ASPECT);
    spec_is_equal(_t_symbol(t),LISTENERS);
    t = __r_get_signals(r,DEFAULT_ASPECT);
    spec_is_equal(_t_symbol(t),SIGNALS);

    // test that you can add a listener to a receptor's aspect
    Tnode *s = _t_new_root(EXPECTATION);
    _t_newi(s,SEMTREX_SYMBOL_LITERAL,0);
    Tnode *a = _t_new_root(ACTION);
    _r_add_expect(r,DEFAULT_ASPECT,TEST_SYMBOL,s,a);

    Tnode *l = _t_child(__r_get_listeners(r,DEFAULT_ASPECT),1);      // listener should have been added as first child of listeners
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

    // the first node on the default aspect signals should be the signal
    Tnode *s = _t_child(__r_get_signals(r,DEFAULT_ASPECT),1);
    spec_is_equal(_t_symbol(s),SIGNAL);
    Tnode *t = _t_child(s,1);  // whose first child should be the contents
    spec_is_equal(*(int *)_t_surface(t),314);
    spec_is_ptr_equal(signal,t);  // at some point this should probably fail, because we should have cloned the signal, not added it directly

    _r_free(r);
}

void testActionReceptor() {
/*
    Receptor *r = _r_new();
    Tnode *signal = _t_newi(0,TEST_SYMBOL,314);

    Tnode *s = _t_new_root(EXPECTATION);
    _t_newi(s,SEMTREX_SYMBOL_LITERAL,TEST_SYMBOL);
    Tnode *a = _t_new_root(ACTION);
    Tnode *add = _t_newi(a,ACTION_WRITE_TO_FLUX,0);
    _t_newi(add,TEST_SYMBOL,12345);

    _r_add_expect(r,TEST_SYMBOL,s,a);
    _r_send(r,r,DEFAULT_ASPECT,signal);

    _r_free(r);
*/
}


void testReceptor() {
    testCreateReceptor();
    testSignalReceptor();
    testActionReceptor();
}
