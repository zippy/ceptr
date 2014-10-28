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
    T *s = _t_news(0,SEMTREX_SYMBOL_LITERAL,TEST_STR_SYMBOL);
    T *ss = _t_newi(s,SEMTREX_SEQUENCE,0);
    T *s1 = _t_news(ss,SEMTREX_SYMBOL_LITERAL,sy1);
    T *s11 = _t_news(s1,SEMTREX_SYMBOL_LITERAL,sy11);
    T *s111 = _t_news(s11,SEMTREX_SYMBOL_LITERAL,sy111);
    T *s2 = _t_news(ss,SEMTREX_SYMBOL_LITERAL,sy2);
    T *s3 = _t_news(ss,SEMTREX_SYMBOL_LITERAL,sy3);
    return s;
}

static int dump_id = 99;

void __stx_dump(SState *s) {
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
	printf("(%s:%d)",_d_get_symbol_name(0,s->data.symbol),s->transition);
	break;
    case StateValue:
	printf("(%s=:%d)",_d_get_symbol_name(0,s->data.value.symbol),s->transition);
	break;
    case StateAny:
	printf("(.:%d)",s->transition);
	break;
    case StateSymbolExcept:
	printf("(!:%d)",s->transition);
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
    spec_is_symbol_equal(0,sa->data.symbol,s);	\
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
    s2 = _t_news(_t_child(s,1),SEMTREX_SYMBOL_LITERAL,sy99);

    spec_is_true(!_t_match(s,t));

    _t_free(t);
    _t_free(s);
}

T *__stxcv(T *stxx,char c);

void testMatchOr() {
    T *t = _makeTestTree1();
    T *s = _t_new_root(SEMTREX_OR);
    T *s1 = _t_news(s,SEMTREX_SYMBOL_LITERAL,sy1);
    T *s11 = _t_news(s1,SEMTREX_SYMBOL_LITERAL,sy11);
    T *s2 = _t_news(s,SEMTREX_SYMBOL_LITERAL,TEST_STR_SYMBOL);
    T *s3;

    spec_is_true(_t_match(s,t));

    Symbol sy99 = {0,0,99};
    s3 = _t_news(s2,SEMTREX_SYMBOL_LITERAL,sy99);

    spec_is_true(!_t_match(s,t));

    _t_free(t);
    _t_free(s);

    t = _t_new_root(ASCII_CHARS);
    _t_newi(t,ASCII_CHAR,'/');
    _t_newi(t,ASCII_CHAR,'X');
    _t_newi(t,ASCII_CHAR,')');

    s = _t_news(0,SEMTREX_SYMBOL_LITERAL,ASCII_CHARS);
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
    T *s1 = _t_news(ss,SEMTREX_SYMBOL_LITERAL, sy1);

    spec_is_true(_t_match(s,t));
    t->contents.symbol.id = 99;
    spec_is_true(_t_match(s,t));

    _t_free(t);
    _t_free(s);
}

void testMatchExcept() {
    T *t = _t_new(0,sy0,"t",2);
    T *t1 = _t_new(t,sy1,"t1",3);

    T *s = _t_news(0,SEMTREX_SYMBOL_EXCEPT,sy0);
    T *ss = _t_newr(s,SEMTREX_SEQUENCE);
    T *s1 = _t_news(ss,SEMTREX_SYMBOL_LITERAL,sy1);

    spec_is_true(!_t_match(s,t));
    t->contents.symbol.id++;
    spec_is_true(_t_match(s,t));

    _t_free(t);
    _t_free(s);
}

void testMatchStar() {
    // /TEST_INT_SYMBOL/1*
    T *s = _t_news(0,SEMTREX_SYMBOL_LITERAL,TEST_INT_SYMBOL);
    T *ss = _t_newr(s,SEMTREX_SEQUENCE);
    T *sss = _t_newr(ss,SEMTREX_ZERO_OR_MORE);
    T *s1 = _t_news(sss,SEMTREX_SYMBOL_LITERAL,sy1);
    T *t1, *t1x, *t1y, *s2;

    T *t = _t_new(0,TEST_INT_SYMBOL,"t",2);
    spec_is_true(_t_match(s,t));

    t1 = _t_new(t,sy1,"t1",3);
    spec_is_true(_t_match(s,t));

    t1x = _t_new(t,sy1,"t1",3);
    t1y = _t_new(t,sy2,"t2",3);
    spec_is_true(_t_match(s,t));

    // /TEST_INT_SYMBOL/1*,2
    s2 = _t_news(ss,SEMTREX_SYMBOL_LITERAL,sy2);
    spec_is_true(_t_match(s,t));
    (*(int *)_t_surface(s2)) = 3;

    spec_is_true(!_t_match(s,t));
    _t_free(t);
    _t_free(s);
}

void testMatchPlus() {
    // /TEST_INT_SYMBOL/1+
    T *s = _t_news(0,SEMTREX_SYMBOL_LITERAL,TEST_INT_SYMBOL);
    T *ss = _t_newr(s,SEMTREX_SEQUENCE);
    T *sss = _t_newr(ss,SEMTREX_ONE_OR_MORE);
    T *s1 = _t_news(sss,SEMTREX_SYMBOL_LITERAL,sy1);
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
    T *s = _t_news(0,SEMTREX_SYMBOL_LITERAL,TEST_INT_SYMBOL);
    T *ss = _t_newr(s,SEMTREX_SEQUENCE);
    T *sss = _t_newr(ss,SEMTREX_ZERO_OR_ONE);
    T *s1 = _t_news(sss,SEMTREX_SYMBOL_LITERAL,sy1);
    T *t1, *t1x, *t1y, *s2;

    T *t = _t_new(0,TEST_INT_SYMBOL,"t",2);
    spec_is_true(_t_match(s,t));

    t1 = _t_new(t,sy1,"t1",3);
    spec_is_true(_t_match(s,t));

    t1x = _t_new(t,sy1,"t1",3);
    t1y = _t_new(t,sy2,"t2",3);

    // /TEST_INT_SYMBOL/1?,2
    s2 = _t_news(ss,SEMTREX_SYMBOL_LITERAL,sy2);
    spec_is_true(!_t_match(s,t));

    _t_free(t);
    _t_free(s);
}

void testMatchGroup() {
    T *sg2, *s3, *t, *r, *p1, *p2, *p1c, *p2c;
    t = _makeTestTree1();

    // /{TEST_STR_SYMBOL}           <- the most simple group semtrex
    T *g = _t_news(0,SEMTREX_GROUP,TEST_STR_SYMBOL);
    _t_news(g,SEMTREX_SYMBOL_LITERAL,TEST_STR_SYMBOL);

    spec_is_true(_t_matchr(g,t,&r));

    // /TEST_STR_SYMBOL/{.*,{.}},sy4  <- a more complicated group semtrex
    T *s = _t_news(0,SEMTREX_SYMBOL_LITERAL,TEST_STR_SYMBOL);
    T *ss = _t_newr(s,SEMTREX_SEQUENCE);
    T *sg = _t_news(ss,SEMTREX_GROUP,TEST_GROUP_SYMBOL1);
    T *ss2 = _t_newr(sg,SEMTREX_SEQUENCE);
    T *st = _t_newr(ss2,SEMTREX_ZERO_OR_MORE);
    _t_newr(st,SEMTREX_SYMBOL_ANY);
    sg2 = _t_news(ss2,SEMTREX_GROUP,TEST_GROUP_SYMBOL2);
    _t_newr(sg2,SEMTREX_SYMBOL_ANY);
    s3 = _t_news(ss,SEMTREX_SYMBOL_LITERAL,sy4);

    _t_free(r);

    spec_is_true(_t_matchr(s,t,&r));
    spec_is_symbol_equal(0,_t_symbol(r),SEMTREX_MATCH_RESULTS);
    spec_is_equal(_t_children(r),2);

    // you should be able to find the matched group positionaly
    p1 = _t_child(r,1);

    spec_is_symbol_equal(0,_t_symbol(p1),SEMTREX_MATCH);
    spec_is_equal(_t_children(p1),3);

    p1c = _t_child(p1,1);
    spec_is_symbol_equal(0,_t_symbol(p1c),SEMTREX_MATCH_SYMBOL);
    spec_is_symbol_equal(0,*(Symbol *)_t_surface(p1c),TEST_GROUP_SYMBOL1);

    p1c = _t_child(p1,3);

    int rp1[] = {1,TREE_PATH_TERMINATOR};
    int rp2[] = {3,TREE_PATH_TERMINATOR};

    //    printf("%s\n",_td(r));
    spec_is_symbol_equal(0,_t_symbol(p1c),SEMTREX_MATCH_SIBLINGS_COUNT);
    spec_is_equal(*(int *)_t_surface(p1c),3);
    spec_is_path_equal(_t_surface(_t_child(p1,2)),rp1);


    p2 = _t_child(r,2);
    spec_is_symbol_equal(0,_t_symbol(p2),SEMTREX_MATCH);

    p2c = _t_child(p2,3);
    spec_is_symbol_equal(0,_t_symbol(p2c),SEMTREX_MATCH_SIBLINGS_COUNT);
    spec_is_equal(*(int *)_t_surface(p2c),1);
    spec_is_path_equal(_t_surface(_t_child(p2,2)),rp2);

    // you should also be able to find the matched group by uid
    spec_is_ptr_equal(_t_get_match(r,TEST_GROUP_SYMBOL1),p1);
    spec_is_ptr_equal(_t_get_match(r,TEST_GROUP_SYMBOL2),p2);
    _t_free(r);
    _t_free(s);

    // test that the correct number of siblings is returned when the match matches a
    // sequence that includes the final sibiling
    // /TEST_STR_SYMBOL/.*,{TEST_GROUP_SYMBOL:sy3,sy4}
    s = _t_news(0,SEMTREX_SYMBOL_LITERAL,TEST_STR_SYMBOL);
    ss = _t_newr(s,SEMTREX_SEQUENCE);
    st = _t_newr(ss,SEMTREX_ZERO_OR_MORE);
    _t_newr(st,SEMTREX_SYMBOL_ANY);
    sg = _t_news(ss,SEMTREX_GROUP,TEST_GROUP_SYMBOL1);
    ss2 = _t_newr(sg,SEMTREX_SEQUENCE);
    _t_news(ss2,SEMTREX_SYMBOL_LITERAL,sy3);
    _t_news(ss2,SEMTREX_SYMBOL_LITERAL,sy4);

    spec_is_true(_t_matchr(s,t,&r));


    char buf[1000];
    __t_dump(0,r,0,buf);
    spec_is_str_equal(buf, " (SEMTREX_MATCH_RESULTS (SEMTREX_MATCH:1 (SEMTREX_MATCH_SYMBOL:TEST_GROUP_SYMBOL1) (SEMTREX_MATCHED_PATH:/3) (SEMTREX_MATCH_SIBLINGS_COUNT:2)))");

    _t_free(r);
    _t_free(t);
    _t_free(s);
    _t_free(g);
}

#define sX(name,str) Symbol name = _d_declare_symbol(G_sys_defs.symbols,str,"" #name "",TEST_CONTEXT)

void testMatchGroupMulti() {
    sX(STX_TOKENS,LIST);
    sX(STX_OP,NULL_STRUCTURE);
    sX(STX_SL,NULL_STRUCTURE);

    T *s = _t_new_root(ASCII_CHARS);
    _t_newi(s,ASCII_CHAR,'/');
    _t_newi(s,ASCII_CHAR,'(');
    _t_newi(s,ASCII_CHAR,'(');

    T *ts = _t_news(0,SEMTREX_GROUP,STX_TOKENS);
    T *g = _t_news(ts,SEMTREX_SYMBOL_LITERAL,ASCII_CHARS);
    T *sq = _t_newr(g,SEMTREX_SEQUENCE);
    T *p = _t_newr(sq,SEMTREX_ONE_OR_MORE);
    T *o = _t_newr(p,SEMTREX_OR);
    T *t = _t_news(o,SEMTREX_GROUP,STX_SL);
    __stxcv(t,'/');
    t = _t_news(o,SEMTREX_GROUP,STX_OP);
    __stxcv(t,'(');

    T *results;
    _t_matchr(ts,s,&results);
    char buf[10000];
    __t_dump(0,results,0,buf);
    spec_is_str_equal(buf, " (SEMTREX_MATCH_RESULTS (SEMTREX_MATCH:1 (SEMTREX_MATCH_SYMBOL:STX_TOKENS) (SEMTREX_MATCHED_PATH:) (SEMTREX_MATCH_SIBLINGS_COUNT:1)) (SEMTREX_MATCH:3 (SEMTREX_MATCH_SYMBOL:STX_OP) (SEMTREX_MATCHED_PATH:/3) (SEMTREX_MATCH_SIBLINGS_COUNT:1)) (SEMTREX_MATCH:3 (SEMTREX_MATCH_SYMBOL:STX_OP) (SEMTREX_MATCHED_PATH:/2) (SEMTREX_MATCH_SIBLINGS_COUNT:1)) (SEMTREX_MATCH:2 (SEMTREX_MATCH_SYMBOL:STX_SL) (SEMTREX_MATCHED_PATH:/1) (SEMTREX_MATCH_SIBLINGS_COUNT:1)))");

    _t_free(s);
    _t_free(results);
    _t_free(ts);
}

void testMatchLiteralValue() {
    T *t = _makeTestTree1();
    T *s;
    Svalue sv;
    sv.flags = SEMTREX_VALUE_NOT_FLAG;
    sv.count = 1;
    sv.symbol = TEST_STR_SYMBOL;
    sv.length = 2;
    ((char *)&sv.value)[0] = 't';
    ((char *)&sv.value)[1] = 0;				// string terminator

    // /TEST_INT_SYMBOL!="t"
    s = _t_new(0,SEMTREX_VALUE_LITERAL,&sv,sizeof(Svalue));
    spec_is_true(!_t_match(s,t));
    _t_free(s);

    // /TEST_INT_SYMBOL="t"
    sv.flags = 0;
    s = _t_new(0,SEMTREX_VALUE_LITERAL,&sv,sizeof(Svalue));
    spec_is_true(_t_match(s,t));
    _t_free(s);

    // /TEST_INT_SYMBOL2="t"
    // don't match on wrong symbol
    sv.symbol = TEST_INT_SYMBOL2;
    s = _t_new(0,SEMTREX_VALUE_LITERAL,&sv,sizeof(Svalue));
    spec_is_true(!_t_match(s,t));
    _t_free(s);
    sv.symbol = TEST_INT_SYMBOL; // restore correct symbol

    // /TEST_INT_SYMBOL="x"
    // don't match if value is wrong
    ((char *)&sv.value)[0] = 'x';
    s = _t_new(0,SEMTREX_VALUE_LITERAL,&sv,sizeof(Svalue));
    spec_is_true(!_t_match(s,t));
    _t_free(s);

    // /TEST_INT_SYMBOL=""
    // don't match if value length is wrong
    ((char *)&sv.value)[0] = 0;
    sv.length = 1;			// length of data is 1, string length is 0.
    s = _t_new(0,SEMTREX_VALUE_LITERAL,&sv,sizeof(Svalue));
    spec_is_true(!_t_match(s,t));
    _t_free(s);

    // /TEST_INT_SYMBOL=["t","q"]
    size_t svl = sizeof(Svalue) + sizeof(size_t) + 5;
    Svalue *svP = malloc(svl);
    svP->flags = 0;
    svP->count = 2;
    svP->symbol = TEST_STR_SYMBOL;
    size_t *l = &svP->length;
    char *m,*m1;
    *l = 2; m = (char *)(l+1); *m++ = 'q'; *m++ = 0;
    l = (size_t *)m;
    *l = 2; m1= m = (char *)(l+1); *m++ = 't'; *m++ = 0;
    s = _t_new(0,SEMTREX_VALUE_LITERAL,svP,svl);
    spec_is_true(_t_match(s,t));
    _t_free(s);

    // /TEST_INT_SYMBOL=["q","x"]
    *m1 = 'x';
    s = _t_new(0,SEMTREX_VALUE_LITERAL,svP,svl);
    spec_is_true(!_t_match(s,t));
    _t_free(s);

    // /TEST_INT_SYMBOL!=["q","x"]
    svP->flags = SEMTREX_VALUE_NOT_FLAG;
    s = _t_new(0,SEMTREX_VALUE_LITERAL,svP,svl);
    spec_is_true(_t_match(s,t));
    _t_free(s);

    // /TEST_INT_SYMBOL!=["q","t"]
    *m1 = 't';
    s = _t_new(0,SEMTREX_VALUE_LITERAL,svP,svl);
    spec_is_true(!_t_match(s,t));
    _t_free(s);

    free(svP);

    _t_free(t);
}

void testMatchDescend() {
    T *t = _makeTestTree1();

    //  //sy1
    T *s = _t_new_root(SEMTREX_DESCEND);
    _t_news(s,SEMTREX_SYMBOL_LITERAL,sy1);
    spec_is_true(_t_match(s,t));
    _t_free(s);

    //  //sy11    <- shouldn't match
    s = _t_new_root(SEMTREX_DESCEND);
    _t_news(s,SEMTREX_SYMBOL_LITERAL,sy11);
    spec_is_true(!_t_match(s,t));
    _t_free(s);

    // ///sy11
    s = _t_new_root(SEMTREX_DESCEND);
    T *ss = _t_newr(s,SEMTREX_DESCEND);
    _t_news(ss,SEMTREX_SYMBOL_LITERAL,sy11);
    spec_is_true(_t_match(s,t));
    _t_free(s);


    _t_free(t);
}

void testMatchWalk() {
    T *t = _makeTestTree1();

    // search for a node down the left branch
    //  /%sy111
    T *s = _t_new_root(SEMTREX_WALK);
    _t_news(s,SEMTREX_SYMBOL_LITERAL,sy111);
    spec_is_true(_t_match(s,t));
    _t_free(s);

    // search for a node that doesn't exist
    //  /%TEST_INT_SYMBOL
    s = _t_new_root(SEMTREX_WALK);
    _t_news(s,SEMTREX_SYMBOL_LITERAL,TEST_INT_SYMBOL);
    spec_is_true(!_t_match(s,t));
    _t_free(s);

    // search for a node down a right branch
    //  /%sy22
    s = _t_new_root(SEMTREX_WALK);
    _t_news(s,SEMTREX_SYMBOL_LITERAL,sy22);
    spec_is_true(_t_match(s,t));
    _t_free(s);

    // search for a sequence
    //  %(sy3,sy4)
    s = _t_new_root(SEMTREX_WALK);
    T *g = _t_news(s,SEMTREX_GROUP,TEST_GROUP_SYMBOL1);
    T *sq = _t_newr(g,SEMTREX_SEQUENCE);
    _t_news(sq,SEMTREX_SYMBOL_LITERAL,sy3);
    _t_news(sq,SEMTREX_SYMBOL_LITERAL,sy4);
    T *results;
    spec_is_true(_t_matchr(s,t,&results));
    char buf[1000];
    spec_is_str_equal(__t_dump(0,results,0,buf)," (SEMTREX_MATCH_RESULTS (SEMTREX_MATCH:1 (SEMTREX_MATCH_SYMBOL:TEST_GROUP_SYMBOL1) (SEMTREX_MATCHED_PATH:/3) (SEMTREX_MATCH_SIBLINGS_COUNT:2)))");
    _t_free(results);
    _t_free(s);

    _t_free(t);

}

void testMatchNot() {
    T *t = _makeTestTree1();

    //  /TEST_STR_SYMBOL/~sy2
    T *s = _t_news(0,SEMTREX_SYMBOL_LITERAL,TEST_STR_SYMBOL);
    T *n = _t_newr(s,SEMTREX_NOT);
    n = _t_news(n,SEMTREX_SYMBOL_LITERAL,sy2);
    spec_is_true(_t_match(s,t));
    _t_free(s);

    //  /TEST_STR_SYMBOL/~sy1
    s = _t_news(0,SEMTREX_SYMBOL_LITERAL,TEST_STR_SYMBOL);
    n = _t_newr(s,SEMTREX_NOT);
    n = _t_news(n,SEMTREX_SYMBOL_LITERAL,sy1);
    spec_is_true(!_t_match(s,t));
    _t_free(s);

    _t_free(t);

}

void testSemtrexDump() {
    T *s = _makeTestSemtrex1();
    char buf[2000];
    Defs d = {0,0,0,0};

    spec_is_str_equal(_dump_semtrex(d,s,buf),"/TEST_STR_SYMBOL/(sy1/sy11/sy111,sy2,sy3)");
    _t_free(s);

    // /TEST_STR_SYMBOL/{.*,{.}},4  <- a more complicated group semtrex
    s = _t_news(0,SEMTREX_SYMBOL_LITERAL,TEST_STR_SYMBOL);
    T *ss = _t_newr(s,SEMTREX_SEQUENCE);
    T *sg = _t_news(ss,SEMTREX_GROUP,TEST_GROUP_SYMBOL1);
    T *ss2 = _t_newr(sg,SEMTREX_SEQUENCE);
    T *st = _t_newr(ss2,SEMTREX_ZERO_OR_MORE);
    _t_newr(st,SEMTREX_SYMBOL_ANY);
    T *sg2 = _t_news(ss2,SEMTREX_GROUP,TEST_GROUP_SYMBOL2);
    _t_newr(sg2,SEMTREX_SYMBOL_ANY);
    T *s3 = _t_news(ss,SEMTREX_SYMBOL_LITERAL,sy4);
    spec_is_str_equal(_dump_semtrex(d,s,buf),"/TEST_STR_SYMBOL/({TEST_GROUP_SYMBOL1:.*,{TEST_GROUP_SYMBOL2:.}},sy4)");
    _t_free(s);

    // /TEST_STR_SYMBOL|TEST_INT_SYMBOL
    s = _t_new_root(SEMTREX_OR);
    _t_news(s,SEMTREX_SYMBOL_LITERAL,TEST_STR_SYMBOL);
    _t_news(s,SEMTREX_SYMBOL_LITERAL,TEST_INT_SYMBOL);
    spec_is_str_equal(_dump_semtrex(d,s,buf),"/TEST_STR_SYMBOL|TEST_INT_SYMBOL");
    _t_free(s);

    Svalue sv;
    sv.flags = SEMTREX_VALUE_NOT_FLAG;
    sv.count = 1;
    sv.symbol = ASCII_CHAR;
    sv.length = sizeof(int);
    *(int *)&sv.value = 'x';
    s = _t_new(0,SEMTREX_VALUE_LITERAL,&sv,sizeof(Svalue));
    spec_is_str_equal(_dump_semtrex(d,s,buf),"/ASCII_CHAR!='x'");
    _t_free(s);

    sv.flags = 0;
    s = _t_new(0,SEMTREX_VALUE_LITERAL,&sv,sizeof(Svalue));
    spec_is_str_equal(_dump_semtrex(d,s,buf),"/ASCII_CHAR='x'");
    _t_free(s);

    sv.symbol = TEST_STR_SYMBOL;
    sv.length = 2;
    *((char *)&sv.value) = 'x';
    *(1+(char *)&sv.value) = 'y';
    s = _t_new(0,SEMTREX_VALUE_LITERAL,&sv,sizeof(Svalue));
    spec_is_str_equal(_dump_semtrex(d,s,buf),"/TEST_STR_SYMBOL=\"xy\"");
    _t_free(s);

    sv.symbol = TEST_INT_SYMBOL;
    sv.length = 2;
    *(int *)&sv.value = 314;
    s = _t_new(0,SEMTREX_VALUE_LITERAL,&sv,sizeof(Svalue));
    spec_is_str_equal(_dump_semtrex(d,s,buf),"/TEST_INT_SYMBOL=314");
    _t_free(s);


    //  /TEST_STR_SYMBOL/~sy1
    s = _t_news(0,SEMTREX_SYMBOL_LITERAL,TEST_STR_SYMBOL);
    T *n = _t_newr(s,SEMTREX_NOT);
    n = _t_news(n,SEMTREX_SYMBOL_LITERAL,sy1);
    spec_is_str_equal(_dump_semtrex(d,s,buf),"/TEST_STR_SYMBOL/~sy1");
    _t_free(s);

}

void testSemtrexParse() {
    Defs d = {0,0,0,0};
    char buf[5000];

    char *stx;
    T *s;
    stx = "/TEST_STR_SYMBOL/(sy1/sy11/sy111,sy2,sy3)";
    s = parseSemtrex(&d,stx);
    spec_is_str_equal(_dump_semtrex(d,s,buf),stx);
    spec_is_str_equal(__t_dump(&d,s,0,buf)," (SEMTREX_SYMBOL_LITERAL:TEST_STR_SYMBOL (SEMTREX_SEQUENCE (SEMTREX_SYMBOL_LITERAL:sy1 (SEMTREX_SYMBOL_LITERAL:sy11 (SEMTREX_SYMBOL_LITERAL:sy111))) (SEMTREX_SYMBOL_LITERAL:sy2) (SEMTREX_SYMBOL_LITERAL:sy3)))");
    _t_free(s);

    stx = "/STX_STAR|STX_PLUS|STX_Q";
    s = parseSemtrex(&d,stx);
    spec_is_str_equal(_dump_semtrex(d,s,buf),"/(STX_STAR|STX_PLUS)|STX_Q");
    spec_is_str_equal(__t_dump(&d,s,0,buf)," (SEMTREX_OR (SEMTREX_OR (SEMTREX_SYMBOL_LITERAL:STX_STAR) (SEMTREX_SYMBOL_LITERAL:STX_PLUS)) (SEMTREX_SYMBOL_LITERAL:STX_Q))");
    _t_free(s);

    stx = "/TEST_STR_SYMBOL/(.+,sy1,.*,sy2,.?)";
    s = parseSemtrex(&d,stx);
    spec_is_str_equal(_dump_semtrex(d,s,buf),stx);
    spec_is_str_equal(__t_dump(&d,s,0,buf)," (SEMTREX_SYMBOL_LITERAL:TEST_STR_SYMBOL (SEMTREX_SEQUENCE (SEMTREX_ONE_OR_MORE (SEMTREX_SYMBOL_ANY)) (SEMTREX_SYMBOL_LITERAL:sy1) (SEMTREX_ZERO_OR_MORE (SEMTREX_SYMBOL_ANY)) (SEMTREX_SYMBOL_LITERAL:sy2) (SEMTREX_ZERO_OR_ONE (SEMTREX_SYMBOL_ANY))))");
    _t_free(s);

    stx = "/STX_TOKENS/%{SEMTREX_SEQUENCE:(!STX_COMMA,STX_COMMA)+,!STX_COMMA}";
    s = parseSemtrex(&d,stx);
    spec_is_str_equal(_dump_semtrex(d,s,buf),stx);
    spec_is_str_equal(__t_dump(&d,s,0,buf)," (SEMTREX_SYMBOL_LITERAL:STX_TOKENS (SEMTREX_WALK (SEMTREX_GROUP:SEMTREX_SEQUENCE (SEMTREX_SEQUENCE (SEMTREX_ONE_OR_MORE (SEMTREX_SEQUENCE (SEMTREX_SYMBOL_EXCEPT:STX_COMMA) (SEMTREX_SYMBOL_LITERAL:STX_COMMA))) (SEMTREX_SYMBOL_EXCEPT:STX_COMMA)))))");
    _t_free(s);

    stx = "/HTTP_REQUEST/(.,.,HTTP_REQUEST_PATH/HTTP_REQUEST_PATH_SEGMENTS/{HTTP_REQUEST_PATH_SEGMENT:HTTP_REQUEST_PATH_SEGMENT})";
    s = parseSemtrex(&test_HTTP_defs,stx);
    spec_is_str_equal(_dump_semtrex(test_HTTP_defs,s,buf),stx);
    spec_is_str_equal(__t_dump(&test_HTTP_defs,s,0,buf)," (SEMTREX_SYMBOL_LITERAL:HTTP_REQUEST (SEMTREX_SEQUENCE (SEMTREX_SYMBOL_ANY) (SEMTREX_SYMBOL_ANY) (SEMTREX_SYMBOL_LITERAL:HTTP_REQUEST_PATH (SEMTREX_SYMBOL_LITERAL:HTTP_REQUEST_PATH_SEGMENTS (SEMTREX_GROUP:HTTP_REQUEST_PATH_SEGMENT (SEMTREX_SYMBOL_LITERAL:HTTP_REQUEST_PATH_SEGMENT))))))");
    _t_free(s);

    stx = "/TEST_STR_SYMBOL/(TEST_INT_SYMBOL=314,ASCII_CHAR='x',TEST_STR_SYMBOL=\"abc\")";
    s = parseSemtrex(&d,stx);
    spec_is_str_equal(_dump_semtrex(d,s,buf),stx);
    spec_is_str_equal(__t_dump(&d,s,0,buf)," (SEMTREX_SYMBOL_LITERAL:TEST_STR_SYMBOL (SEMTREX_SEQUENCE (SEMTREX_VALUE_LITERAL:TEST_INT_SYMBOL) (SEMTREX_VALUE_LITERAL:ASCII_CHAR) (SEMTREX_VALUE_LITERAL:TEST_STR_SYMBOL)))");
    _t_free(s);

    stx = "/TEST_STR_SYMBOL/~sy2";
    s = parseSemtrex(&d,stx);
    spec_is_str_equal(_dump_semtrex(d,s,buf),stx);
    spec_is_str_equal(__t_dump(&d,s,0,buf)," (SEMTREX_SYMBOL_LITERAL:TEST_STR_SYMBOL (SEMTREX_NOT (SEMTREX_SYMBOL_LITERAL:sy2)))");
    _t_free(s);


}

T *__stxcv(T *stxx,char c);
T *__stxcvm(T *stxx,int not,int count,...);

void testSemtrexParseHHTPReq() {
    char *req = "GET /path/file.ext?name=joe&age=30 HTTP/0.9";
    T *t,*s = _t_new_root(ASCII_CHARS);
    while(*req) {
	_t_newi(s,ASCII_CHAR,*req);
	req++;
    }

    char *stxs = "/{HTTP_REQUEST:ASCII_CHARS/({HTTP_REQUEST_METHOD:.*},ASCII_CHAR=' '+,{HTTP_REQUEST_PATH:{HTTP_REQUEST_PATH_SEGMENTS:{HTTP_REQUEST_PATH_SEGMENT:ASCII_CHAR='/',~(ASCII_CHAR='/'|ASCII_CHAR='?'|ASCII_CHAR=' ')*}+}},{HTTP_REQUEST_PATH_FILE:{FILE_NAME:~(ASCII_CHAR='?'|ASCII_CHAR=' ')+},(ASCII_CHAR='.',{FILE_EXTENSION:~(ASCII_CHAR='?'|ASCII_CHAR=' ')+})?},(ASCII_CHAR='?',{HTTP_REQUEST_PATH_QUERY:{HTTP_PATH_QUERY_PARAMS:{PARAM_KEY:~(ASCII_CHAR='='|ASCII_CHAR='&'|ASCII_CHAR=' ')},ASCII_CHAR='=',{PARAM_VALUE:~(ASCII_CHAR='&'|ASCII_CHAR=' ')*}}+})?,ASCII_CHAR=' '+,ASCII_CHAR='H',ASCII_CHAR='T',ASCII_CHAR='T',ASCII_CHAR='P',ASCII_CHAR='/',{HTTP_REQUEST_VERSION:{VERSION_MAJOR:ASCII_CHAR='0'},ASCII_CHAR='.',{VERSION_MINOR:(ASCII_CHAR='9')+}})}";

    char buf[5000];

    T *stx= _t_news(0,SEMTREX_SYMBOL_LITERAL,ASCII_CHARS);
    t = _t_news(stx,SEMTREX_GROUP,HTTP_REQUEST);
    T *sq = _t_newr(t,SEMTREX_SEQUENCE);
    t = _t_news(sq,SEMTREX_GROUP,HTTP_REQUEST_METHOD);
    t = _t_newr(t,SEMTREX_ONE_OR_MORE);
    _t_newr(t,SEMTREX_SYMBOL_ANY);

    t = _t_news(sq,SEMTREX_GROUP,HTTP_REQUEST_PATH);
    T *sqq = _t_newr(t,SEMTREX_SEQUENCE);
    __stxcv(sqq,' ');

    t = _t_news(sqq,SEMTREX_GROUP,HTTP_REQUEST_PATH_SEGMENTS);
    t = _t_newr(t,SEMTREX_ONE_OR_MORE);
    t = _t_newr(t,SEMTREX_SEQUENCE);
    __stxcv(t,'/');
    t = _t_news(t,SEMTREX_GROUP,HTTP_REQUEST_PATH_SEGMENT);
    t = _t_newr(t,SEMTREX_ZERO_OR_MORE);
    __stxcvm(t,1,3,'/','?',' ');

    t = _t_news(sq,SEMTREX_GROUP,HTTP_REQUEST_PATH_FILE);
    T *f = _t_newr(t,SEMTREX_SEQUENCE);
    t = _t_news(f,SEMTREX_GROUP,FILE_NAME);
    t = _t_newr(t,SEMTREX_ONE_OR_MORE);
    __stxcvm(t,1,2,'?',' ');

    t = _t_newr(f,SEMTREX_ZERO_OR_ONE);
    t = _t_newr(t,SEMTREX_SEQUENCE);
    __stxcv(t,'.');
    t = _t_news(t,SEMTREX_GROUP,FILE_EXTENSION);
    t = _t_newr(t,SEMTREX_ONE_OR_MORE);
    __stxcvm(t,1,2,'?',' ');

    t = _t_newr(sq,SEMTREX_ZERO_OR_ONE);
    t = _t_newr(t,SEMTREX_SEQUENCE);
    __stxcv(t,'?');
    t = _t_news(t,SEMTREX_GROUP,HTTP_REQUEST_PATH_QUERY);
    t = _t_newr(t,SEMTREX_ONE_OR_MORE);
    t = _t_news(t,SEMTREX_GROUP,HTTP_REQUEST_PATH_QUERY_PARAMS);
    f = _t_newr(t,SEMTREX_SEQUENCE);
    t = _t_news(f,SEMTREX_GROUP,PARAM_KEY);
    t = _t_newr(t,SEMTREX_ONE_OR_MORE);
    __stxcvm(t,1,3,'&',' ','=');

    t = _t_news(f,SEMTREX_GROUP,PARAM_VALUE);
    t = _t_newr(t,SEMTREX_ZERO_OR_MORE);
    __stxcvm(t,1,2,'&',' ');

    __stxcv(sq,' ');
    __stxcv(sq,'H');
    __stxcv(sq,'T');
    __stxcv(sq,'T');
    __stxcv(sq,'P');
    __stxcv(sq,'/');
    t = _t_news(sq,SEMTREX_GROUP,HTTP_REQUEST_VERSION);
    t = _t_newr(t,SEMTREX_SEQUENCE);
    f = _t_news(t,SEMTREX_GROUP,VERSION_MAJOR);
    __stxcv(f,'0');
    __stxcv(t,'.');
    f = _t_news(t,SEMTREX_GROUP,VERSION_MINOR);
    __stxcv(f,'9');

    __dump_semtrex(test_HTTP_defs,stx,buf);
    spec_is_str_equal(buf,"ASCII_CHARS/{HTTP_REQUEST:{HTTP_REQUEST_METHOD:.+},{HTTP_REQUEST_PATH:ASCII_CHAR=' ',{HTTP_REQUEST_PATH_SEGMENTS:(ASCII_CHAR='/',{HTTP_REQUEST_PATH_SEGMENT:ASCII_CHAR!=['/','?',' ']*})+}},{HTTP_REQUEST_PATH_FILE:{FILE_NAME:ASCII_CHAR!=['?',' ']+},(ASCII_CHAR='.',{FILE_EXTENSION:ASCII_CHAR!=['?',' ']+})?},(ASCII_CHAR='?',{HTTP_REQUEST_PATH_QUERY:{HTTP_REQUEST_PATH_QUERY_PARAMS:{PARAM_KEY:ASCII_CHAR!=['&',' ','=']+},{PARAM_VALUE:ASCII_CHAR!=['&',' ']*}}+})?,ASCII_CHAR=' ',ASCII_CHAR='H',ASCII_CHAR='T',ASCII_CHAR='T',ASCII_CHAR='P',ASCII_CHAR='/',{HTTP_REQUEST_VERSION:{VERSION_MAJOR:ASCII_CHAR='0'},ASCII_CHAR='.',{VERSION_MINOR:ASCII_CHAR='9'}}}");

    T *results;
    spec_is_true(_t_matchr(stx,s,&results));
    spec_is_str_equal(__t_dump(&test_HTTP_defs,results,0,buf)," (SEMTREX_MATCH_RESULTS (SEMTREX_MATCH:1 (SEMTREX_MATCH_SYMBOL:HTTP_REQUEST) (SEMTREX_MATCHED_PATH:/1) (SEMTREX_MATCH_SIBLINGS_COUNT:43)) (SEMTREX_MATCH:2 (SEMTREX_MATCH_SYMBOL:HTTP_REQUEST_VERSION) (SEMTREX_MATCHED_PATH:/41) (SEMTREX_MATCH_SIBLINGS_COUNT:3)) (SEMTREX_MATCH:3 (SEMTREX_MATCH_SYMBOL:VERSION_MINOR) (SEMTREX_MATCHED_PATH:/43) (SEMTREX_MATCH_SIBLINGS_COUNT:1)) (SEMTREX_MATCH:4 (SEMTREX_MATCH_SYMBOL:VERSION_MAJOR) (SEMTREX_MATCHED_PATH:/41) (SEMTREX_MATCH_SIBLINGS_COUNT:1)) (SEMTREX_MATCH:9 (SEMTREX_MATCH_SYMBOL:HTTP_REQUEST_PATH_FILE) (SEMTREX_MATCHED_PATH:/34) (SEMTREX_MATCH_SIBLINGS_COUNT:1)) (SEMTREX_MATCH:11 (SEMTREX_MATCH_SYMBOL:FILE_NAME) (SEMTREX_MATCHED_PATH:/34) (SEMTREX_MATCH_SIBLINGS_COUNT:1)) (SEMTREX_MATCH:12 (SEMTREX_MATCH_SYMBOL:HTTP_REQUEST_PATH) (SEMTREX_MATCHED_PATH:/4) (SEMTREX_MATCH_SIBLINGS_COUNT:30)) (SEMTREX_MATCH:13 (SEMTREX_MATCH_SYMBOL:HTTP_REQUEST_PATH_SEGMENTS) (SEMTREX_MATCHED_PATH:/5) (SEMTREX_MATCH_SIBLINGS_COUNT:29)) (SEMTREX_MATCH:14 (SEMTREX_MATCH_SYMBOL:HTTP_REQUEST_PATH_SEGMENT) (SEMTREX_MATCHED_PATH:/6) (SEMTREX_MATCH_SIBLINGS_COUNT:28)) (SEMTREX_MATCH:15 (SEMTREX_MATCH_SYMBOL:HTTP_REQUEST_METHOD) (SEMTREX_MATCHED_PATH:/1) (SEMTREX_MATCH_SIBLINGS_COUNT:3)))");

    _t_free(s);
    _t_free(stx);
}

void testSemtrex() {
    _stxSetup();
    _setup_HTTPDefs();
    testMakeFA();
    testMatchTrees();
    testMatchOr();
    testMatchAny();
    testMatchExcept();
    testMatchStar();
    testMatchPlus();
    testMatchQ();
    testMatchGroup();
    testMatchGroupMulti();
    testMatchLiteralValue();
    testMatchDescend();
    testMatchWalk();
    //    testMatchNot();
    testSemtrexDump();
    testSemtrexParse();

    testSemtrexParseHHTPReq();
    _cleanup_HTTPDefs();
}
