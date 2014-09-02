/**
 * @file semtrex_spec.h
 * @copyright Copyright (C) 2013-2014, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 * @ingroup tests
 */

#include "../src/ceptr.h"
#include "../src/hashfn.h"
#include "../src/semtrex.h"
#include "../src/receptor.h"

Tnode *_makeTestTree1() {
    Tnode *t = _t_new(0,TEST_STR_SYMBOL,"t",2);
    Tnode *t1 = _t_new(t,1,"t1",3);             // 1, 11, etc., are chosen symbols to match where the nodes are in the tree structure.  3 is length of t1....etc...
    Tnode *t11 = _t_new(t1,11,"t11",4);
    Tnode *t111 = _t_new(t11,111,"t111",5);
    Tnode *t2 = _t_new(t,2,"t2",3);
    Tnode *t21 = _t_new(t2,21,"t21",4);
    Tnode *t22 = _t_new(t2,22,"t22",4);
    Tnode *t3 = _t_new(t,3,"t3",3);
    Tnode *t4 = _t_new(t,4,"t4",3);
    return t;
}

Tnode *_makeTestSemtrex1() {
    //  /TEST_STR_SYMBOL/(1/11/111),2,3
    Tnode *s = _t_newi(0,SEMTREX_SYMBOL_LITERAL,TEST_STR_SYMBOL);
    Tnode *ss = _t_newi(s,SEMTREX_SEQUENCE,0);
    Tnode *s1 = _t_newi(ss,SEMTREX_SYMBOL_LITERAL,1);
    Tnode *s11 = _t_newi(s1,SEMTREX_SYMBOL_LITERAL,11);
    Tnode *s111 = _t_newi(s11,SEMTREX_SYMBOL_LITERAL,111);
    Tnode *s2 = _t_newi(ss,SEMTREX_SYMBOL_LITERAL,2);
    Tnode *s3 = _t_newi(ss,SEMTREX_SYMBOL_LITERAL,3);
    return s;
}

static int dump_id = 99;

void __s_dump(SState *s) {
    if (s->_did == dump_id) {printf("X");return;}
    s->_did = dump_id;
    switch (s->type) {
    case StateMatch:
	printf("(M)");
	break;
    case StateGroup:
	if (s->data.group.type == GroupOpen)
	    printf("{%s",_d_get_symbol_name(0,s->data.group.id));
	else
	    printf("%s}",_d_get_symbol_name(0,s->data.group.id));
	break;
    case StateSymbol:
	printf("(%s:%d)",_d_get_symbol_name(0,s->data.symbol),s->transition);
	break;
    case StateValue:
	printf("(%s:%d=)",_d_get_symbol_name(0,s->data.value.symbol),s->transition);
	break;
    case StateAny:
	printf("(.:%d)",s->transition);
	break;
    case StateSplit:
	printf("S");
	break;
    }
    if (s->out) {printf("->");__s_dump(s->out);}
    if (s->out1) {printf("[->");__s_dump(s->out1);printf("]");}
    //        printf("\n");
}

void _s_dump(SState *s) {
    ++dump_id;
    __s_dump(s);
}


#define spec_state_equal(sa,st,tt,s) \
    spec_is_equal(sa->type,st);\
    spec_is_equal(sa->transition,tt);\
    spec_is_symbol_equal(0,sa->data.symbol,s);	\
    spec_is_ptr_equal(sa->out1,NULL);


void testMakeFA() {
    SState *s1, *s2, *s3, *s4, *s5, *s6;
    Tnode *s = _makeTestSemtrex1();

    int states = 0;
    SState *sa = _s_makeFA(s,&states);
    spec_is_equal(states,6);

    spec_state_equal(sa,StateSymbol,TransitionDown,TEST_STR_SYMBOL);

    s1 = sa->out;
    spec_state_equal(s1,StateSymbol,TransitionDown,1);

    s2 = s1->out;
    spec_state_equal(s2,StateSymbol,TransitionDown,11);

    s3 = s2->out;
    spec_state_equal(s3,StateSymbol,-2,111);

    s4 = s3->out;
    spec_state_equal(s4,StateSymbol,TransitionNextChild,2);

    s5 = s4->out;
    spec_state_equal(s5,StateSymbol,TransitionUp,3);

    s6 = s5->out;
    spec_is_equal(s6->type,StateMatch);

    spec_is_ptr_equal(s6->out,NULL);

    _s_freeFA(sa);
    _t_free(s);
}

void testMatchTrees() {
    Tnode *t = _makeTestTree1();
    Tnode *s = _makeTestSemtrex1(t);
    Tnode *s2;

    spec_is_true(_t_match(s,t));

    s2 = _t_newi(_t_child(s,1),SEMTREX_SYMBOL_LITERAL,99);

    spec_is_true(!_t_match(s,t));

    _t_free(t);
    _t_free(s);
}

void testMatchOr() {
    Tnode *t = _makeTestTree1();
    Tnode *s = _t_newi(0,SEMTREX_OR,0);
    Tnode *s1 = _t_newi(s,SEMTREX_SYMBOL_LITERAL,1);
    Tnode *s11 = _t_newi(s1,SEMTREX_SYMBOL_LITERAL,11);
    Tnode *s2 = _t_newi(s,SEMTREX_SYMBOL_LITERAL,TEST_STR_SYMBOL);
    Tnode *s3;

    spec_is_true(_t_match(s,t));

    s3 = _t_newi(s2,SEMTREX_SYMBOL_LITERAL,99);

    spec_is_true(!_t_match(s,t));

    _t_free(t);
    _t_free(s);
}

void testMatchAny() {
    Tnode *t = _t_new(0,0,"t",2);
    Tnode *t1 = _t_new(t,1,"t1",3);

    Tnode *s = _t_newi(0,SEMTREX_SYMBOL_ANY,0);
    Tnode *ss = _t_newi(s,SEMTREX_SEQUENCE,0);
    Tnode *s1 = _t_newi(ss,SEMTREX_SYMBOL_LITERAL,1);

    spec_is_true(_t_match(s,t));
    t->contents.symbol = 99;
    spec_is_true(_t_match(s,t));

    _t_free(t);
    _t_free(s);
}

void testMatchStar() {
    // /TEST_SYMBOL/1*
    Tnode *s = _t_newi(0,SEMTREX_SYMBOL_LITERAL,TEST_SYMBOL);
    Tnode *ss = _t_newi(s,SEMTREX_SEQUENCE,0);
    Tnode *sss = _t_newi(ss,SEMTREX_ZERO_OR_MORE,0);
    Tnode *s1 = _t_newi(sss,SEMTREX_SYMBOL_LITERAL,1);
    Tnode *t1, *t1x, *t1y, *s2;

    Tnode *t = _t_new(0,TEST_SYMBOL,"t",2);
    spec_is_true(_t_match(s,t));

    t1 = _t_new(t,1,"t1",3);
    spec_is_true(_t_match(s,t));

    t1x = _t_new(t,1,"t1",3);
    t1y = _t_new(t,2,"t2",3);
    spec_is_true(_t_match(s,t));

    // /TEST_SYMBOL/1*,2
    s2 = _t_newi(ss,SEMTREX_SYMBOL_LITERAL,2);
    spec_is_true(_t_match(s,t));
    (*(int *)_t_surface(s2)) = 3;

    spec_is_true(!_t_match(s,t));
    _t_free(t);
    _t_free(s);
}

void testMatchPlus() {
    // /TEST_SYMBOL/1+
    Tnode *s = _t_newi(0,SEMTREX_SYMBOL_LITERAL,TEST_SYMBOL);
    Tnode *ss = _t_newi(s,SEMTREX_SEQUENCE,0);
    Tnode *sss = _t_newi(ss,SEMTREX_ONE_OR_MORE,0);
    Tnode *s1 = _t_newi(sss,SEMTREX_SYMBOL_LITERAL,1);
    Tnode *t1, *t1x, *t1y;

    Tnode *t = _t_new(0,TEST_SYMBOL,"t",2);
    spec_is_true(!_t_match(s,t));

    t1 = _t_new(t,1,"t1",3);
    spec_is_true(_t_match(s,t));

    t1x = _t_new(t,1,"t1",3);
    t1y = _t_new(t,2,"t2",3);
    spec_is_true(_t_match(s,t));

    _t_free(t);
    _t_free(s);
}

void testMatchQ() {
    // /TEST_SYMBOL/1?
    Tnode *s = _t_newi(0,SEMTREX_SYMBOL_LITERAL,TEST_SYMBOL);
    Tnode *ss = _t_newi(s,SEMTREX_SEQUENCE,0);
    Tnode *sss = _t_newi(ss,SEMTREX_ZERO_OR_ONE,0);
    Tnode *s1 = _t_newi(sss,SEMTREX_SYMBOL_LITERAL,1);
    Tnode *t1, *t1x, *t1y, *s2;

    Tnode *t = _t_new(0,TEST_SYMBOL,"t",2);
    spec_is_true(_t_match(s,t));

    t1 = _t_new(t,1,"t1",3);
    spec_is_true(_t_match(s,t));

    t1x = _t_new(t,1,"t1",3);
    t1y = _t_new(t,2,"t2",3);

    // /TEST_SYMBOL/1?,2
    s2 = _t_newi(ss,SEMTREX_SYMBOL_LITERAL,2);
    spec_is_true(!_t_match(s,t));

    _t_free(t);
    _t_free(s);
}

#define TEST_GROUP_SYMBOL1 1234
#define TEST_GROUP_SYMBOL2 1235

void testMatchGroup() {
    Tnode *sg2, *s3, *t, *r, *p1, *p2, *p1c, *p2c;
    t = _makeTestTree1();

    // /{TEST_STR_SYMBOL}           <- the most simple group semtrex
    Tnode *g = _t_newi(0,SEMTREX_GROUP,TEST_STR_SYMBOL);
    _t_newi(g,SEMTREX_SYMBOL_LITERAL,TEST_STR_SYMBOL);

    spec_is_true(_t_matchr(g,t,&r));

    // /TEST_STR_SYMBOL/{.*,{.}},4  <- a more complicated group semtrex
    Tnode *s = _t_newi(0,SEMTREX_SYMBOL_LITERAL,TEST_STR_SYMBOL);
    Tnode *ss = _t_newi(s,SEMTREX_SEQUENCE,0);
    Tnode *sg = _t_newi(ss,SEMTREX_GROUP,TEST_GROUP_SYMBOL1);
    Tnode *ss2 = _t_newi(sg,SEMTREX_SEQUENCE,0);
    Tnode *st = _t_newi(ss2,SEMTREX_ZERO_OR_MORE,0);
    int rp1[] = {1,TREE_PATH_TERMINATOR};
    int rp2[] = {3,TREE_PATH_TERMINATOR};

    _t_newi(st,SEMTREX_SYMBOL_ANY,0);
    sg2 = _t_newi(ss2,SEMTREX_GROUP,TEST_GROUP_SYMBOL2);
    _t_newi(sg2,SEMTREX_SYMBOL_ANY,0);
    s3 = _t_newi(ss,SEMTREX_SYMBOL_LITERAL,4);

    _t_free(r);

    spec_is_true(_t_matchr(s,t,&r));
    spec_is_symbol_equal(0,_t_symbol(r),SEMTREX_MATCH_RESULTS);
    spec_is_equal(_t_children(r),2);

    // you should be able to find the matched group positionaly
    p1 = _t_child(r,1);

    spec_is_symbol_equal(0,_t_symbol(p1),SEMTREX_MATCH);
    spec_is_equal(_t_children(p1),2);

    p1c = _t_child(p1,2);

    //    printf("%s\n",_td(r));
    spec_is_symbol_equal(0,_t_symbol(p1c),SEMTREX_MATCH_SIBLINGS_COUNT);
    spec_is_equal(*(int *)_t_surface(p1c),3);
    spec_is_path_equal(_t_surface(_t_child(p1,1)),rp1);


    p2 = _t_child(r,2);
    spec_is_symbol_equal(0,_t_symbol(p2),SEMTREX_MATCH);

    p2c = _t_child(p2,2);
    spec_is_symbol_equal(0,_t_symbol(p2c),SEMTREX_MATCH_SIBLINGS_COUNT);
    spec_is_equal(*(int *)_t_surface(p2c),1);
    spec_is_path_equal(_t_surface(_t_child(p2,1)),rp2);

    // you should also be able to find the matched group semantically
    spec_is_ptr_equal(_t_get_match(r,TEST_GROUP_SYMBOL1),p1);
    spec_is_ptr_equal(_t_get_match(r,TEST_GROUP_SYMBOL2),p2);

    _t_free(r);
    _t_free(t);
    _t_free(s);
    _t_free(g);
}

void testMatchLiteralValue() {
    Tnode *t = _makeTestTree1();
    Tnode *s;
    Svalue sv;
    sv.symbol = TEST_STR_SYMBOL;
    sv.length = 2;
    ((char *)&sv.value)[0] = 't';
    ((char *)&sv.value)[1] = 0;				// string terminator

    // /TEST_SYMBOL="t"
    s = _t_new(0,SEMTREX_VALUE_LITERAL,&sv,sizeof(Svalue));
    spec_is_true(_t_match(s,t));
    _t_free(s);

    // /TEST_SYMBOL2="t"
    // don't match on wrong symbol
    sv.symbol = TEST_SYMBOL2;
    s = _t_new(0,SEMTREX_VALUE_LITERAL,&sv,sizeof(Svalue));
    spec_is_true(!_t_match(s,t));
    _t_free(s);
    sv.symbol = TEST_SYMBOL; // restore correct symbol

    // /TEST_SYMBOL="x"
    // don't match if value is wrong
    ((char *)&sv.value)[0] = 'x';
    s = _t_new(0,SEMTREX_VALUE_LITERAL,&sv,sizeof(Svalue));
    spec_is_true(!_t_match(s,t));
    _t_free(s);

    // /TEST_SYMBOL=""
    // don't match if value length is wrong
    ((char *)&sv.value)[0] = 0;
    sv.length = 1;			// length of data is 1, string length is 0.
    s = _t_new(0,SEMTREX_VALUE_LITERAL,&sv,sizeof(Svalue));
    spec_is_true(!_t_match(s,t));
    _t_free(s);

    _t_free(t);
}

void testSemtrex() {
    testMakeFA();
    testMatchTrees();
    testMatchOr();
    testMatchAny();
    testMatchStar();
    testMatchPlus();
    testMatchQ();
    testMatchGroup();
    testMatchLiteralValue();
}
