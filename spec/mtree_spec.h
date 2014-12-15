/**
 * @file mtree_spec.h
 * @copyright Copyright (C) 2013-2014, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 * @ingroup tests
 */

#include "../src/ceptr.h"
#include "../src/receptor.h"
#include "http_example.h"

/*
#define TT(t) ((t) && (*((int *)t) == matrixImpl))
#define TE(t,te,me) ((TT(t)) ? (me):(te))
#define TNEW(impl,parent,symbol,surface,size) ((impl == matrixImpl) ? (GT)_m_new(parent,symbol,surface,size) : (GT)_t_new(parent,symbol,surface,size))
#define TFREE(t) TE(t,_t_free((T*)t),_m_free((M*)t))
#define TSIZE(t) TE(t,_t_size((T *)t),_m_size((M *)t))
#define TKIDS(t) TE(t,_t_children((T *)t),_m_children((M *)t))
#define TSURFACE(t) TE(t,_t_surface((T *)t),_m_surface((M *)t))

void testCreateTreeNodesT() {
    uint32_t impl;
    for(impl=FIRST_TREE_IMPL_TYPE;impl != 0;impl++) {
	GT t = TNEW(impl,0,TEST_STR_SYMBOL,"hello",6);
	spec_is_long_equal(TSIZE(t),(size_t)6);
	spec_is_equal(TKIDS(t),0);
	spec_is_str_equal((char *)TSURFACE(t),"hello");

	TFREE(t);
    }
}
*/

mtd(H h) {
    H hh;
    int i;
    printf("\n");
    for(i=0;i<h.m->levels;i++) {
	hh.m = h.m;
	hh.a.l = i;
	L *l = _GET_LEVEL(h,i);
	int j;
	printf("%d(%d):",i,l->nodes);
	for(j=0;j<l->nodes;j++) {
	    hh.a.i = j;
	    N *n = _GET_NODE(h,l,j);
	    if (!(n->flags & TFLAG_DELETED))
		printf("%s:%d, ",(char *)_m_surface(hh),n->parenti);
	    else
		printf("X, ");
	}
	printf("\n");
    }
}

void testCreateTreeNodesM(){

    H h,h1,h11,h2,h21,h22,h3;

    // test adding a new root
    h = _m_new(null_H,TEST_STR_SYMBOL,"hello",6);
    spec_is_equal(h.a.l,0);
    spec_is_equal(h.a.i,0);
    spec_is_equal(h.m->levels,1);
    spec_is_long_equal(_m_size(h),(size_t)6);
    spec_is_equal(_m_children(h),0);
    spec_is_str_equal((char *)_m_surface(h),"hello");
    spec_is_maddr_equal(_m_parent(h),null_H.a);
    spec_is_maddr_equal(_m_child(h,1),null_H.a);
    spec_is_maddr_equal(_m_child(h,NULL_ADDR),null_H.a);
    spec_is_true(semeq(_m_symbol(h),TEST_STR_SYMBOL));

    // test adding first child
    h1 = _m_new(h,TEST_STR_SYMBOL,"t1",3);
    spec_is_equal(h.m->levels,2);
    spec_is_str_equal((char *)_m_surface(h1),"t1");
    spec_is_maddr_equal(_m_parent(h1),h.a);
    spec_is_equal(_m_children(h),1);
    spec_is_maddr_equal(_m_child(h,1),h1.a);
    spec_is_maddr_equal(_m_child(h,NULL_ADDR),h1.a);
    spec_is_maddr_equal(_m_child(h1,NULL_ADDR),null_H.a);

    // test adding second child
    h2 = _m_new(h,TEST_STR_SYMBOL,"t2",3);
    spec_is_equal(h.m->levels,2);
    spec_is_str_equal((char *)_m_surface(h2),"t2");
    spec_is_maddr_equal(_m_parent(h2),h.a);
    spec_is_equal(_m_children(h),2);
    spec_is_maddr_equal(_m_child(h,2),h2.a);
    spec_is_maddr_equal(_m_child(h,NULL_ADDR),h2.a);
    spec_is_equal(_m_children(h2),0);

    // test adding child to second child
    h21 = _m_new(h2,TEST_STR_SYMBOL,"t21",4);
    spec_is_equal(h21.a.l,2);
    spec_is_equal(h21.a.i,0);
    spec_is_equal(h.m->levels,3);
    spec_is_str_equal((char *)_m_surface(h21),"t21");
    spec_is_maddr_equal(_m_parent(h21),h2.a);
    spec_is_equal(_m_children(h2),1);
    spec_is_maddr_equal(_m_child(h2,1),h21.a);
    spec_is_maddr_equal(_m_child(h2,NULL_ADDR),h21.a);

    // now adding in a child to the first child (i.e. out of order)
    spec_is_maddr_equal(_m_child(h1,NULL_ADDR),null_H.a);
    h11 = _m_new(h1,TEST_STR_SYMBOL,"t11",4);
    spec_is_equal(h11.a.l,2);
    spec_is_equal(h11.a.i,1);  //should be inserted after the t21 node
    spec_is_equal(h.m->levels,3);
    spec_is_str_equal((char *)_m_surface(h11),"t11");
    spec_is_maddr_equal(_m_parent(h11),h1.a);
    spec_is_equal(_m_children(h1),1);
    spec_is_maddr_equal(_m_child(h1,1),h11.a);
    spec_is_maddr_equal(_m_child(h1,NULL_ADDR),h11.a);

    // this test is here because it makes sure that things work even
    // if nodes are inserted out of order
    h22 = _m_new(h2,TEST_STR_SYMBOL,"t22",4);
    spec_is_equal(h22.a.l,2);
    spec_is_equal(h22.a.i,2);  //should be inserted after the t11 node
    spec_is_equal(h.m->levels,3);
    spec_is_str_equal((char *)_m_surface(h22),"t22");
    spec_is_maddr_equal(_m_parent(h22),h2.a);
    spec_is_equal(_m_children(h2),2);
    spec_is_maddr_equal(_m_child(h2,2),h22.a);
    spec_is_maddr_equal(_m_child(h2,NULL_ADDR),h22.a);

    Symbol s99 = {0,0,99};

    // test adding in non allocated integer surface
    h3 = _m_newi(h,s99,101);
    spec_is_maddr_equal(_m_parent(h3),h.a);
    spec_is_equal(_m_children(h),3);
    spec_is_equal(*(int *)_m_surface(h3),101);

    // test next sibling
    spec_is_maddr_equal(_m_next_sibling(h1),h2.a);
    spec_is_maddr_equal(_m_next_sibling(h2),h3.a);
    spec_is_maddr_equal(_m_next_sibling(h3),null_H.a);
    spec_is_maddr_equal(_m_next_sibling(h),null_H.a);

    h21.a = _m_child(h2,1); // we have to do this because the h21 handle is stale
    spec_is_maddr_equal(_m_next_sibling(h21),h22.a);
    spec_is_maddr_equal(_m_next_sibling(h11),null_H.a);

    // test add
    H h12 = _m_new(null_H,TEST_TREE_SYMBOL,"t12",4);
    spec_is_str_equal((char *)_m_surface(h12),"t12");
    _m_new(h12,TEST_STR_SYMBOL,"t121",4);
    H h221 = _m_new(h22,TEST_STR_SYMBOL,"t221",5);
    spec_is_equal(h221.a.l,3);
    spec_is_equal(h221.a.i,0);  //should be first and only node on third level
    spec_is_equal(GET_LEVEL(h221)->nodes,1);

    h12 = _m_add(h1,h12);
    spec_is_equal(h12.a.l,2);
    spec_is_equal(h12.a.i,3);  //should have been appended
    spec_is_equal(_m_children(h1),2);
    spec_is_maddr_equal(_m_child(h1,2),h12.a);
    h22.a = _m_child(h2,2);
    spec_is_equal(h22.a.l,2);
    spec_is_equal(h22.a.i,2);  //should remain the same

    spec_is_equal(GET_LEVEL(h221)->nodes,2);
    spec_is_str_equal((char *)_m_surface(h221),"t221");

    h221.a = _m_child(h22,1);
    spec_is_str_equal((char *)_m_surface(h221),"t221");

    H h121;
    h121.m = h.m;
    h121.a = _m_child(h12,1);
    spec_is_str_equal((char *)_m_surface(h121),"t121");
    spec_is_equal(h121.a.l,3);
    spec_is_equal(h121.a.i,1);  //should be last because it was appended by add

    //    mtd(h);
    spec_is_equal(_m_children(h),3);
    h2 = _m_detatch(h2);
    //    mtd(h2);
    //    mtd(h);

    spec_is_true(h2.m != h.m);
    spec_is_equal(_m_children(h2),2);
    spec_is_equal(_m_children(h),2);
    //    _m_free(h22);

    _m_free(h);
}

void testMTree() {
    testCreateTreeNodesM();
}
