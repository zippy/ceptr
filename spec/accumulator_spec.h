/**
 * @file de_spec.h
 * @copyright Copyright (C) 2013-2014, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 * @ingroup tests
 */

#include "../src/ceptr.h"
#include "../src/accumulator.h"
#include <sys/dir.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

void _testReceptorClockAddExpectation(Receptor *r);
void testAccBootStrap() {
    spec_is_ptr_equal(G_vm,NULL);

    // test first boot before any initialization has occurred

    // first boot should create a data directory
    char *dname = "tmp/test_vm";
    struct stat st = {0};

    system("rm -r tmp/test_vm");  // cleanup from previous test runs

    _a_boot(dname);
    spec_is_equal(stat(dname, &st),0); // does after boot

    spec_is_str_equal(t2s(G_vm->r->root),"(RECEPTOR_INSTANCE (INSTANCE_OF:SYS_RECEPTOR) (CONTEXT_NUM:0) (PARENT_CONTEXT_NUM:-1) (RECEPTOR_STATE (FLUX (DEFAULT_ASPECT (EXPECTATIONS) (SIGNALS))) (PENDING_SIGNALS) (PENDING_RESPONSES) (RECEPTOR_ELAPSED_TIME:0)))");

    // new clock receptor should be instantiated and active
    spec_is_equal(G_vm->active_receptor_count,1);
    Receptor *clock = G_vm->active_receptors[0].r;
    spec_is_str_equal(t2s(clock->root),"(RECEPTOR_INSTANCE (INSTANCE_OF:CLOCK_RECEPTOR) (CONTEXT_NUM:4) (PARENT_CONTEXT_NUM:0) (RECEPTOR_STATE (FLUX (DEFAULT_ASPECT (EXPECTATIONS (EXPECTATION (CARRIER:tell_time) (PATTERN (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:CLOCK_TELL_TIME))) (ACTION:respond with current time) (PARAMS) (END_CONDITIONS (UNLIMITED)))) (SIGNALS))) (PENDING_SIGNALS) (PENDING_RESPONSES) (RECEPTOR_ELAPSED_TIME:0)))");

    // now add an expectation in the clock
    T *tick = __r_make_tick();
    // _testReceptorClockAddExpectation(clock);

    // sleep for a bit, which should allow the clock to run for a second
    // and reduce the tick expectation to the tick it received
    //sleep(1);

    // @todo, this is broken since weve changed how the clock receptor works...
    //spec_is_str_equal(_td(clock,tick),_td(clock,_t_child(clock->q->completed->context->run_tree,1)));


    // __r_kill(clock);

    // clone the clock for later comparison
    T *clk_flux = _t_clone(clock->flux);

    // now shut down the vm
    _a_shut_down();
    spec_is_ptr_equal(G_vm,NULL);

    // boot again
    _a_boot(dname);

    // verify that clock with planted expectation and it's accumulated data are re-instantiated at boot
    //__r_dump_instances(G_vm->r);

    Xaddr x = {CLOCK_RECEPTOR,1};
    T *ct = _r_get_instance(G_vm->r,x);
    spec_is_false(ct == NULL);
    /* Receptor *cr = __r_get_receptor(ct); */

    /* char buf1[1000]; */
    /* __td(cr,clk_flux,buf1); */

    /* // @todo comparison fails here because we can't control time!! sometimes it work */
    /* // depending on threads and what ran when..  grr */
    /* //    spec_is_str_equal(_t2s(&cr->defs,cr->flux),"buf1"); */
    /* // spec_is_str_equal(_t2s(&cr->defs,cr->flux),buf1); */
    /* spec_is_symbol_equal(G_vm->r,_t_symbol(cr->root),CLOCK_RECEPTOR); */
    /* spec_is_str_equal(t2s(cr->root),"(RECEPTOR_INSTANCE (CONTEXT_NUM:4) (PARENT_CONTEXT_NUM:0) (RECEPTOR_STATE (FLUX (DEFAULT_ASPECT (EXPECTATIONS (EXPECTATION (CARRIER:CLOCK_TELL_TIME) (PATTERN (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:CLOCK_TELL_TIME))) (ACTION:respond with current time) (PARAMS) (END_CONDITIONS (UNLIMITED)))) (SIGNALS))) (PENDING_SIGNALS) (PENDING_RESPONSES) (RECEPTOR_ELAPSED_TIME:0)))"); */
    /* spec_is_equal(G_vm->active_receptor_count,1); */

    // @todo and that they run appropriately
    _a_shut_down();
    _t_free(clk_flux);
    _t_free(tick);
}

void testAccInstances() {
    Instances i = NULL;
    T *t = _t_newi(0,TEST_INT_SYMBOL,1);

    Xaddr x = _a_new_instance(&i,t);
    spec_is_ptr_equal(t,_a_get_instance(&i,x));

    t = _t_newi(0,TEST_INT_SYMBOL,2);
    _a_set_instance(&i,x,t);
    spec_is_ptr_equal(t,_a_get_instance(&i,x));

    t = _t_new_str(0,TEST_STR_SYMBOL,"fish");
    x = _a_new_instance(&i,t);

    t = _t_newi(0,TEST_INT_SYMBOL,3);
    x = _a_new_instance(&i,t);

    t = _t_newi(0,TEST_INT_SYMBOL,4);
    _a_new_instance(&i,t);
    _a_delete_instance(&i,x);

    t = _a_get_instance(&i,x);
    spec_is_ptr_equal(t,NULL);

    spec_is_str_equal(t2s(i),"(INSTANCE_STORE (INSTANCES (SYMBOL_INSTANCES:TEST_INT_SYMBOL (TEST_INT_SYMBOL:2) (DELETED_INSTANCE) (TEST_INT_SYMBOL:4)) (SYMBOL_INSTANCES:TEST_STR_SYMBOL (TEST_STR_SYMBOL:fish))))");
    _a_free_instances(&i);
}

void testAccGetInstances() {
    Instances i = NULL;
    T *t;
    Xaddr x;

    t = _t_newi(0,TEST_INT_SYMBOL,1);
    x = _a_new_instance(&i,t);
    t = _t_newi(0,TEST_INT_SYMBOL,2);
    x = _a_new_instance(&i,t);
    t = _t_newi(0,TEST_INT_SYMBOL,3);
    x = _a_new_instance(&i,t);
    t = _t_newi(0,TEST_INT_SYMBOL,4);
    _a_new_instance(&i,t);

    spec_is_str_equal(t2s(i),"(INSTANCE_STORE (INSTANCES (SYMBOL_INSTANCES:TEST_INT_SYMBOL (TEST_INT_SYMBOL:1) (TEST_INT_SYMBOL:2) (TEST_INT_SYMBOL:3) (TEST_INT_SYMBOL:4))))");

    _a_delete_instance(&i,x);

    t = _t_new_root(ITERATION_DATA);
    _a_get_instances(&i,TEST_INT_SYMBOL,t);
    spec_is_str_equal(t2s(t),"(ITERATION_DATA (TEST_INT_SYMBOL:1) (TEST_INT_SYMBOL:2) (TEST_INT_SYMBOL:4))");
    _t_free(t);

    _a_free_instances(&i);

}

void testAccPersistInstances() {
    Instances i = NULL;

    T *it = _t_newi(0,TEST_INT_SYMBOL,1);
    _a_new_instance(&i,it);

    it = _t_newi(0,TEST_INT_SYMBOL,2);
    Xaddr z = _a_new_instance(&i,it);

    T *t = _t_new_root(PARAMS);
    _t_newi(t,TEST_INT_SYMBOL,314);
    Xaddr x = _a_new_instance(&i,t);

    T *ht = _makeTestHTTPRequestTree(); // GET /groups/5/users.json?sort_by=last_name?page=2 HTTP/1.0
    T *htc = _t_clone(ht);
    Xaddr y = _a_new_instance(&i,ht);

    S *s = __a_serialize_instances(&i);
    _a_free_instances(&i);
    __a_unserialize_instances(G_sem,&i,s);

    spec_is_str_equal(t2s(_a_get_instance(&i,x)),"(PARAMS (TEST_INT_SYMBOL:314))");
    spec_is_str_equal(t2s(_a_get_instance(&i,y)),t2s(htc));
    spec_is_str_equal(t2s(_a_get_instance(&i,z)),"(TEST_INT_SYMBOL:2)");

    _t_free(htc);
    _a_free_instances(&i);
    free(s);
}

void testAccToken() {
    Instances i = NULL;
    T *t,*token1,*token2,*d1,*d2;
    Xaddr x,xx;

    t = _t_newi(0,TEST_INT_SYMBOL,1);
    x = _a_new_instance(&i,t);

    d1 = _t_newi(0,TEST_INT_SYMBOL,314); // dependency 1
    d2 = _t_newi(0,TEST_INT_SYMBOL,123); // dependency 2
    // generate a token for the xaddr with  each dependency
    token1 = _a_gen_token(&i,x,d1);
    token2 = _a_gen_token(&i,x,d2);

    spec_is_str_equal(t2s(i),"(INSTANCE_STORE (INSTANCES (SYMBOL_INSTANCES:TEST_INT_SYMBOL (TEST_INT_SYMBOL:1))) (INSTANCE_TOKENS (LAST_TOKEN:2) (INSTANCE_TOKEN:1 (TOKEN_XADDR:TEST_INT_SYMBOL.1) (DEPENDENCY_HASH:1156909558)) (INSTANCE_TOKEN:2 (TOKEN_XADDR:TEST_INT_SYMBOL.1) (DEPENDENCY_HASH:1552538631))))");

    // test getting back xaddrs from tokens and their dependency
    xx = _a_get_token_xaddr(&i,token1,d1);
    spec_is_xaddr_equal(G_sem,x,xx);
    xx = _a_get_token_xaddr(&i,token2,d2);
    spec_is_xaddr_equal(G_sem,x,xx);

    // test that getting an xaddr from a token with an incorrect dependency fails
    xx = _a_get_token_xaddr(&i,token1,d2);
    spec_is_true(is_null_xaddr(xx));
    xx = _a_get_token_xaddr(&i,token2,d1);
    spec_is_true(is_null_xaddr(xx));

    // test adding a dependency to a token
    T *d3 = _t_newi(0,TEST_INT_SYMBOL,999); // dependency 3
    xx = _a_get_token_xaddr(&i,token1,d3);
    spec_is_true(is_null_xaddr(xx));
    _a_add_dependency(&i,token1,d3);
    spec_is_str_equal(t2s(i),"(INSTANCE_STORE (INSTANCES (SYMBOL_INSTANCES:TEST_INT_SYMBOL (TEST_INT_SYMBOL:1))) (INSTANCE_TOKENS (LAST_TOKEN:2) (INSTANCE_TOKEN:1 (TOKEN_XADDR:TEST_INT_SYMBOL.1) (DEPENDENCY_HASH:1156909558) (DEPENDENCY_HASH:1973223375)) (INSTANCE_TOKEN:2 (TOKEN_XADDR:TEST_INT_SYMBOL.1) (DEPENDENCY_HASH:1552538631))))");
    xx = _a_get_token_xaddr(&i,token1,d3);
    spec_is_xaddr_equal(G_sem,x,xx);
    xx = _a_get_token_xaddr(&i,token2,d3);
    spec_is_true(is_null_xaddr(xx));

    _t_free(token1);
    _t_free(token2);
    _t_free(d1);
    _t_free(d2);
    _t_free(d3);
    _a_free_instances(&i);

}

void testAccumulator() {
    struct stat st = {0};
    char *temp_dir = "tmp";
    if (stat(temp_dir, &st) == -1) {
        mkdir(temp_dir,0700);
    }

    testAccBootStrap();
    testAccInstances();
    testAccGetInstances();
    testAccPersistInstances();
    testAccToken();
}
