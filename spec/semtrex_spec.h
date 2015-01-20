/**
 * @file semtrex_spec.h
 * @copyright Copyright (C) 2013-2014, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 * @ingroup tests
 */

#include "../src/ceptr.h"
#include "../src/hashfn.h"
#include "../src/semtrex.h"
#include "../src/receptor.h"

Symbol sy0 = {0,0,0};
Symbol sy1 = {0,0,1};
Symbol sy11 = {0,0,11};
Symbol sy111 = {0,0,111};
Symbol sy2 = {0,0,2};
Symbol sy21 = {0,0,21};
Symbol sy22 = {0,0,22};
Symbol sy3 = {0,0,3};
Symbol sy4 = {0,0,4};

Symbol TEST_GROUP_SYMBOL1;
Symbol TEST_GROUP_SYMBOL2;

#define sYt(name,str) name = _d_declare_symbol(G_sys_defs.symbols,str,"" #name "",TEST_CONTEXT)
#define sX(name,str) Symbol name = _d_declare_symbol(G_sys_defs.symbols,str,"" #name "",TEST_CONTEXT)

void _stxSetup() {
    sYt(sy0,CSTRING);
    sYt(sy1,CSTRING);
    sYt(sy11,CSTRING);
    sYt(sy111,CSTRING);
    sYt(sy2,CSTRING);
    sYt(sy21,CSTRING);
    sYt(sy22,CSTRING);
    sYt(sy3,CSTRING);
    sYt(sy4,CSTRING);
    sYt(TEST_GROUP_SYMBOL1,INTEGER);
    sYt(TEST_GROUP_SYMBOL2,CSTRING);
}

T *_makeTestTree1() {
    T *t = _t_new(0,TEST_STR_SYMBOL,"t",2);
    T *t1 = _t_new(t,sy1,"t1",3);             // 1, 11, etc., are chosen symbols to match where the nodes are in the tree structure.  3 is length of t1....etc...
    T *t11 = _t_new(t1,sy11,"t11",4);
    T *t111 = _t_new(t11,sy111,"t111",5);
    T *t2 = _t_new(t,sy2,"t2",3);
    T *t21 = _t_new(t2,sy21,"t21",4);
    T *t22 = _t_new(t2,sy22,"t22",4);
    T *t3 = _t_new(t,sy3,"t3",3);
    T *t4 = _t_new(t,sy4,"t4",3);
    return t;
}

T *_makeTestSemtrex1() {
    //  /TEST_STR_SYMBOL/(1/11/111),2,3
    T *s = _sl(0,TEST_STR_SYMBOL);
    T *ss = _t_newi(s,SEMTREX_SEQUENCE,0);
    T *s1 = _sl(ss,sy1);
    T *s11 = _sl(s1,sy11);
    T *s111 = _sl(s11,sy111);
    T *s2 = _sl(ss,sy2);
    T *s3 = _sl(ss,sy3);
    return s;
}

static int dump_id = 99;

void __stx_dump(SState *s) {
    T *v;
    if (s->_did == dump_id) {printf("X");return;}
    s->_did = dump_id;
    switch (s->type) {
    case StateMatch:
	printf("(M)");
	break;
    case StateGroupOpen:
	printf("{%d:%s",s->data.groupo.uid,_d_get_symbol_name(0,s->data.groupo.symbol));
	break;
    case StateGroupClose:
	printf("%d:%s}",s->data.groupc.openP->data.groupo.uid,_d_get_symbol_name(0,s->data.groupc.openP->data.groupo.symbol));
	break;
    case StateSymbol:
	printf("(%s%s:%d)",(s->data.symbol.flags & LITERAL_NOT) ? "!" : "",
	       _d_get_symbol_name(0,_t_symbol(_t_child(_t_child(s->data.symbol.symbols,1),1))),s->transition);
	break;
    case StateValue:
	printf("(%s%s=:%d)",_d_get_symbol_name(0,_t_symbol(_t_child(_t_child(s->data.value.values,1),1))),(s->data.value.flags & LITERAL_NOT) ? "!" : "",s->transition);
	break;
    case StateAny:
	printf("(.:%d)",s->transition);
	break;
    case StateDescend:
	printf("(/)");
	break;
    case StateNot:
	printf("(~)");
	break;
    case StateSplit:
	printf("S");
	break;
    case StateWalk:
	printf("(%%)");
	break;
    default:
	printf("(\?\?)");
    }
    if (s->out) {printf("->");__stx_dump(s->out);}
    if (s->out1) {printf("[->");__stx_dump(s->out1);printf("]");}
    //        printf("\n");
}

void _stx_dump(SState *s) {
    ++dump_id;
    __stx_dump(s);
}

void stx_dump(T *s) {
    int l;

    SState *f = _stx_makeFA(s,&l);    _stx_dump(f);
    _stx_freeFA(f);
}

#define spec_state_equal(sa,st,tt,s) \
    spec_is_equal(sa->type,st);\
    spec_is_equal(sa->transition,tt);\
    spec_is_symbol_equal(0,*(Symbol *)_t_surface(sa->data.symbol.symbols),s); \
    spec_is_ptr_equal(sa->out1,NULL);


void testMakeFA() {
    SState *s1, *s2, *s3, *s4, *s5, *s6;
    T *s = _makeTestSemtrex1();

    int states = 0;
    SState *sa = _stx_makeFA(s,&states);
    spec_is_equal(states,6);

    spec_state_equal(sa,StateSymbol,TransitionDown,TEST_STR_SYMBOL);

    s1 = sa->out;
    spec_state_equal(s1,StateSymbol,TransitionDown,sy1);

    s2 = s1->out;
    spec_state_equal(s2,StateSymbol,TransitionDown,sy11);

    s3 = s2->out;
    spec_state_equal(s3,StateSymbol,-2,sy111);

    s4 = s3->out;
    spec_state_equal(s4,StateSymbol,TransitionNextChild,sy2);

    s5 = s4->out;
    spec_state_equal(s5,StateSymbol,TransitionUp,sy3);

    s6 = s5->out;
    spec_is_equal(s6->type,StateMatch);

    spec_is_ptr_equal(s6->out,NULL);

    _stx_freeFA(sa);
    _t_free(s);
}

void testMatchTrees() {
    T *t = _makeTestTree1();
    T *s = _makeTestSemtrex1(t);
    T *s2;

    spec_is_true(_t_match(s,t));
    Symbol sy99 = {0,0,99};
    s2 = _sl(_t_child(s,2),sy99);
    spec_is_true(!_t_match(s,t));
    _t_free(t);
    _t_free(s);
}

T *__stxcv(T *stxx,char c);

void testMatchOr() {
    T *t = _makeTestTree1();
    T *s = _t_new_root(SEMTREX_OR);
    T *s1 = _sl(s,sy1);
    T *s11 = _sl(s1,sy11);
    T *s2 = _sl(s,TEST_STR_SYMBOL);
    T *s3;

    spec_is_true(_t_match(s,t));

    Symbol sy99 = {0,0,99};
    s3 = _sl(s2,sy99);

    spec_is_true(!_t_match(s,t));

    _t_free(t);
    _t_free(s);

    t = _t_new_root(ASCII_CHARS);
    _t_newi(t,ASCII_CHAR,'/');
    _t_newi(t,ASCII_CHAR,'X');
    _t_newi(t,ASCII_CHAR,')');

    s = _sl(0,ASCII_CHARS);
    T *x = _t_newr(s,SEMTREX_SEQUENCE);
    __stxcv(x,'/');
    T *o = _t_newr(x,SEMTREX_OR);
    T *parens = _t_newr(o,SEMTREX_SEQUENCE);
    __stxcv(parens,'(');
    __stxcv(parens,')');
    __stxcv(o,'X');

    spec_is_true(_t_match(s,t));

    _t_free(t);
    _t_free(s);

}

void testMatchAny() {
    T *t = _t_new(0,sy0,"t",2);
    T *t1 = _t_new(t,sy1,"t1",3);

    T *s = _t_new_root(SEMTREX_SYMBOL_ANY);
    T *ss = _t_newr(s,SEMTREX_SEQUENCE);
    T *s1 = _sl(ss, sy1);

    spec_is_true(_t_match(s,t));
    t->contents.symbol.id = 99;
    spec_is_true(_t_match(s,t));

    _t_free(t);
    _t_free(s);
}

void testMatchExcept() {
    T *t = _t_new(0,sy0,"t",2);
    T *t1 = _t_new(t,sy1,"t1",3);

    // //!sy0/sy1
    T *s = _sln(0,sy0);
    T *ss = _t_newr(s,SEMTREX_SEQUENCE);
    T *s1 = _sl(ss,sy1);
    spec_is_true(!_t_match(s,t));

    _t_free(s);

    // //!sy1/sy1
    s = _sln(0,sy1);
    ss = _t_newr(s,SEMTREX_SEQUENCE);
    _sl(ss,sy1);
    spec_is_true(_t_match(s,t));

    _t_free(s);

    // //!{sy1,sy2}
    s = __sl(0,1,2,sy1,sy2);
    spec_is_true(_t_match(s,t));

    _t_free(s);

    // //!{sy1,sy0}
    s = __sl(0,1,2,sy1,sy0);
    spec_is_true(!_t_match(s,t));

    _t_free(s);

    // //{sy1,sy0}
    s = __sl(0,0,2,sy1,sy0);
    spec_is_true(_t_match(s,t));

    _t_free(s);

    // //{sy1,sy2}
    s = __sl(0,0,2,sy1,sy2);
    spec_is_true(!_t_match(s,t));

    _t_free(s);
    _t_free(t);
}

void testMatchStar() {
    // /TEST_INT_SYMBOL/1*
    T *s = _sl(0,TEST_INT_SYMBOL);
    T *ss = _t_newr(s,SEMTREX_SEQUENCE);
    T *sss = _t_newr(ss,SEMTREX_ZERO_OR_MORE);
    T *s1 = _sl(sss,sy1);
    T *t1, *t1x, *t1y, *s2;

    T *t = _t_new(0,TEST_INT_SYMBOL,"t",2);
    spec_is_true(_t_match(s,t));

    t1 = _t_new(t,sy1,"t1",3);
    spec_is_true(_t_match(s,t));

    t1x = _t_new(t,sy1,"t1",3);
    t1y = _t_new(t,sy2,"t2",3);
    spec_is_true(_t_match(s,t));

    // /TEST_INT_SYMBOL/1*,2
    s2 = _sl(ss,sy2);
    spec_is_true(_t_match(s,t));
    (*(int *)_t_surface(_t_child(s2,1))) = 3;

    spec_is_true(!_t_match(s,t));
    _t_free(t);
    _t_free(s);
}

void testMatchPlus() {
    // /TEST_INT_SYMBOL/1+
    T *s = _sl(0,TEST_INT_SYMBOL);
    T *ss = _t_newr(s,SEMTREX_SEQUENCE);
    T *sss = _t_newr(ss,SEMTREX_ONE_OR_MORE);
    T *s1 = _sl(sss,sy1);
    T *t1, *t1x, *t1y;

    T *t = _t_new(0,TEST_INT_SYMBOL,"t",2);
    spec_is_true(!_t_match(s,t));

    t1 = _t_new(t,sy1,"t1",3);
    spec_is_true(_t_match(s,t));

    t1x = _t_new(t,sy1,"t1",3);
    t1y = _t_new(t,sy2,"t2",3);
    spec_is_true(_t_match(s,t));

    _t_free(t);
    _t_free(s);
}

void testMatchQ() {
    // /TEST_INT_SYMBOL/1?
    T *s = _sl(0,TEST_INT_SYMBOL);
    T *ss = _t_newr(s,SEMTREX_SEQUENCE);
    T *sss = _t_newr(ss,SEMTREX_ZERO_OR_ONE);
    T *s1 = _sl(sss,sy1);
    T *t1, *t1x, *t1y, *s2;

    T *t = _t_new(0,TEST_INT_SYMBOL,"t",2);
    spec_is_true(_t_match(s,t));

    t1 = _t_new(t,sy1,"t1",3);
    spec_is_true(_t_match(s,t));

    t1x = _t_new(t,sy1,"t1",3);
    t1y = _t_new(t,sy2,"t2",3);

    // /TEST_INT_SYMBOL/1?,2
    s2 = _sl(ss,sy2);
    spec_is_true(!_t_match(s,t));

    _t_free(t);
    _t_free(s);
}

void testMatchGroup() {
    T *sg2, *s3, *t, *r, *p1, *p2, *p1c, *p2c;
    t = _makeTestTree1();

    // /<TEST_STR_SYMBOL:TEST_STR_SYMBOL>           <- the most simple group semtrex
    T *g = _t_news(0,SEMTREX_GROUP,TEST_STR_SYMBOL);
    _sl(g,TEST_STR_SYMBOL);

    spec_is_true(_t_matchr(g,t,&r));

    _t_free(r);

    // /TEST_STR_SYMBOL/(<TEST_GROUP_SYMBOL1:.*,<TEST_GROUP_SYMBOL2:.>>,sy4)  <- a more complicated group semtrex
    T *s = _sl(0,TEST_STR_SYMBOL);
    T *ss = _t_newr(s,SEMTREX_SEQUENCE);
    T *sg = _t_news(ss,SEMTREX_GROUP,TEST_GROUP_SYMBOL1);
    T *ss2 = _t_newr(sg,SEMTREX_SEQUENCE);
    T *st = _t_newr(ss2,SEMTREX_ZERO_OR_MORE);
    _t_newr(st,SEMTREX_SYMBOL_ANY);
    sg2 = _t_news(ss2,SEMTREX_GROUP,TEST_GROUP_SYMBOL2);
    _t_newr(sg2,SEMTREX_SYMBOL_ANY);
    s3 = _sl(ss,sy4);

    spec_is_true(_t_matchr(s,t,&r));

    spec_is_str_equal(t2s(r),"(SEMTREX_MATCH:1 (SEMTREX_MATCH_SYMBOL:TEST_GROUP_SYMBOL1) (SEMTREX_MATCHED_PATH:/1) (SEMTREX_MATCH_SIBLINGS_COUNT:3) (SEMTREX_MATCH:2 (SEMTREX_MATCH_SYMBOL:TEST_GROUP_SYMBOL2) (SEMTREX_MATCHED_PATH:/3) (SEMTREX_MATCH_SIBLINGS_COUNT:1)))");

    // you should also be able to find the matched group by uid
    spec_is_ptr_equal(_t_get_match(r,TEST_GROUP_SYMBOL1),r);
    spec_is_ptr_equal(_t_get_match(r,TEST_GROUP_SYMBOL2),_t_child(r,4));
    _t_free(r);
    _t_free(s);

    // /TEST_STR_SYMBOL/(<TEST_GROUP_SYMBOL1:<TEST_GROUP_SYMBOL2:.>*>,sy4)  <- a more complicated group semtrex, this time where a group will be repeated
    s = _sl(0,TEST_STR_SYMBOL);
    ss = _t_newr(s,SEMTREX_SEQUENCE);
    sg = _t_news(ss,SEMTREX_GROUP,TEST_GROUP_SYMBOL1);
    ss2 = _t_newr(sg,SEMTREX_SEQUENCE);
    //    _t_newr(ss2,SEMTREX_SYMBOL_ANY);
    st = _t_newr(ss2,SEMTREX_ZERO_OR_MORE);
    sg2 = _t_news(st,SEMTREX_GROUP,TEST_GROUP_SYMBOL2);
    _t_newr(sg2,SEMTREX_SYMBOL_ANY);
    s3 = _sl(ss,sy4);


    spec_is_true(_t_matchr(s,t,&r));

    spec_is_str_equal(t2s(r),"(SEMTREX_MATCH:1 (SEMTREX_MATCH_SYMBOL:TEST_GROUP_SYMBOL1) (SEMTREX_MATCHED_PATH:/1) (SEMTREX_MATCH_SIBLINGS_COUNT:3) (SEMTREX_MATCH:2 (SEMTREX_MATCH_SYMBOL:TEST_GROUP_SYMBOL2) (SEMTREX_MATCHED_PATH:/1) (SEMTREX_MATCH_SIBLINGS_COUNT:1)) (SEMTREX_MATCH:2 (SEMTREX_MATCH_SYMBOL:TEST_GROUP_SYMBOL2) (SEMTREX_MATCHED_PATH:/2) (SEMTREX_MATCH_SIBLINGS_COUNT:1)) (SEMTREX_MATCH:2 (SEMTREX_MATCH_SYMBOL:TEST_GROUP_SYMBOL2) (SEMTREX_MATCHED_PATH:/3) (SEMTREX_MATCH_SIBLINGS_COUNT:1)))");
    _t_free(s);
    _t_free(r);

    // test that the correct number of siblings is returned when the match matches a
    // sequence that includes the final sibiling
    // /TEST_STR_SYMBOL/.*,<TEST_GROUP_SYMBOL:sy3,sy4>
    s = _sl(0,TEST_STR_SYMBOL);
    ss = _t_newr(s,SEMTREX_SEQUENCE);
    st = _t_newr(ss,SEMTREX_ZERO_OR_MORE);
    _t_newr(st,SEMTREX_SYMBOL_ANY);
    sg = _t_news(ss,SEMTREX_GROUP,TEST_GROUP_SYMBOL1);
    ss2 = _t_newr(sg,SEMTREX_SEQUENCE);
    _sl(ss2,sy3);
    _sl(ss2,sy4);
    //    G_debug_match = 1;
    spec_is_true(_t_matchr(s,t,&r));

    spec_is_str_equal(t2s(r), "(SEMTREX_MATCH:1 (SEMTREX_MATCH_SYMBOL:TEST_GROUP_SYMBOL1) (SEMTREX_MATCHED_PATH:/3) (SEMTREX_MATCH_SIBLINGS_COUNT:2))");

    _t_free(r);
    _t_free(t);
    _t_free(s);
    _t_free(g);
}

void testMatchGroupMulti() {
    sX(STX_TOKENS,LIST);
    sX(STX_OP,NULL_STRUCTURE);
    sX(STX_SL,NULL_STRUCTURE);

    T *s = _t_new_root(ASCII_CHARS);
    _t_newi(s,ASCII_CHAR,'/');
    _t_newi(s,ASCII_CHAR,'(');
    _t_newi(s,ASCII_CHAR,'(');

    T *ts = _t_news(0,SEMTREX_GROUP,STX_TOKENS);
    T *g = _sl(ts,ASCII_CHARS);
    T *sq = _t_newr(g,SEMTREX_SEQUENCE);
    T *p = _t_newr(sq,SEMTREX_ONE_OR_MORE);
    T *o = _t_newr(p,SEMTREX_OR);
    T *t = _t_news(o,SEMTREX_GROUP,STX_SL);
    __stxcv(t,'/');
    t = _t_news(o,SEMTREX_GROUP,STX_OP);
    __stxcv(t,'(');

    T *results;
    _t_matchr(ts,s,&results);

    spec_is_str_equal(t2s(results), "(SEMTREX_MATCH:1 (SEMTREX_MATCH_SYMBOL:STX_TOKENS) (SEMTREX_MATCHED_PATH:) (SEMTREX_MATCH_SIBLINGS_COUNT:1) (SEMTREX_MATCH:2 (SEMTREX_MATCH_SYMBOL:STX_SL) (SEMTREX_MATCHED_PATH:/1) (SEMTREX_MATCH_SIBLINGS_COUNT:1)) (SEMTREX_MATCH:3 (SEMTREX_MATCH_SYMBOL:STX_OP) (SEMTREX_MATCHED_PATH:/2) (SEMTREX_MATCH_SIBLINGS_COUNT:1)) (SEMTREX_MATCH:3 (SEMTREX_MATCH_SYMBOL:STX_OP) (SEMTREX_MATCHED_PATH:/3) (SEMTREX_MATCH_SIBLINGS_COUNT:1)))");

    _t_free(s);
    _t_free(results);
    _t_free(ts);
}

T *newvl(T *p,int not,int count,...) {
    va_list values;
    T *t =  _t_newr(p,not ? SEMTREX_VALUE_LITERAL_NOT : SEMTREX_VALUE_LITERAL);
    T *v = _t_newr(t,SEMTREX_VALUE_SET);

    va_start(values,count);
    int i;
    for(i=0;i<count;i++) {
	T *x = va_arg(values,T *);
	_t_add(v,x);
    }
    va_end(values);
    return t;
}

void testMatchLiteralValue() {
    T *t = _makeTestTree1();
    T *s;

    // /TEST_INT_SYMBOL!="t"
    s = newvl(0,1,1,_t_new(0,TEST_STR_SYMBOL,"t",2));
    spec_is_true(!_t_match(s,t));
    _t_free(s);

    // /TEST_INT_SYMBOL="t"
    s = newvl(0,0,1,_t_new(0,TEST_STR_SYMBOL,"t",2));
    spec_is_true(_t_match(s,t));
    _t_free(s);

    // /TEST_INT_SYMBOL2="t"
    // don't match on wrong symbol
    s = newvl(0,0,1,_t_new(0,TEST_INT_SYMBOL2,"t",2));
    spec_is_true(!_t_match(s,t));
    _t_free(s);

    // /TEST_INT_SYMBOL="x"
    // don't match if value is wrong
    s = newvl(0,0,1,_t_new(0,TEST_STR_SYMBOL,"x",2));
    spec_is_true(!_t_match(s,t));
    _t_free(s);

    // /TEST_INT_SYMBOL=""
    // don't match if value length is wrong
    s = newvl(0,0,1,_t_new(0,TEST_STR_SYMBOL,"tt",3));
    spec_is_true(!_t_match(s,t));
    _t_free(s);

    // /TEST_INT_SYMBOL={"t","q"}
    s = newvl(0,0,2,_t_new(0,TEST_STR_SYMBOL,"t",2),_t_new(0,TEST_STR_SYMBOL,"q",2));
    spec_is_true(_t_match(s,t));
    _t_free(s);

    // /TEST_INT_SYMBOL={"q","x"}
    s = newvl(0,0,2,_t_new(0,TEST_STR_SYMBOL,"q",2),_t_new(0,TEST_STR_SYMBOL,"x",2));
    spec_is_true(!_t_match(s,t));
    _t_free(s);

    // /TEST_INT_SYMBOL!={"q","x"}
    s = newvl(0,1,2,_t_new(0,TEST_STR_SYMBOL,"q",2),_t_new(0,TEST_STR_SYMBOL,"x",2));
    spec_is_true(_t_match(s,t));
    _t_free(s);

    // /TEST_INT_SYMBOL!={"q","t"}
    s = newvl(0,1,2,_t_new(0,TEST_STR_SYMBOL,"q",2),_t_new(0,TEST_STR_SYMBOL,"t",2));
    spec_is_true(!_t_match(s,t));
    _t_free(s);

    _t_free(t);

    t = _t_new_root(ASCII_CHARS);
    _t_newi(t,ASCII_CHAR,'a');
    _t_newi(t,ASCII_CHAR,'b');
    _t_newi(t,ASCII_CHAR,'/');

    s = _sl(0,ASCII_CHARS);
    T *sq = _t_newr(s,SEMTREX_SEQUENCE);
    T *g = _t_news(sq,SEMTREX_GROUP,TEST_GROUP_SYMBOL1);
    T *x = _t_newr(g,SEMTREX_ZERO_OR_MORE);
    newvl(x,1,3,_t_newi(0,ASCII_CHAR,'/'),_t_newi(0,ASCII_CHAR,'?'),_t_newi(0,ASCII_CHAR,' '));

    T *r;
    //G_debug_match = 1;
    spec_is_true(_t_matchr(s,t,&r));

    spec_is_str_equal(t2s(r),"(SEMTREX_MATCH:1 (SEMTREX_MATCH_SYMBOL:TEST_GROUP_SYMBOL1) (SEMTREX_MATCHED_PATH:/1) (SEMTREX_MATCH_SIBLINGS_COUNT:2))");

    _t_free(r);
    _t_free(t);
    _t_free(s);

}

void testMatchDescend() {
    T *t = _makeTestTree1();

    //  //sy1
    T *s = _t_new_root(SEMTREX_DESCEND);
    _sl(s,sy1);
    spec_is_true(_t_match(s,t));
    _t_free(s);

    //  //sy11    <- shouldn't match
    s = _t_new_root(SEMTREX_DESCEND);
    _sl(s,sy11);
    spec_is_true(!_t_match(s,t));
    _t_free(s);

    // ///sy11
    s = _t_new_root(SEMTREX_DESCEND);
    T *ss = _t_newr(s,SEMTREX_DESCEND);
    _sl(ss,sy11);
    spec_is_true(_t_match(s,t));
    _t_free(s);


    _t_free(t);
}

void testMatchWalk() {
    T *t = _makeTestTree1();

    // search for a node down the left branch
    //  /%sy111
    T *s = _t_new_root(SEMTREX_WALK);

    _sl(s,sy111);
    spec_is_true(_t_match(s,t));
    _t_free(s);

    // search for a node that doesn't exist
    //  /%TEST_INT_SYMBOL
    s = _t_new_root(SEMTREX_WALK);
    _sl(s,TEST_INT_SYMBOL);
    spec_is_true(!_t_match(s,t));
    _t_free(s);

    // search for a node down a right branch
    //  /%sy22
    s = _t_new_root(SEMTREX_WALK);
    _sl(s,sy22);
    spec_is_true(_t_match(s,t));
    _t_free(s);

    // search for a sequence
    //  %(sy3,sy4)
    s = _t_new_root(SEMTREX_WALK);
    T *g = _t_news(s,SEMTREX_GROUP,TEST_GROUP_SYMBOL1);
    T *sq = _t_newr(g,SEMTREX_SEQUENCE);
    _sl(sq,sy3);
    _sl(sq,sy4);
    T *results;
    spec_is_true(_t_matchr(s,t,&results));
    spec_is_str_equal(t2s(results),"(SEMTREX_MATCH:1 (SEMTREX_MATCH_SYMBOL:TEST_GROUP_SYMBOL1) (SEMTREX_MATCHED_PATH:/3) (SEMTREX_MATCH_SIBLINGS_COUNT:2))");

    _t_free(results);
    _t_free(s);

    _t_free(t);

}

void testMatchNot() {
    T *t = _makeTestTree1();

    //  /TEST_STR_SYMBOL/~sy2
    T *s = _sl(0,TEST_STR_SYMBOL);
    T *n = _t_newr(s,SEMTREX_NOT);
    n = _sl(n,sy2);
    spec_is_true(_t_match(s,t));
    _t_free(s);

    //  /TEST_STR_SYMBOL/~sy1
    s = _sl(0,TEST_STR_SYMBOL);
    n = _t_newr(s,SEMTREX_NOT);
    n = _sl(n,sy1);
    spec_is_true(!_t_match(s,t));
    _t_free(s);
    _t_free(t);

    sX(R,TREE);
    sX(A,NULL_STRUCTURE);
    sX(B,NULL_STRUCTURE);
    sX(C,NULL_STRUCTURE);
    sX(G,NULL_STRUCTURE);

    t = _t_new_root(R);
    _t_newr(t,A);
    _t_newr(t,B);
    _t_newr(t,A);
    _t_newr(t,B);
    _t_newr(t,B);
    _t_newr(t,C);

    char buf[2000];
    Defs d = {0,0,0,0};
    T *g,*x,*results;

    s = _t_newr(0,SEMTREX_WALK);
    g = _t_news(s,SEMTREX_GROUP,G);
    x = _t_newr(g,SEMTREX_SEQUENCE);
    //   x = _t_newr(x,SEMTREX_NOT);
    //_sl(x,A);

    _sl(x,B);
    n = _t_newr(x,SEMTREX_NOT);
    _sl(n,A);
    n = _t_newr(x,SEMTREX_NOT);
    _sl(n,A);
    spec_is_str_equal(_dump_semtrex(&d,s,buf),"");

    stx_dump(s);

    //G_debug_match = 1;
    spec_is_true(_t_matchr(s,t,&results));
    //G_debug_match = 0;
    spec_is_str_equal(t2s(results),"");
    int *p = _t_get_path(G_ts);
    puts("MATCHPATH:");
    puts(_t_sprint_path(p,buf));

    _t_free(results);
    free(p);

/*    __t_dump(0,t,0,buf);
    puts("ABABBA:");
    puts(buf);
*/
    _t_free(s);
    _t_free(t);

}

void testSemtrexDump() {
    T *s = _makeTestSemtrex1();
    char buf[2000];
    Defs d = {0,0,0,0};

    spec_is_str_equal(_dump_semtrex(&d,s,buf),"/TEST_STR_SYMBOL/(sy1/sy11/sy111,sy2,sy3)");
    _t_free(s);

    // /TEST_STR_SYMBOL/<.*,<.>>,4  <- a more complicated group semtrex
    s = _sl(0,TEST_STR_SYMBOL);
    T *ss = _t_newr(s,SEMTREX_SEQUENCE);
    T *sg = _t_news(ss,SEMTREX_GROUP,TEST_GROUP_SYMBOL1);
    T *ss2 = _t_newr(sg,SEMTREX_SEQUENCE);
    T *st = _t_newr(ss2,SEMTREX_ZERO_OR_MORE);
    _t_newr(st,SEMTREX_SYMBOL_ANY);
    T *sg2 = _t_news(ss2,SEMTREX_GROUP,TEST_GROUP_SYMBOL2);
    _t_newr(sg2,SEMTREX_SYMBOL_ANY);
    T *s3 = _sl(ss,sy4);
    spec_is_str_equal(_dump_semtrex(&d,s,buf),"/TEST_STR_SYMBOL/(<TEST_GROUP_SYMBOL1:.*,<TEST_GROUP_SYMBOL2:.>>,sy4)");
    _t_free(s);

    // /TEST_STR_SYMBOL|TEST_INT_SYMBOL
    s = _t_new_root(SEMTREX_OR);
    _sl(s,TEST_STR_SYMBOL);
    _sl(s,TEST_INT_SYMBOL);
    spec_is_str_equal(_dump_semtrex(&d,s,buf),"/TEST_STR_SYMBOL|TEST_INT_SYMBOL");
    _t_free(s);

    s = newvl(0,1,1,_t_newi(0,ASCII_CHAR,'x'));
    spec_is_str_equal(_dump_semtrex(&d,s,buf),"/ASCII_CHAR!='x'");
    _t_free(s);

    s = newvl(0,0,1,_t_newi(0,ASCII_CHAR,'x'));
    spec_is_str_equal(_dump_semtrex(&d,s,buf),"/ASCII_CHAR='x'");
    _t_free(s);

    s = newvl(0,0,1,_t_new(0,TEST_STR_SYMBOL,"test",5));
    spec_is_str_equal(_dump_semtrex(&d,s,buf),"/TEST_STR_SYMBOL=\"test\"");
    _t_free(s);

    s = newvl(0,0,1,_t_newi(0,TEST_INT_SYMBOL,314));
    spec_is_str_equal(_dump_semtrex(&d,s,buf),"/TEST_INT_SYMBOL=314");
    _t_free(s);

    //  /TEST_STR_SYMBOL/~sy1
    s = _sl(0,TEST_STR_SYMBOL);
    T *n = _t_newr(s,SEMTREX_NOT);
    n = _sl(n,sy1);
    spec_is_str_equal(_dump_semtrex(&d,s,buf),"/TEST_STR_SYMBOL/~sy1");
    _t_free(s);

}

void testSemtrexParse() {
    Defs d = {0,0,0,0};
    char buf[5000];

    char *stx;
    T *s;
    stx = "/TEST_STR_SYMBOL/(sy1/sy11/sy111,sy2,sy3)";
    s = parseSemtrex(&d,stx);
    spec_is_str_equal(_dump_semtrex(&d,s,buf),stx);
    spec_is_str_equal(_t2s(&d,s),"(SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:TEST_STR_SYMBOL) (SEMTREX_SEQUENCE (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:sy1) (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:sy11) (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:sy111)))) (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:sy2)) (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:sy3))))");
    _t_free(s);

    stx = "/STX_STAR|STX_PLUS|STX_Q";
    s = parseSemtrex(&d,stx);
    spec_is_str_equal(_dump_semtrex(&d,s,buf),"/(STX_STAR|STX_PLUS)|STX_Q");
    spec_is_str_equal(_t2s(&d,s),"(SEMTREX_OR (SEMTREX_OR (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:STX_STAR)) (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:STX_PLUS))) (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:STX_Q)))");
    _t_free(s);

    stx = "/TEST_STR_SYMBOL/(.+,sy1,.*,sy2,.?)";
    s = parseSemtrex(&d,stx);
    spec_is_str_equal(_dump_semtrex(&d,s,buf),stx);
    spec_is_str_equal(_t2s(&d,s),"(SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:TEST_STR_SYMBOL) (SEMTREX_SEQUENCE (SEMTREX_ONE_OR_MORE (SEMTREX_SYMBOL_ANY)) (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:sy1)) (SEMTREX_ZERO_OR_MORE (SEMTREX_SYMBOL_ANY)) (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:sy2)) (SEMTREX_ZERO_OR_ONE (SEMTREX_SYMBOL_ANY))))");
    _t_free(s);

    stx = "/STX_TOKENS/(%<SEMTREX_SEQUENCE:(!STX_COMMA,STX_COMMA)+,!STX_COMMA>)";
    s = parseSemtrex(&d,stx);
    spec_is_str_equal(_dump_semtrex(&d,s,buf),stx);
    spec_is_str_equal(_t2s(&d,s),"(SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:STX_TOKENS) (SEMTREX_WALK (SEMTREX_GROUP:SEMTREX_SEQUENCE (SEMTREX_SEQUENCE (SEMTREX_ONE_OR_MORE (SEMTREX_SEQUENCE (SEMTREX_SYMBOL_LITERAL_NOT (SEMTREX_SYMBOL:STX_COMMA)) (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:STX_COMMA)))) (SEMTREX_SYMBOL_LITERAL_NOT (SEMTREX_SYMBOL:STX_COMMA))))))");
    _t_free(s);

    stx = "/HTTP_REQUEST/(.,.,HTTP_REQUEST_PATH/HTTP_REQUEST_PATH_SEGMENTS/<HTTP_REQUEST_PATH_SEGMENT:HTTP_REQUEST_PATH_SEGMENT>)";
    s = parseSemtrex(&test_HTTP_defs,stx);
    spec_is_str_equal(_dump_semtrex(&test_HTTP_defs,s,buf),stx);
    spec_is_str_equal(_t2s(&test_HTTP_defs,s),"(SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:HTTP_REQUEST) (SEMTREX_SEQUENCE (SEMTREX_SYMBOL_ANY) (SEMTREX_SYMBOL_ANY) (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:HTTP_REQUEST_PATH) (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:HTTP_REQUEST_PATH_SEGMENTS) (SEMTREX_GROUP:HTTP_REQUEST_PATH_SEGMENT (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:HTTP_REQUEST_PATH_SEGMENT)))))))");
    _t_free(s);

    stx = "/TEST_STR_SYMBOL/(TEST_INT_SYMBOL=314,ASCII_CHAR='x',TEST_STR_SYMBOL=\"abc\",TEST_STR_SYMBOL!=\"abc\")";
    s = parseSemtrex(&d,stx);
    spec_is_str_equal(_dump_semtrex(&d,s,buf),stx);
    spec_is_str_equal(_t2s(&d,s),"(SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:TEST_STR_SYMBOL) (SEMTREX_SEQUENCE (SEMTREX_VALUE_LITERAL (TEST_INT_SYMBOL:314)) (SEMTREX_VALUE_LITERAL (ASCII_CHAR:'x')) (SEMTREX_VALUE_LITERAL (TEST_STR_SYMBOL:abc)) (SEMTREX_VALUE_LITERAL_NOT (TEST_STR_SYMBOL:abc))))");
    _t_free(s);

    stx = "/TEST_STR_SYMBOL/~sy2";
    s = parseSemtrex(&d,stx);
    spec_is_str_equal(_dump_semtrex(&d,s,buf),stx);
    spec_is_str_equal(_t2s(&d,s),"(SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:TEST_STR_SYMBOL) (SEMTREX_NOT (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:sy2))))");
    _t_free(s);


    stx = "/STX_TOKENS/(.*,<STX_OP:STX_OP,<STX_SIBS:!STX_CP+>,STX_CP>)";
    s = parseSemtrex(&d,stx);
    spec_is_str_equal(_dump_semtrex(&d,s,buf),stx);
    spec_is_str_equal(_t2s(&d,s),"(SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:STX_TOKENS) (SEMTREX_SEQUENCE (SEMTREX_ZERO_OR_MORE (SEMTREX_SYMBOL_ANY)) (SEMTREX_GROUP:STX_OP (SEMTREX_SEQUENCE (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:STX_OP)) (SEMTREX_GROUP:STX_SIBS (SEMTREX_ONE_OR_MORE (SEMTREX_SYMBOL_LITERAL_NOT (SEMTREX_SYMBOL:STX_CP)))) (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:STX_CP))))))");
    _t_free(s);

    stx = "/TEST_STR_SYMBOL=\"fish\"";
    s = parseSemtrex(&d,stx);
    spec_is_str_equal(_dump_semtrex(&d,s,buf),stx);
    spec_is_str_equal(_t2s(&d,s),"(SEMTREX_VALUE_LITERAL (TEST_STR_SYMBOL:fish))");
    _t_free(s);

    stx = "/ASCII_CHARS/(ASCII_CHAR={'a','b'},ASCII_CHAR!={'a','b'})";
    s = parseSemtrex(&d,stx);
    spec_is_str_equal(_dump_semtrex(&d,s,buf),stx);
    spec_is_str_equal(_t2s(&d,s),"(SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:ASCII_CHARS) (SEMTREX_SEQUENCE (SEMTREX_VALUE_LITERAL (SEMTREX_VALUE_SET (ASCII_CHAR:'a') (ASCII_CHAR:'b'))) (SEMTREX_VALUE_LITERAL_NOT (SEMTREX_VALUE_SET (ASCII_CHAR:'a') (ASCII_CHAR:'b')))))");
    _t_free(s);

    stx = "/STX_TOKENS/(!{STX_OP,STX_CP},ASCII_CHAR!={'a','b'},{STX_OP,STX_CP})";
    s = parseSemtrex(&d,stx);
    spec_is_str_equal(_dump_semtrex(&d,s,buf),stx);
    spec_is_str_equal(_t2s(&d,s),"(SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:STX_TOKENS) (SEMTREX_SEQUENCE (SEMTREX_SYMBOL_LITERAL_NOT (SEMTREX_SYMBOL_SET (SEMTREX_SYMBOL:STX_OP) (SEMTREX_SYMBOL:STX_CP))) (SEMTREX_VALUE_LITERAL_NOT (SEMTREX_VALUE_SET (ASCII_CHAR:'a') (ASCII_CHAR:'b'))) (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL_SET (SEMTREX_SYMBOL:STX_OP) (SEMTREX_SYMBOL:STX_CP)))))");
    _t_free(s);
}

void testSemtrexParseHHTPReq() {
    char *req = "GET /path/to/file.ext?name=joe&age=30 HTTP/0.9";
    T *r,*s = makeASCIITree(req);

    char *stxs = "/ASCII_CHARS/<HTTP_REQUEST:<HTTP_REQUEST_METHOD:ASCII_CHAR!=' '+>,ASCII_CHAR=' ',<HTTP_REQUEST_PATH:<HTTP_REQUEST_PATH_SEGMENTS:(ASCII_CHAR='/',<HTTP_REQUEST_PATH_SEGMENT:ASCII_CHAR!={'/','?',' '}*>)+>>,(ASCII_CHAR='?',<HTTP_REQUEST_PATH_QUERY:<HTTP_REQUEST_PATH_QUERY_PARAMS:(<HTTP_REQUEST_PATH_QUERY_PARAM:<PARAM_KEY:ASCII_CHAR!={'&',' ','='}+>,ASCII_CHAR='=',<PARAM_VALUE:ASCII_CHAR!={'&',' '}*>>,ASCII_CHAR='&'?)+>+>)?,ASCII_CHAR=' ',ASCII_CHAR='H',ASCII_CHAR='T',ASCII_CHAR='T',ASCII_CHAR='P',ASCII_CHAR='/',<HTTP_REQUEST_VERSION:<VERSION_MAJOR:ASCII_CHAR='0'>,ASCII_CHAR='.',<VERSION_MINOR:ASCII_CHAR='9'>>>";

    char buf[5000];
    T *stx;
    stx = parseSemtrex(&test_HTTP_defs,stxs);
    //  stx = _makeHTTPRequestSemtrex();

    _dump_semtrex(&test_HTTP_defs,stx,buf);
    spec_is_str_equal(stxs,buf);

    spec_is_str_equal(buf,"/ASCII_CHARS/<HTTP_REQUEST:<HTTP_REQUEST_METHOD:ASCII_CHAR!=' '+>,ASCII_CHAR=' ',<HTTP_REQUEST_PATH:<HTTP_REQUEST_PATH_SEGMENTS:(ASCII_CHAR='/',<HTTP_REQUEST_PATH_SEGMENT:ASCII_CHAR!={'/','?',' '}*>)+>>,(ASCII_CHAR='?',<HTTP_REQUEST_PATH_QUERY:<HTTP_REQUEST_PATH_QUERY_PARAMS:(<HTTP_REQUEST_PATH_QUERY_PARAM:<PARAM_KEY:ASCII_CHAR!={'&',' ','='}+>,ASCII_CHAR='=',<PARAM_VALUE:ASCII_CHAR!={'&',' '}*>>,ASCII_CHAR='&'?)+>+>)?,ASCII_CHAR=' ',ASCII_CHAR='H',ASCII_CHAR='T',ASCII_CHAR='T',ASCII_CHAR='P',ASCII_CHAR='/',<HTTP_REQUEST_VERSION:<VERSION_MAJOR:ASCII_CHAR='0'>,ASCII_CHAR='.',<VERSION_MINOR:ASCII_CHAR='9'>>>");
    //G_d = &test_HTTP_defs;
    //G_debug_match = 1;
    spec_is_true(_t_matchr(stx,s,&r));
    spec_is_str_equal(_t2s(&test_HTTP_defs,r),"(SEMTREX_MATCH:1 (SEMTREX_MATCH_SYMBOL:HTTP_REQUEST) (SEMTREX_MATCHED_PATH:/1) (SEMTREX_MATCH_SIBLINGS_COUNT:46) (SEMTREX_MATCH:13 (SEMTREX_MATCH_SYMBOL:HTTP_REQUEST_METHOD) (SEMTREX_MATCHED_PATH:/1) (SEMTREX_MATCH_SIBLINGS_COUNT:3)) (SEMTREX_MATCH:10 (SEMTREX_MATCH_SYMBOL:HTTP_REQUEST_PATH) (SEMTREX_MATCHED_PATH:/5) (SEMTREX_MATCH_SIBLINGS_COUNT:17) (SEMTREX_MATCH:11 (SEMTREX_MATCH_SYMBOL:HTTP_REQUEST_PATH_SEGMENTS) (SEMTREX_MATCHED_PATH:/5) (SEMTREX_MATCH_SIBLINGS_COUNT:17) (SEMTREX_MATCH:12 (SEMTREX_MATCH_SYMBOL:HTTP_REQUEST_PATH_SEGMENT) (SEMTREX_MATCHED_PATH:/6) (SEMTREX_MATCH_SIBLINGS_COUNT:4)) (SEMTREX_MATCH:12 (SEMTREX_MATCH_SYMBOL:HTTP_REQUEST_PATH_SEGMENT) (SEMTREX_MATCHED_PATH:/11) (SEMTREX_MATCH_SIBLINGS_COUNT:2)) (SEMTREX_MATCH:12 (SEMTREX_MATCH_SYMBOL:HTTP_REQUEST_PATH_SEGMENT) (SEMTREX_MATCHED_PATH:/14) (SEMTREX_MATCH_SIBLINGS_COUNT:8)))) (SEMTREX_MATCH:5 (SEMTREX_MATCH_SYMBOL:HTTP_REQUEST_PATH_QUERY) (SEMTREX_MATCHED_PATH:/23) (SEMTREX_MATCH_SIBLINGS_COUNT:15) (SEMTREX_MATCH:6 (SEMTREX_MATCH_SYMBOL:HTTP_REQUEST_PATH_QUERY_PARAMS) (SEMTREX_MATCHED_PATH:/23) (SEMTREX_MATCH_SIBLINGS_COUNT:15) (SEMTREX_MATCH:7 (SEMTREX_MATCH_SYMBOL:HTTP_REQUEST_PATH_QUERY_PARAM) (SEMTREX_MATCHED_PATH:/23) (SEMTREX_MATCH_SIBLINGS_COUNT:8) (SEMTREX_MATCH:9 (SEMTREX_MATCH_SYMBOL:PARAM_KEY) (SEMTREX_MATCHED_PATH:/23) (SEMTREX_MATCH_SIBLINGS_COUNT:4)) (SEMTREX_MATCH:8 (SEMTREX_MATCH_SYMBOL:PARAM_VALUE) (SEMTREX_MATCHED_PATH:/28) (SEMTREX_MATCH_SIBLINGS_COUNT:3))) (SEMTREX_MATCH:7 (SEMTREX_MATCH_SYMBOL:HTTP_REQUEST_PATH_QUERY_PARAM) (SEMTREX_MATCHED_PATH:/32) (SEMTREX_MATCH_SIBLINGS_COUNT:6) (SEMTREX_MATCH:9 (SEMTREX_MATCH_SYMBOL:PARAM_KEY) (SEMTREX_MATCHED_PATH:/32) (SEMTREX_MATCH_SIBLINGS_COUNT:3)) (SEMTREX_MATCH:8 (SEMTREX_MATCH_SYMBOL:PARAM_VALUE) (SEMTREX_MATCHED_PATH:/36) (SEMTREX_MATCH_SIBLINGS_COUNT:2))))) (SEMTREX_MATCH:2 (SEMTREX_MATCH_SYMBOL:HTTP_REQUEST_VERSION) (SEMTREX_MATCHED_PATH:/44) (SEMTREX_MATCH_SIBLINGS_COUNT:3) (SEMTREX_MATCH:4 (SEMTREX_MATCH_SYMBOL:VERSION_MAJOR) (SEMTREX_MATCHED_PATH:/44) (SEMTREX_MATCH_SIBLINGS_COUNT:1)) (SEMTREX_MATCH:3 (SEMTREX_MATCH_SYMBOL:VERSION_MINOR) (SEMTREX_MATCHED_PATH:/46) (SEMTREX_MATCH_SIBLINGS_COUNT:1))))");

    T *t = _t_embody_from_match(&test_HTTP_defs,r,s);
    spec_is_str_equal(_t2s(&test_HTTP_defs,t),"(HTTP_REQUEST (HTTP_REQUEST_METHOD:GET) (HTTP_REQUEST_PATH (HTTP_REQUEST_PATH_SEGMENTS (HTTP_REQUEST_PATH_SEGMENT:path) (HTTP_REQUEST_PATH_SEGMENT:to) (HTTP_REQUEST_PATH_SEGMENT:file.ext))) (HTTP_REQUEST_PATH_QUERY (HTTP_REQUEST_PATH_QUERY_PARAMS (HTTP_REQUEST_PATH_QUERY_PARAM (PARAM_KEY:name) (PARAM_VALUE:joe)) (HTTP_REQUEST_PATH_QUERY_PARAM (PARAM_KEY:age) (PARAM_VALUE:30)))) (HTTP_REQUEST_VERSION (VERSION_MAJOR:0) (VERSION_MINOR:9)))");

    _t_free(r);
    T *pstx = parseSemtrex(&test_HTTP_defs,"/%HTTP_REQUEST_PATH_SEGMENTS/(HTTP_REQUEST_PATH_SEGMENT=\"path\",HTTP_REQUEST_PATH_SEGMENT=\"to\",HTTP_REQUEST_PATH_SEGMENT=\"file.ext\")");

    spec_is_true(_t_match(pstx,t));

    _t_free(pstx);
    _t_free(t);
    _t_free(s);
    _t_free(stx);
}

void testEmbodyFromMatch() {

    // test how embody from match ignores any capture that's identified as NULL_SYMBOL
    char *ts = "label:abc,123,3.1415";
    T *r,*t = makeASCIITree(ts);
    sX(LABELED_TRIPLET,LIST);
    sX(LABEL,CSTRING);
    sX(X1,CSTRING);
    sX(X2,INTEGER);
    sX(X3,FLOAT);
    Defs d = {0,0,0,0};
    T *s;
    char *stx;
    s = parseSemtrex(&d,stx = "/ASCII_CHARS/<LABELED_TRIPLET:(<LABEL:ASCII_CHAR+>,<NULL_SYMBOL:ASCII_CHAR=':'>,<X1:ASCII_CHAR+>,<NULL_SYMBOL:ASCII_CHAR=','>,<X2:ASCII_CHAR+>,<NULL_SYMBOL:ASCII_CHAR=','>,<X3:ASCII_CHAR+>)>");

    spec_is_str_equal(t2s(s),"(SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:ASCII_CHARS) (SEMTREX_GROUP:LABELED_TRIPLET (SEMTREX_SEQUENCE (SEMTREX_GROUP:LABEL (SEMTREX_ONE_OR_MORE (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:ASCII_CHAR)))) (SEMTREX_GROUP:NULL_SYMBOL (SEMTREX_VALUE_LITERAL (ASCII_CHAR:':'))) (SEMTREX_GROUP:X1 (SEMTREX_ONE_OR_MORE (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:ASCII_CHAR)))) (SEMTREX_GROUP:NULL_SYMBOL (SEMTREX_VALUE_LITERAL (ASCII_CHAR:','))) (SEMTREX_GROUP:X2 (SEMTREX_ONE_OR_MORE (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:ASCII_CHAR)))) (SEMTREX_GROUP:NULL_SYMBOL (SEMTREX_VALUE_LITERAL (ASCII_CHAR:','))) (SEMTREX_GROUP:X3 (SEMTREX_ONE_OR_MORE (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:ASCII_CHAR)))))))");

    spec_is_true(_t_matchr(s,t,&r));

    T *e = _t_embody_from_match(0,r,t);
    spec_is_str_equal(t2s(e),"(LABELED_TRIPLET (LABEL:label) (X1:abc) (X2:123) (X3:3.141500))");
    _t_free(r);
    _t_free(e);
    _t_free(s);
    _t_free(t);
}

void testSemtrex() {
    _stxSetup();
    _setup_HTTPDefs();
    testMakeFA();
    testMatchTrees();
    testSemtrexDump();
    testMatchOr();
    testMatchAny();
    testMatchExcept();
    testMatchStar();
    testMatchPlus();
    testMatchQ();
    testMatchLiteralValue();
    testMatchGroup();
    testMatchGroupMulti();
    testMatchDescend();
    testMatchWalk();
    //testMatchNot();
    testSemtrexParse();
    testSemtrexParseHHTPReq();
    testEmbodyFromMatch();
    _cleanup_HTTPDefs();
}
