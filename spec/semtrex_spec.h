/**
 * @file semtrex_spec.h
 * @copyright Copyright (C) 2013-2016, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 * @ingroup tests
 */

#include "../src/ceptr.h"
#include "../src/hashfn.h"
#include "../src/semtrex.h"
#include "../src/receptor.h"
#include "http_example.h"
#include "spec_utils.h"

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

/* #define spec_state_equal(sa,st,tt,s) \ */
/*     spec_is_equal(sa->type,st);\ */
/*     spec_is_equal(sa->transition,tt);\ */
/*     spec_is_symbol_equal(0,*(Symbol *)_t_surface(sa->data.symbol.symbols),s); \ */
/*     spec_is_ptr_equal(sa->out1,NULL); */


/* void testMakeFA() { */
/*     SState *s1, *s2, *s3, *s4, *s5, *s6; */
/*     T *s = _makeTestSemtrex1(); */

/*     int states = 0; */
/*     SState *sa = _stx_makeFA(s,&states); */
/*     spec_is_equal(states,6); */

/*     spec_state_equal(sa,StateSymbol,TransitionDown,TEST_STR_SYMBOL); */

/*     s1 = sa->out; */
/*     spec_state_equal(s1,StateSymbol,TransitionDown,sy1); */

/*     s2 = s1->out; */
/*     spec_state_equal(s2,StateSymbol,TransitionDown,sy11); */

/*     s3 = s2->out; */
/*     spec_state_equal(s3,StateSymbol,-2,sy111); */

/*     s4 = s3->out; */
/*     spec_state_equal(s4,StateSymbol,TransitionNextChild,sy2); */

/*     s5 = s4->out; */
/*     spec_state_equal(s5,StateSymbol,TransitionUp,sy3); */

/*     s6 = s5->out; */
/*     spec_is_equal(s6->type,StateMatch); */

/*     spec_is_ptr_equal(s6->out,NULL); */

/*     _stx_freeFA(sa); */
/*     _t_free(s); */
/* } */

void testMatchTrees() {
    T *t = _makeTestTree1();
    T *s = _makeTestSemtrex1();
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

    s = _t_parse(G_sem,0,"(SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:ASCII_CHARS) (SEMTREX_SEQUENCE (SEMTREX_VALUE_LITERAL (ASCII_CHAR:'/')) (SEMTREX_OR (SEMTREX_SEQUENCE (SEMTREX_VALUE_LITERAL (ASCII_CHAR:'(')) (SEMTREX_VALUE_LITERAL (ASCII_CHAR:')'))) (SEMTREX_VALUE_LITERAL (ASCII_CHAR:'X')))))");

    char buf[2000];
    spec_is_str_equal(_dump_semtrex(G_sem,s,buf),"/ASCII_CHARS/(ASCII_CHAR='/',(ASCII_CHAR='(',ASCII_CHAR=')')|ASCII_CHAR='X')");
    t = _t_parse(G_sem,0,"(ASCII_CHARS (ASCII_CHAR:'/') (ASCII_CHAR:'X') (ASCII_CHAR:')')");
    spec_is_true(_t_match(s,t));
    _t_free(t);

    t = _t_parse(G_sem,0,"(ASCII_CHARS (ASCII_CHAR:'/') (ASCII_CHAR:'(') (ASCII_CHAR:')')");
    spec_is_true(_t_match(s,t));
    _t_free(t);

    t = _t_parse(G_sem,0,"(ASCII_CHARS (ASCII_CHAR:'/') (ASCII_CHAR:')')");
    spec_is_false(_t_match(s,t));
    _t_free(t);

    _t_free(s);

    s = _t_parse(G_sem,0,"(SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:A) (SEMTREX_SEQUENCE (SEMTREX_OR (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:B)) (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:C) (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:D)))) (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:E))))");
    spec_is_str_equal(_dump_semtrex(G_sem,s,buf),"/A/(B|C/D,E)");

    t = _t_parse(G_sem,0,"(A (C (D)) (E))");

    //debug_enable(D_STX_MATCH);
    spec_is_true(_t_match(s,t));
    debug_disable(D_STX_MATCH);
    _t_free(t);
    t = _t_parse(G_sem,0,"(A (B) (E))");
    spec_is_true(_t_match(s,t));
    _t_free(t);
    t = _t_parse(G_sem,0,"(A (C) (E))");
    spec_is_false(_t_match(s,t));
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
    // /SIGNALS/<SIGNALS:SIGNAL*>
    T *r;
    T *s = _sl(0,SIGNALS);
    T *g = _t_news(s,SEMTREX_GROUP,SIGNALS);
    T *ss = _t_newr(g,SEMTREX_ZERO_OR_MORE);
    _sl(ss,SIGNAL);

    T *t = _t_new_root(SIGNALS);

    spec_is_true(_t_match(s,t));
    // @todo this is actually broken, it looks like if you add the GROUP something has to match
    //spec_is_true(_t_matchr(s,t,&r));
    //spec_is_str_equal(t2s(r),"(SEMTREX_MATCH:1 (SEMTREX_MATCH_SYMBOL:SIGNALS) (SEMTREX_MATCH_PATH:/1) (SEMTREX_MATCH_SIBLINGS_COUNT:0))");
    //    _t_free(r);

    T *signal_contents = _t_newi(0,TEST_INT_SYMBOL,314);
    ReceptorAddress fr = {3}; // DUMMY ADDR
    ReceptorAddress to = {4}; // DUMMY ADDR
    T *sig = __r_make_signal(fr,to,DEFAULT_ASPECT,TESTING,signal_contents,0,0,0);
    T *sig2 = _t_clone(sig);
    _t_add(t,sig);

    spec_is_true(_t_matchr(s,t,&r));
    spec_is_str_equal(t2s(r),"(SEMTREX_MATCH:1 (SEMTREX_MATCH_SYMBOL:SIGNALS) (SEMTREX_MATCH_PATH:/1) (SEMTREX_MATCH_SIBLINGS_COUNT:1))");
    _t_free(r);

    _t_add(t,sig2);
    spec_is_true(_t_matchr(s,t,&r));
    spec_is_str_equal(t2s(r),"(SEMTREX_MATCH:1 (SEMTREX_MATCH_SYMBOL:SIGNALS) (SEMTREX_MATCH_PATH:/1) (SEMTREX_MATCH_SIBLINGS_COUNT:2))");
    _t_free(r);

    spec_is_false(_t_match(s,signal_contents));
    _t_free(t);
    _t_free(s);

    char buf[1000];
    s = _t_parse(G_sem,0,"(SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:A) (SEMTREX_SEQUENCE (SEMTREX_ZERO_OR_MORE (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:B))) (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:C))))");
    spec_is_str_equal(_dump_semtrex(G_sem,s,buf),"/A/(B*,C)");

    t = _t_parse(G_sem,0,"(A (B) (C))");
    spec_is_true(_t_match(s,t));
    _t_free(t);
    t = _t_parse(G_sem,0,"(A (B) (B) (B) (C))");
    spec_is_true(_t_match(s,t));
    _t_free(t);
    _t_free(s);

    s = _t_parse(G_sem,0,"(SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:A) (SEMTREX_SEQUENCE (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:B) (SEMTREX_ZERO_OR_MORE (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:C)))) (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:D))))");
    spec_is_str_equal(_dump_semtrex(G_sem,s,buf),"/A/(B/C*,D)");

    t = _t_parse(G_sem,0,"(A (B (C)) (D))");
//    debug_enable(D_STX_MATCH+D_STX_BUILD);
    spec_is_true(_t_match(s,t));
    debug_disable(D_STX_MATCH+D_STX_BUILD);
    _t_free(t);

    t = _t_parse(G_sem,0,"(A (B) (D))");
//    debug_enable(D_STX_MATCH+D_STX_BUILD);
    spec_is_true(_t_match(s,t));
    debug_disable(D_STX_MATCH+D_STX_BUILD);
    _t_free(t);


    _t_free(s);

}

void testMatchPlus() {
    // /SIGNALS/<SIGNALS:SIGNAL+>
    T *r;
    T *s = _sl(0,SIGNALS);
    T *g = _t_news(s,SEMTREX_GROUP,SIGNALS);
    T *ss = _t_newr(g,SEMTREX_ONE_OR_MORE);
    _sl(ss,SIGNAL);

    T *t = _t_new_root(SIGNALS);

    spec_is_false(_t_matchr(s,t,&r));

    T *signal_contents = _t_newi(0,TEST_INT_SYMBOL,314);
    ReceptorAddress fr = {3}; // DUMMY ADDR
    ReceptorAddress to = {4}; // DUMMY ADDR
    T *sig = __r_make_signal(fr,to,DEFAULT_ASPECT,TESTING,signal_contents,0,0,0);
    T *sig2 = _t_clone(sig);
    _t_add(t,sig);
    spec_is_true(_t_matchr(s,t,&r));
    spec_is_str_equal(t2s(r),"(SEMTREX_MATCH:1 (SEMTREX_MATCH_SYMBOL:SIGNALS) (SEMTREX_MATCH_PATH:/1) (SEMTREX_MATCH_SIBLINGS_COUNT:1))");
    _t_free(r);

    _t_add(t,sig2);
    spec_is_true(_t_matchr(s,t,&r));
    spec_is_str_equal(t2s(r),"(SEMTREX_MATCH:1 (SEMTREX_MATCH_SYMBOL:SIGNALS) (SEMTREX_MATCH_PATH:/1) (SEMTREX_MATCH_SIBLINGS_COUNT:2))");
    _t_free(r);

    spec_is_false(_t_match(s,signal_contents));
    _t_free(t);
    _t_free(s);
}

void testMatchQ() {
    // /SIGNALS/<SIGNALS:SIGNAL?>
    T *r;
    T *s = _sl(0,SIGNALS);
    T *g = _t_news(s,SEMTREX_GROUP,SIGNALS);
    T *ss = _t_newr(g,SEMTREX_ZERO_OR_ONE);
    _sl(ss,SIGNAL);

    T *t = _t_new_root(SIGNALS);

    spec_is_true(_t_match(s,t));
    // @todo this is actually broken, it looks like if you add the GROUP something has to match
    //spec_is_true(_t_matchr(s,t,&r));
    //spec_is_str_equal(t2s(r),"(SEMTREX_MATCH:1 (SEMTREX_MATCH_SYMBOL:SIGNALS) (SEMTREX_MATCH_PATH:/1) (SEMTREX_MATCH_SIBLINGS_COUNT:0))");
    //    _t_free(r);

    T *signal_contents = _t_newi(0,TEST_INT_SYMBOL,314);
    ReceptorAddress fr = {3}; // DUMMY ADDR
    ReceptorAddress to = {4}; // DUMMY ADDR
    T *sig = __r_make_signal(fr,to,DEFAULT_ASPECT,TESTING,signal_contents,0,0,0);
    T *sig2 = _t_clone(sig);
    _t_add(t,sig);
    //debug_enable(D_STX_MATCH);
    spec_is_true(_t_matchr(s,t,&r));
    debug_disable(D_STX_MATCH);
    spec_is_str_equal(t2s(r),"(SEMTREX_MATCH:1 (SEMTREX_MATCH_SYMBOL:SIGNALS) (SEMTREX_MATCH_PATH:/1) (SEMTREX_MATCH_SIBLINGS_COUNT:1))");
    _t_free(r);

    // add a second signal and it still only matches on the first sibling
    _t_add(t,sig2);
    spec_is_true(_t_matchr(s,t,&r));
    spec_is_str_equal(t2s(r),"(SEMTREX_MATCH:1 (SEMTREX_MATCH_SYMBOL:SIGNALS) (SEMTREX_MATCH_PATH:/1) (SEMTREX_MATCH_SIBLINGS_COUNT:1))");
    _t_free(r);

    spec_is_false(_t_match(s,signal_contents));
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

    //debug_enable(D_STX_MATCH);
    spec_is_true(_t_matchr(s,t,&r));
    debug_disable(D_STX_MATCH);

    spec_is_str_equal(t2s(r),"(SEMTREX_MATCH:1 (SEMTREX_MATCH_SYMBOL:TEST_GROUP_SYMBOL1) (SEMTREX_MATCH_PATH:/1) (SEMTREX_MATCH_SIBLINGS_COUNT:3) (SEMTREX_MATCH:2 (SEMTREX_MATCH_SYMBOL:TEST_GROUP_SYMBOL2) (SEMTREX_MATCH_PATH:/3) (SEMTREX_MATCH_SIBLINGS_COUNT:1)))");

    // @todo fix this spec so it will work.  The spec uses nonsensical
    // TEST_GROUP_SYMBOL that are integers so you wouldn't actually match on them
    // for something with children...
    //    T *map = _stx_results2sem_map(G_sem,r,t);
    //spec_is_str_equal(t2s(map),"(SEMANTIC_MAP (SEMANTIC_LINK (USAGE:TEST_GROUP_SYMBOL1) (REPLACEMENT_VALUE (TEST_GROUP_SYMBOL1:16789876 (sy11:t11 (sy111:t111))))) (SEMANTIC_LINK (USAGE:TEST_GROUP_SYMBOL2) (REPLACEMENT_VALUE (TEST_GROUP_SYMBOL2:t3))))");

    // you should also be able to find the matched group by uid
    spec_is_ptr_equal(_t_get_match(r,TEST_GROUP_SYMBOL1),r);
    spec_is_ptr_equal(_t_get_match(r,TEST_GROUP_SYMBOL2),_t_child(r,4));
    _t_free(r);
    _t_free(s);

    // /TEST_STR_SYMBOL/(<TEST_GROUP_SYMBOL1:<TEST_GROUP_SYMBOL2:.>*>,sy4)  <- a more complicated group semtrex, this time where a group will be repeated
    s = _t_parse(G_sem,0,"(SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:TEST_STR_SYMBOL) (SEMTREX_SEQUENCE (SEMTREX_GROUP:TEST_GROUP_SYMBOL1 (SEMTREX_ZERO_OR_MORE (SEMTREX_GROUP:TEST_GROUP_SYMBOL2 (SEMTREX_SYMBOL_ANY)))) (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:sy4))))");
    char buf[2000];
    spec_is_str_equal(_dump_semtrex(G_sem,s,buf),"/TEST_STR_SYMBOL/(<TEST_GROUP_SYMBOL1:<TEST_GROUP_SYMBOL2:.>*>,sy4)");
    //t = _t_parse(G_sem,0,"");
    //t = (TEST_STR_SYMBOL:t (sy1:t1 (sy11:t11 (sy111:t111))) (sy2:t2 (sy21:t21) (sy22:t22)) (sy3:t3) (sy4:t4))
    //debug_enable(D_STX_MATCH+D_STX_BUILD);
    spec_is_true(_t_matchr(s,t,&r));
    debug_disable(D_STX_MATCH+D_STX_BUILD);

    spec_is_str_equal(t2s(r),"(SEMTREX_MATCH:1 (SEMTREX_MATCH_SYMBOL:TEST_GROUP_SYMBOL1) (SEMTREX_MATCH_PATH:/1) (SEMTREX_MATCH_SIBLINGS_COUNT:3) (SEMTREX_MATCH:2 (SEMTREX_MATCH_SYMBOL:TEST_GROUP_SYMBOL2) (SEMTREX_MATCH_PATH:/1) (SEMTREX_MATCH_SIBLINGS_COUNT:1)) (SEMTREX_MATCH:2 (SEMTREX_MATCH_SYMBOL:TEST_GROUP_SYMBOL2) (SEMTREX_MATCH_PATH:/2) (SEMTREX_MATCH_SIBLINGS_COUNT:1)) (SEMTREX_MATCH:2 (SEMTREX_MATCH_SYMBOL:TEST_GROUP_SYMBOL2) (SEMTREX_MATCH_PATH:/3) (SEMTREX_MATCH_SIBLINGS_COUNT:1)))");
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
    //debug_enable(D_STX_MATCH);
    spec_is_true(_t_matchr(s,t,&r));
    //debug_disable(D_STX_MATCH);

    spec_is_str_equal(t2s(r), "(SEMTREX_MATCH:1 (SEMTREX_MATCH_SYMBOL:TEST_GROUP_SYMBOL1) (SEMTREX_MATCH_PATH:/3) (SEMTREX_MATCH_SIBLINGS_COUNT:2))");

    _t_free(r);
    _t_free(t);
    _t_free(s);
    _t_free(g);

    sg = _t_news(0,SEMTREX_GROUP,VERB);
    s = _t_newr(sg,SEMTREX_SYMBOL_LITERAL);
    _t_news(s,SEMTREX_SYMBOL,LINE);

    t = _t_new_str(0,LINE,"receptors");
    spec_is_true(_t_matchr(sg,t,&r));
    spec_is_str_equal(t2s(r), "(SEMTREX_MATCH:1 (SEMTREX_MATCH_SYMBOL:VERB) (SEMTREX_MATCH_PATH:/) (SEMTREX_MATCH_SIBLINGS_COUNT:1))");

    T *map = _stx_results2sem_map(G_sem,r,t);
    spec_is_str_equal(t2s(map),"(SEMANTIC_MAP (SEMANTIC_LINK (USAGE:VERB) (REPLACEMENT_VALUE (VERB:receptors))))");

    _t_free(map);
    _t_free(r);
    _t_free(t);
    _t_free(sg);

}

void testMatchGroupMulti() {
    sX(TSTX_TOKENS,NULL_STRUCTURE); //should be some optionality LIST
    sX(TSTX_OP,NULL_STRUCTURE);
    sX(TSTX_SL,NULL_STRUCTURE);

    T *s = _t_new_root(ASCII_CHARS);
    _t_newc(s,ASCII_CHAR,'/');
    _t_newc(s,ASCII_CHAR,'(');
    _t_newc(s,ASCII_CHAR,'(');

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

    spec_is_str_equal(t2s(results), "(SEMTREX_MATCH:1 (SEMTREX_MATCH_SYMBOL:STX_TOKENS) (SEMTREX_MATCH_PATH:/) (SEMTREX_MATCH_SIBLINGS_COUNT:1) (SEMTREX_MATCH:2 (SEMTREX_MATCH_SYMBOL:STX_SL) (SEMTREX_MATCH_PATH:/1) (SEMTREX_MATCH_SIBLINGS_COUNT:1)) (SEMTREX_MATCH:3 (SEMTREX_MATCH_SYMBOL:STX_OP) (SEMTREX_MATCH_PATH:/2) (SEMTREX_MATCH_SIBLINGS_COUNT:1)) (SEMTREX_MATCH:3 (SEMTREX_MATCH_SYMBOL:STX_OP) (SEMTREX_MATCH_PATH:/3) (SEMTREX_MATCH_SIBLINGS_COUNT:1)))");

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

    s = _t_parse(G_sem,0,"(SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:ASCII_CHARS) (SEMTREX_SEQUENCE (SEMTREX_GROUP:TEST_GROUP_SYMBOL1 (SEMTREX_ZERO_OR_MORE (SEMTREX_VALUE_LITERAL_NOT (SEMTREX_VALUE_SET (ASCII_CHAR:'/') (ASCII_CHAR:'?') (ASCII_CHAR:' ')))))))");
    char buf[2000];
    spec_is_str_equal(_dump_semtrex(G_sem,s,buf),"/ASCII_CHARS/<TEST_GROUP_SYMBOL1:ASCII_CHAR!={'/','?',' '}*>");
    t = _t_parse(G_sem,0,"(ASCII_CHARS (ASCII_CHAR:'a') (ASCII_CHAR:'b') (ASCII_CHAR:'/'))");

    T *r;
    //debug_enable(D_STX_MATCH+D_STX_BUILD);
    spec_is_true(_t_matchr(s,t,&r));
    debug_disable(D_STX_MATCH+D_STX_BUILD);

    spec_is_str_equal(t2s(r),"(SEMTREX_MATCH:1 (SEMTREX_MATCH_SYMBOL:TEST_GROUP_SYMBOL1) (SEMTREX_MATCH_PATH:/1) (SEMTREX_MATCH_SIBLINGS_COUNT:2))");

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
    //debug_enable(D_STX_MATCH);
    s = _t_new_root(SEMTREX_WALK);
    T *g = _t_news(s,SEMTREX_GROUP,TEST_GROUP_SYMBOL1);
    T *sq = _t_newr(g,SEMTREX_SEQUENCE);
    _sl(sq,sy3);
    _sl(sq,sy4);
    T *results;
    spec_is_true(_t_matchr(s,t,&results));
    spec_is_str_equal(t2s(results),"(SEMTREX_MATCH:1 (SEMTREX_MATCH_SYMBOL:TEST_GROUP_SYMBOL1) (SEMTREX_MATCH_PATH:/3) (SEMTREX_MATCH_SIBLINGS_COUNT:2))");

    _t_free(results);
    _t_free(s);

    _t_free(t);

    // this test is taken from code that broke in semtrex parsing, I think because of the
    // walking
    s = _t_parse(G_sem,0,"(SEMTREX_WALK (SEMTREX_SEQUENCE (SEMTREX_ZERO_OR_MORE (SEMTREX_SYMBOL_ANY)) (SEMTREX_GROUP:STX_CHILD (SEMTREX_SEQUENCE (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:STX_LABEL)) (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:STX_SL)) (SEMTREX_SYMBOL_LITERAL_NOT (SEMTREX_SYMBOL:STX_SL))))))");
    t = _t_parse(G_sem,0,"(STX_TOKENS (STX_SL) (STX_LABEL:\"TEST_STR_SYMBOL\") (STX_SL) (STX_SIBS (STX_LABEL:\"sy1\") (STX_SL) (STX_LABEL:\"sy11\") (STX_SL) (STX_LABEL:\"sy111\") (STX_COMMA) (STX_LABEL:\"sy2\") (STX_COMMA) (STX_LABEL:\"sy3\")))");

    //debug_enable(D_STX_MATCH);
    _t_matchr(s,t,&results);
    debug_disable(D_STX_MATCH);

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

    sX(R,NULL_STRUCTURE);
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
    spec_is_str_equal(_dump_semtrex(G_sem,s,buf),"");

    stx_dump(s);

    //debug_enable(D_STX_MATCH);
    spec_is_true(_t_matchr(s,t,&results));
    //debug_disable(D_STX_MATCH);

    spec_is_str_equal(t2s(results),"");
    //int *p = _t_get_path(G_ts);
    //puts("MATCHPATH:");
    //puts(_t_sprint_path(p,buf));

    //_t_free(results);
    //free(p);

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

    spec_is_str_equal(_dump_semtrex(G_sem,s,buf),"/TEST_STR_SYMBOL/(sy1/sy11/sy111,sy2,sy3)");
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
    spec_is_str_equal(_dump_semtrex(G_sem,s,buf),"/TEST_STR_SYMBOL/(<TEST_GROUP_SYMBOL1:.*,<TEST_GROUP_SYMBOL2:.>>,sy4)");
    _t_free(s);

    // /TEST_STR_SYMBOL|TEST_INT_SYMBOL
    s = _t_new_root(SEMTREX_OR);
    _sl(s,TEST_STR_SYMBOL);
    _sl(s,TEST_INT_SYMBOL);
    spec_is_str_equal(_dump_semtrex(G_sem,s,buf),"/TEST_STR_SYMBOL|TEST_INT_SYMBOL");
    _t_free(s);

    s = newvl(0,1,1,_t_newc(0,ASCII_CHAR,'x'));
    spec_is_str_equal(_dump_semtrex(G_sem,s,buf),"/ASCII_CHAR!='x'");
    _t_free(s);

    s = newvl(0,0,1,_t_newc(0,ASCII_CHAR,'x'));
    spec_is_str_equal(_dump_semtrex(G_sem,s,buf),"/ASCII_CHAR='x'");
    _t_free(s);

    s = newvl(0,0,1,_t_new(0,TEST_STR_SYMBOL,"test",5));
    spec_is_str_equal(_dump_semtrex(G_sem,s,buf),"/TEST_STR_SYMBOL=\"test\"");
    _t_free(s);

    s = newvl(0,0,1,_t_newi(0,TEST_INT_SYMBOL,314));
    spec_is_str_equal(_dump_semtrex(G_sem,s,buf),"/TEST_INT_SYMBOL=314");
    _t_free(s);

    //  /TEST_STR_SYMBOL/~sy1
    s = _sl(0,TEST_STR_SYMBOL);
    T *n = _t_newr(s,SEMTREX_NOT);
    n = _sl(n,sy1);
    spec_is_str_equal(_dump_semtrex(G_sem,s,buf),"/TEST_STR_SYMBOL/~sy1");
    _t_free(s);

}

void testSemtrexParse() {
    char buf[5000];

    char *stx;
    T *s;
    stx = "/TEST_STR_SYMBOL/(sy1/sy11/sy111,sy2,sy3)";

    s = parseSemtrex(G_sem,stx);

    spec_is_str_equal(_dump_semtrex(G_sem,s,buf),stx);
    spec_is_str_equal(t2s(s),"(SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:TEST_STR_SYMBOL) (SEMTREX_SEQUENCE (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:sy1) (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:sy11) (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:sy111)))) (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:sy2)) (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:sy3))))");
    _t_free(s);

    stx = "/STX_STAR|STX_PLUS|STX_Q";
    s = parseSemtrex(G_sem,stx);
    spec_is_str_equal(_dump_semtrex(G_sem,s,buf),"/(STX_STAR|STX_PLUS)|STX_Q");
    spec_is_str_equal(t2s(s),"(SEMTREX_OR (SEMTREX_OR (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:STX_STAR)) (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:STX_PLUS))) (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:STX_Q)))");
    _t_free(s);

    stx = "/TEST_STR_SYMBOL/(.+,sy1,.*,sy2,.?)";
    s = parseSemtrex(G_sem,stx);
    spec_is_str_equal(_dump_semtrex(G_sem,s,buf),stx);
    spec_is_str_equal(t2s(s),"(SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:TEST_STR_SYMBOL) (SEMTREX_SEQUENCE (SEMTREX_ONE_OR_MORE (SEMTREX_SYMBOL_ANY)) (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:sy1)) (SEMTREX_ZERO_OR_MORE (SEMTREX_SYMBOL_ANY)) (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:sy2)) (SEMTREX_ZERO_OR_ONE (SEMTREX_SYMBOL_ANY))))");
    _t_free(s);

    stx = "/STX_TOKENS/(%<SEMTREX_SEQUENCE:(!STX_COMMA,STX_COMMA)+,!STX_COMMA>)";
    s = parseSemtrex(G_sem,stx);
    spec_is_str_equal(_dump_semtrex(G_sem,s,buf),stx);
    spec_is_str_equal(t2s(s),"(SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:STX_TOKENS) (SEMTREX_WALK (SEMTREX_GROUP:SEMTREX_SEQUENCE (SEMTREX_SEQUENCE (SEMTREX_ONE_OR_MORE (SEMTREX_SEQUENCE (SEMTREX_SYMBOL_LITERAL_NOT (SEMTREX_SYMBOL:STX_COMMA)) (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:STX_COMMA)))) (SEMTREX_SYMBOL_LITERAL_NOT (SEMTREX_SYMBOL:STX_COMMA))))))");
    _t_free(s);

    stx = "/HTTP_REQUEST/(.,.,HTTP_REQUEST_PATH/HTTP_REQUEST_PATH_SEGMENTS/<HTTP_REQUEST_PATH_SEGMENT:HTTP_REQUEST_PATH_SEGMENT>)";
    s = parseSemtrex(G_sem,stx);
    spec_is_str_equal(_dump_semtrex(G_sem,s,buf),stx);
    spec_is_str_equal(t2s(s),"(SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:HTTP_REQUEST) (SEMTREX_SEQUENCE (SEMTREX_SYMBOL_ANY) (SEMTREX_SYMBOL_ANY) (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:HTTP_REQUEST_PATH) (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:HTTP_REQUEST_PATH_SEGMENTS) (SEMTREX_GROUP:HTTP_REQUEST_PATH_SEGMENT (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:HTTP_REQUEST_PATH_SEGMENT)))))))");
    _t_free(s);

    stx = "/TEST_STR_SYMBOL/(TEST_INT_SYMBOL=314,ASCII_CHAR='x',TEST_STR_SYMBOL=\"abc\",TEST_STR_SYMBOL!=\"abc\")";
    s = parseSemtrex(G_sem,stx);
    spec_is_str_equal(_dump_semtrex(G_sem,s,buf),stx);
    spec_is_str_equal(t2s(s),"(SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:TEST_STR_SYMBOL) (SEMTREX_SEQUENCE (SEMTREX_VALUE_LITERAL (TEST_INT_SYMBOL:314)) (SEMTREX_VALUE_LITERAL (ASCII_CHAR:'x')) (SEMTREX_VALUE_LITERAL (TEST_STR_SYMBOL:abc)) (SEMTREX_VALUE_LITERAL_NOT (TEST_STR_SYMBOL:abc))))");
    _t_free(s);

    stx = "/TEST_STR_SYMBOL/~sy2";
    s = parseSemtrex(G_sem,stx);
    spec_is_str_equal(_dump_semtrex(G_sem,s,buf),stx);
    spec_is_str_equal(t2s(s),"(SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:TEST_STR_SYMBOL) (SEMTREX_NOT (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:sy2))))");
    _t_free(s);


    stx = "/STX_TOKENS/(.*,<STX_OP:STX_OP,<STX_SIBS:!STX_CP+>,STX_CP>)";
    s = parseSemtrex(G_sem,stx);
    spec_is_str_equal(_dump_semtrex(G_sem,s,buf),stx);
    spec_is_str_equal(t2s(s),"(SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:STX_TOKENS) (SEMTREX_SEQUENCE (SEMTREX_ZERO_OR_MORE (SEMTREX_SYMBOL_ANY)) (SEMTREX_GROUP:STX_OP (SEMTREX_SEQUENCE (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:STX_OP)) (SEMTREX_GROUP:STX_SIBS (SEMTREX_ONE_OR_MORE (SEMTREX_SYMBOL_LITERAL_NOT (SEMTREX_SYMBOL:STX_CP)))) (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:STX_CP))))))");
    _t_free(s);

    stx = "/TEST_STR_SYMBOL=\"fish\"";
    s = parseSemtrex(G_sem,stx);
    spec_is_str_equal(_dump_semtrex(G_sem,s,buf),stx);
    spec_is_str_equal(t2s(s),"(SEMTREX_VALUE_LITERAL (TEST_STR_SYMBOL:fish))");
    _t_free(s);

    stx = "/ASCII_CHARS/(ASCII_CHAR={'a','b'},ASCII_CHAR!={'a','b'})";
    s = parseSemtrex(G_sem,stx);
    spec_is_str_equal(_dump_semtrex(G_sem,s,buf),stx);
    spec_is_str_equal(t2s(s),"(SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:ASCII_CHARS) (SEMTREX_SEQUENCE (SEMTREX_VALUE_LITERAL (SEMTREX_VALUE_SET (ASCII_CHAR:'a') (ASCII_CHAR:'b'))) (SEMTREX_VALUE_LITERAL_NOT (SEMTREX_VALUE_SET (ASCII_CHAR:'a') (ASCII_CHAR:'b')))))");
    _t_free(s);

    stx = "/STX_TOKENS/(!{STX_OP,STX_CP},ASCII_CHAR!={'a','b'},{STX_OP,STX_CP})";
    s = parseSemtrex(G_sem,stx);
    spec_is_str_equal(_dump_semtrex(G_sem,s,buf),stx);
    spec_is_str_equal(t2s(s),"(SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:STX_TOKENS) (SEMTREX_SEQUENCE (SEMTREX_SYMBOL_LITERAL_NOT (SEMTREX_SYMBOL_SET (SEMTREX_SYMBOL:STX_OP) (SEMTREX_SYMBOL:STX_CP))) (SEMTREX_VALUE_LITERAL_NOT (SEMTREX_VALUE_SET (ASCII_CHAR:'a') (ASCII_CHAR:'b'))) (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL_SET (SEMTREX_SYMBOL:STX_OP) (SEMTREX_SYMBOL:STX_CP)))))");
    _t_free(s);

    stx = "/HTTP_REQUEST/(HTTP_REQUEST_HEADERS/HTTP_REQUEST_HOST*,HTTP_REQUEST_BODY)";
    s = parseSemtrex(G_sem,stx);
    spec_is_str_equal(_dump_semtrex(G_sem,s,buf),stx);

    spec_is_str_equal(t2s(s),"(SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:HTTP_REQUEST) (SEMTREX_SEQUENCE (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:HTTP_REQUEST_HEADERS) (SEMTREX_ZERO_OR_MORE (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:HTTP_REQUEST_HOST)))) (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:HTTP_REQUEST_BODY))))");
//    stx_dump(s);
//(HTTP_REQUEST_LINE (HTTP_REQUEST_VERSION (VERSION_MAJOR:1) (VERSION_MINOR:1)) (HTTP_REQUEST_METHOD:\"GET\") (HTTP_REQUEST_PATH (HTTP_REQUEST_PATH_SEGMENTS (HTTP_REQUEST_PATH_SEGMENT:\"groups\") (HTTP_REQUEST_PATH_SEGMENT:\"5\")) (HTTP_REQUEST_PATH_FILE (FILE_NAME:\"users\") (FILE_EXTENSION:\"json\")) (HTTP_REQUEST_PATH_QUERY (HTTP_REQUEST_PATH_QUERY_PARAMS (HTTP_REQUEST_PATH_QUERY_PARAM (PARAM_KEY:\"sort_by\") (PARAM_VALUE:\"last_name\")) (HTTP_REQUEST_PATH_QUERY_PARAM (PARAM_KEY:\"page\") (PARAM_VALUE:\"2\"))))))
    T *t = _t_parse(G_sem,0,"(HTTP_REQUEST  (HTTP_REQUEST_HEADERS (HTTP_REQUEST_HOST:\"fish.com\")) (HTTP_REQUEST_BODY))");

    spec_is_true(_t_match(s,t));
    _t_free(s);
    _t_free(t);

}

void testSemtrexParseHHTPReq() {
    char *req = "GET /path/to/file.ext?name=joe&age=30 HTTP/0.9";
    T *r,*s = makeASCIITree(req);

    char buf[50000];
    T *stx;
    stx = _makeHTTPRequestSemtrex();

    // we add the NULL_SYMBOL group in to test the kind of match that happens on expectations
    T *x = _t_news(0,SEMTREX_GROUP,NULL_SYMBOL);
    _t_add(x,stx);
    stx = x;

    _t2json(G_sem,stx,INDENT,buf);
    writeFile("web/httpreqstx.json",buf,strlen(buf));

    _dump_semtrex(G_sem,stx,buf);

    spec_is_str_equal(buf,"/<NULL_SYMBOL:ASCII_CHARS/<HTTP_REQUEST:<HTTP_REQUEST_METHOD:ASCII_CHAR!=' '+>,ASCII_CHAR=' ',<HTTP_REQUEST_PATH:<HTTP_REQUEST_PATH_SEGMENTS:(ASCII_CHAR='/',<HTTP_REQUEST_PATH_SEGMENT:ASCII_CHAR!={'/','?',' '}*>)+>>,(ASCII_CHAR='?',<HTTP_REQUEST_PATH_QUERY:<HTTP_REQUEST_PATH_QUERY_PARAMS:(<HTTP_REQUEST_PATH_QUERY_PARAM:<PARAM_KEY:ASCII_CHAR!={'&',' ','='}+>,ASCII_CHAR='=',<PARAM_VALUE:ASCII_CHAR!={'&',' '}*>>,ASCII_CHAR='&'?)+>+>)?,ASCII_CHAR=' ',ASCII_CHAR='H',ASCII_CHAR='T',ASCII_CHAR='T',ASCII_CHAR='P',ASCII_CHAR='/',<HTTP_REQUEST_VERSION:<VERSION_MAJOR:ASCII_CHAR='0'>,ASCII_CHAR='.',<VERSION_MINOR:ASCII_CHAR='9'>>>>");

    //debug_enable(D_STX_MATCH);
    spec_is_true(_t_matchr(stx,s,&r));
    //debug_disable(D_STX_MATCH);

    spec_is_str_equal(t2s(r),"(SEMTREX_MATCH:1 (SEMTREX_MATCH_SYMBOL:NULL_SYMBOL) (SEMTREX_MATCH_PATH:/) (SEMTREX_MATCH_SIBLINGS_COUNT:1) (SEMTREX_MATCH:2 (SEMTREX_MATCH_SYMBOL:HTTP_REQUEST) (SEMTREX_MATCH_PATH:/1) (SEMTREX_MATCH_SIBLINGS_COUNT:46) (SEMTREX_MATCH:14 (SEMTREX_MATCH_SYMBOL:HTTP_REQUEST_METHOD) (SEMTREX_MATCH_PATH:/1) (SEMTREX_MATCH_SIBLINGS_COUNT:3)) (SEMTREX_MATCH:11 (SEMTREX_MATCH_SYMBOL:HTTP_REQUEST_PATH) (SEMTREX_MATCH_PATH:/5) (SEMTREX_MATCH_SIBLINGS_COUNT:17) (SEMTREX_MATCH:12 (SEMTREX_MATCH_SYMBOL:HTTP_REQUEST_PATH_SEGMENTS) (SEMTREX_MATCH_PATH:/5) (SEMTREX_MATCH_SIBLINGS_COUNT:17) (SEMTREX_MATCH:13 (SEMTREX_MATCH_SYMBOL:HTTP_REQUEST_PATH_SEGMENT) (SEMTREX_MATCH_PATH:/6) (SEMTREX_MATCH_SIBLINGS_COUNT:4)) (SEMTREX_MATCH:13 (SEMTREX_MATCH_SYMBOL:HTTP_REQUEST_PATH_SEGMENT) (SEMTREX_MATCH_PATH:/11) (SEMTREX_MATCH_SIBLINGS_COUNT:2)) (SEMTREX_MATCH:13 (SEMTREX_MATCH_SYMBOL:HTTP_REQUEST_PATH_SEGMENT) (SEMTREX_MATCH_PATH:/14) (SEMTREX_MATCH_SIBLINGS_COUNT:8)))) (SEMTREX_MATCH:6 (SEMTREX_MATCH_SYMBOL:HTTP_REQUEST_PATH_QUERY) (SEMTREX_MATCH_PATH:/23) (SEMTREX_MATCH_SIBLINGS_COUNT:15) (SEMTREX_MATCH:7 (SEMTREX_MATCH_SYMBOL:HTTP_REQUEST_PATH_QUERY_PARAMS) (SEMTREX_MATCH_PATH:/23) (SEMTREX_MATCH_SIBLINGS_COUNT:15) (SEMTREX_MATCH:8 (SEMTREX_MATCH_SYMBOL:HTTP_REQUEST_PATH_QUERY_PARAM) (SEMTREX_MATCH_PATH:/23) (SEMTREX_MATCH_SIBLINGS_COUNT:8) (SEMTREX_MATCH:10 (SEMTREX_MATCH_SYMBOL:PARAM_KEY) (SEMTREX_MATCH_PATH:/23) (SEMTREX_MATCH_SIBLINGS_COUNT:4)) (SEMTREX_MATCH:9 (SEMTREX_MATCH_SYMBOL:PARAM_VALUE) (SEMTREX_MATCH_PATH:/28) (SEMTREX_MATCH_SIBLINGS_COUNT:3))) (SEMTREX_MATCH:8 (SEMTREX_MATCH_SYMBOL:HTTP_REQUEST_PATH_QUERY_PARAM) (SEMTREX_MATCH_PATH:/32) (SEMTREX_MATCH_SIBLINGS_COUNT:6) (SEMTREX_MATCH:10 (SEMTREX_MATCH_SYMBOL:PARAM_KEY) (SEMTREX_MATCH_PATH:/32) (SEMTREX_MATCH_SIBLINGS_COUNT:3)) (SEMTREX_MATCH:9 (SEMTREX_MATCH_SYMBOL:PARAM_VALUE) (SEMTREX_MATCH_PATH:/36) (SEMTREX_MATCH_SIBLINGS_COUNT:2))))) (SEMTREX_MATCH:3 (SEMTREX_MATCH_SYMBOL:HTTP_REQUEST_VERSION) (SEMTREX_MATCH_PATH:/44) (SEMTREX_MATCH_SIBLINGS_COUNT:3) (SEMTREX_MATCH:5 (SEMTREX_MATCH_SYMBOL:VERSION_MAJOR) (SEMTREX_MATCH_PATH:/44) (SEMTREX_MATCH_SIBLINGS_COUNT:1)) (SEMTREX_MATCH:4 (SEMTREX_MATCH_SYMBOL:VERSION_MINOR) (SEMTREX_MATCH_PATH:/46) (SEMTREX_MATCH_SIBLINGS_COUNT:1)))))");

    T *t = _t_embody_from_match(G_sem,r,HTTP_REQUEST,s);
    spec_is_str_equal(t2s(t),"(HTTP_REQUEST (HTTP_REQUEST_METHOD:GET) (HTTP_REQUEST_PATH (HTTP_REQUEST_PATH_SEGMENTS (HTTP_REQUEST_PATH_SEGMENT:path) (HTTP_REQUEST_PATH_SEGMENT:to) (HTTP_REQUEST_PATH_SEGMENT:file.ext))) (HTTP_REQUEST_PATH_QUERY (HTTP_REQUEST_PATH_QUERY_PARAMS (HTTP_REQUEST_PATH_QUERY_PARAM (PARAM_KEY:name) (PARAM_VALUE:joe)) (HTTP_REQUEST_PATH_QUERY_PARAM (PARAM_KEY:age) (PARAM_VALUE:30)))) (HTTP_REQUEST_VERSION (VERSION_MAJOR:0) (VERSION_MINOR:9)))");

    T *map = _stx_results2sem_map(G_sem,r,s);
    spec_is_str_equal(t2s(map),"(SEMANTIC_MAP (SEMANTIC_LINK (USAGE:NULL_SYMBOL) (REPLACEMENT_VALUE (ASCII_CHARS (ASCII_CHAR:'G') (ASCII_CHAR:'E') (ASCII_CHAR:'T') (ASCII_CHAR:' ') (ASCII_CHAR:'/') (ASCII_CHAR:'p') (ASCII_CHAR:'a') (ASCII_CHAR:'t') (ASCII_CHAR:'h') (ASCII_CHAR:'/') (ASCII_CHAR:'t') (ASCII_CHAR:'o') (ASCII_CHAR:'/') (ASCII_CHAR:'f') (ASCII_CHAR:'i') (ASCII_CHAR:'l') (ASCII_CHAR:'e') (ASCII_CHAR:'.') (ASCII_CHAR:'e') (ASCII_CHAR:'x') (ASCII_CHAR:'t') (ASCII_CHAR:'?') (ASCII_CHAR:'n') (ASCII_CHAR:'a') (ASCII_CHAR:'m') (ASCII_CHAR:'e') (ASCII_CHAR:'=') (ASCII_CHAR:'j') (ASCII_CHAR:'o') (ASCII_CHAR:'e') (ASCII_CHAR:'&') (ASCII_CHAR:'a') (ASCII_CHAR:'g') (ASCII_CHAR:'e') (ASCII_CHAR:'=') (ASCII_CHAR:'3') (ASCII_CHAR:'0') (ASCII_CHAR:' ') (ASCII_CHAR:'H') (ASCII_CHAR:'T') (ASCII_CHAR:'T') (ASCII_CHAR:'P') (ASCII_CHAR:'/') (ASCII_CHAR:'0') (ASCII_CHAR:'.') (ASCII_CHAR:'9')))) (SEMANTIC_LINK (USAGE:HTTP_REQUEST) (REPLACEMENT_VALUE (HTTP_REQUEST (HTTP_REQUEST_METHOD:GET) (HTTP_REQUEST_PATH (HTTP_REQUEST_PATH_SEGMENTS (HTTP_REQUEST_PATH_SEGMENT:path) (HTTP_REQUEST_PATH_SEGMENT:to) (HTTP_REQUEST_PATH_SEGMENT:file.ext))) (HTTP_REQUEST_PATH_QUERY (HTTP_REQUEST_PATH_QUERY_PARAMS (HTTP_REQUEST_PATH_QUERY_PARAM (PARAM_KEY:name) (PARAM_VALUE:joe)) (HTTP_REQUEST_PATH_QUERY_PARAM (PARAM_KEY:age) (PARAM_VALUE:30)))) (HTTP_REQUEST_VERSION (VERSION_MAJOR:0) (VERSION_MINOR:9))))) (SEMANTIC_LINK (USAGE:HTTP_REQUEST_METHOD) (REPLACEMENT_VALUE (HTTP_REQUEST_METHOD:GET))) (SEMANTIC_LINK (USAGE:HTTP_REQUEST_PATH) (REPLACEMENT_VALUE (HTTP_REQUEST_PATH (HTTP_REQUEST_PATH_SEGMENTS (HTTP_REQUEST_PATH_SEGMENT:path) (HTTP_REQUEST_PATH_SEGMENT:to) (HTTP_REQUEST_PATH_SEGMENT:file.ext))))) (SEMANTIC_LINK (USAGE:HTTP_REQUEST_PATH_SEGMENTS) (REPLACEMENT_VALUE (HTTP_REQUEST_PATH_SEGMENTS (HTTP_REQUEST_PATH_SEGMENT:path) (HTTP_REQUEST_PATH_SEGMENT:to) (HTTP_REQUEST_PATH_SEGMENT:file.ext)))) (SEMANTIC_LINK (USAGE:HTTP_REQUEST_PATH_SEGMENT) (REPLACEMENT_VALUE (HTTP_REQUEST_PATH_SEGMENT:path))) (SEMANTIC_LINK (USAGE:HTTP_REQUEST_PATH_SEGMENT) (REPLACEMENT_VALUE (HTTP_REQUEST_PATH_SEGMENT:to))) (SEMANTIC_LINK (USAGE:HTTP_REQUEST_PATH_SEGMENT) (REPLACEMENT_VALUE (HTTP_REQUEST_PATH_SEGMENT:file.ext))) (SEMANTIC_LINK (USAGE:HTTP_REQUEST_PATH_QUERY) (REPLACEMENT_VALUE (HTTP_REQUEST_PATH_QUERY (HTTP_REQUEST_PATH_QUERY_PARAMS (HTTP_REQUEST_PATH_QUERY_PARAM (PARAM_KEY:name) (PARAM_VALUE:joe)) (HTTP_REQUEST_PATH_QUERY_PARAM (PARAM_KEY:age) (PARAM_VALUE:30)))))) (SEMANTIC_LINK (USAGE:HTTP_REQUEST_PATH_QUERY_PARAMS) (REPLACEMENT_VALUE (HTTP_REQUEST_PATH_QUERY_PARAMS (HTTP_REQUEST_PATH_QUERY_PARAM (PARAM_KEY:name) (PARAM_VALUE:joe)) (HTTP_REQUEST_PATH_QUERY_PARAM (PARAM_KEY:age) (PARAM_VALUE:30))))) (SEMANTIC_LINK (USAGE:HTTP_REQUEST_PATH_QUERY_PARAM) (REPLACEMENT_VALUE (HTTP_REQUEST_PATH_QUERY_PARAM (PARAM_KEY:name) (PARAM_VALUE:joe)))) (SEMANTIC_LINK (USAGE:PARAM_KEY) (REPLACEMENT_VALUE (PARAM_KEY:name))) (SEMANTIC_LINK (USAGE:PARAM_VALUE) (REPLACEMENT_VALUE (PARAM_VALUE:joe))) (SEMANTIC_LINK (USAGE:HTTP_REQUEST_PATH_QUERY_PARAM) (REPLACEMENT_VALUE (HTTP_REQUEST_PATH_QUERY_PARAM (PARAM_KEY:age) (PARAM_VALUE:30)))) (SEMANTIC_LINK (USAGE:PARAM_KEY) (REPLACEMENT_VALUE (PARAM_KEY:age))) (SEMANTIC_LINK (USAGE:PARAM_VALUE) (REPLACEMENT_VALUE (PARAM_VALUE:30))) (SEMANTIC_LINK (USAGE:HTTP_REQUEST_VERSION) (REPLACEMENT_VALUE (HTTP_REQUEST_VERSION (VERSION_MAJOR:0) (VERSION_MINOR:9)))) (SEMANTIC_LINK (USAGE:VERSION_MAJOR) (REPLACEMENT_VALUE (VERSION_MAJOR:0))) (SEMANTIC_LINK (USAGE:VERSION_MINOR) (REPLACEMENT_VALUE (VERSION_MINOR:9))))");
    _t_free(r);
    _t_free(map);

    T *pstx = parseSemtrex(G_sem,"/%HTTP_REQUEST_PATH_SEGMENTS/(HTTP_REQUEST_PATH_SEGMENT=\"path\",HTTP_REQUEST_PATH_SEGMENT=\"to\",HTTP_REQUEST_PATH_SEGMENT=\"file.ext\")");

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
    sX(LABELED_TRIPLET,NULL_STRUCTURE); //should be an optionality LIST
    sX(LABEL,CSTRING);
    sX(X1,CSTRING);
    sX(X2,INTEGER);
    sX(X3,FLOAT);

    T *s;
    char *stx;
    s = parseSemtrex(G_sem,stx = "/ASCII_CHARS/<LABELED_TRIPLET:(<LABEL:ASCII_CHAR+>,<NULL_SYMBOL:ASCII_CHAR=':'>,<X1:ASCII_CHAR+>,<NULL_SYMBOL:ASCII_CHAR=','>,<X2:ASCII_CHAR+>,<NULL_SYMBOL:ASCII_CHAR=','>,<X3:ASCII_CHAR+>)>");

    spec_is_str_equal(t2s(s),"(SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:ASCII_CHARS) (SEMTREX_GROUP:LABELED_TRIPLET (SEMTREX_SEQUENCE (SEMTREX_GROUP:LABEL (SEMTREX_ONE_OR_MORE (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:ASCII_CHAR)))) (SEMTREX_GROUP:NULL_SYMBOL (SEMTREX_VALUE_LITERAL (ASCII_CHAR:':'))) (SEMTREX_GROUP:X1 (SEMTREX_ONE_OR_MORE (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:ASCII_CHAR)))) (SEMTREX_GROUP:NULL_SYMBOL (SEMTREX_VALUE_LITERAL (ASCII_CHAR:','))) (SEMTREX_GROUP:X2 (SEMTREX_ONE_OR_MORE (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:ASCII_CHAR)))) (SEMTREX_GROUP:NULL_SYMBOL (SEMTREX_VALUE_LITERAL (ASCII_CHAR:','))) (SEMTREX_GROUP:X3 (SEMTREX_ONE_OR_MORE (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:ASCII_CHAR)))))))");

    spec_is_true(_t_matchr(s,t,&r));

    T *e = __t_embody_from_match(G_sem,r,t);
    spec_is_str_equal(t2s(e),"(LABELED_TRIPLET (LABEL:label) (X1:abc) (X2:123) (X3:3.141500))");

    _t_free(r);
    _t_free(s);

    s =  parseSemtrex(G_sem,"/LABELED_TRIPLET/(.*,X3=3.1415)");
    spec_is_str_equal(t2s(s),"(SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:LABELED_TRIPLET) (SEMTREX_SEQUENCE (SEMTREX_ZERO_OR_MORE (SEMTREX_SYMBOL_ANY)) (SEMTREX_VALUE_LITERAL (X3:3.141500))))");
    spec_is_true(_t_match(s,e));

    //    _t_free(r);
    _t_free(s);

    _t_free(e);
    _t_free(t);
}

void testSemtrexReplace() {
    //    char *stx = "%EXPECT/.*,<ACTION:GOAL=RESPOND>";
    //    T *s = parseSemtrex(G_sem,stx);  Doesn't work for symbols as value literals, sigh
    // %EXPECT/.*,<ACTION:GOAL=some_process>
    T *stx = _t_new_root(SEMTREX_WALK);
    T *s = _sl(stx,EXPECT);
    s = _t_newr(s,SEMTREX_SEQUENCE);
    _t_newr(_t_newr(s,SEMTREX_ZERO_OR_MORE),SEMTREX_SYMBOL_ANY);
    T *g = _t_news(s,SEMTREX_GROUP,ACTION);
    T *vl = _t_newr(g,SEMTREX_VALUE_LITERAL);
    Symbol some_process = _d_define_symbol(G_sem,PROCESS,"some_process",TEST_CONTEXT);
    Symbol some_interaction = _d_define_symbol(G_sem,INTERACTION,"some_interaction",TEST_CONTEXT);
    _t_news(vl,GOAL,some_process);

    T *d = _t_new_root(some_interaction);
    T *e = _t_newr(d,EXPECT);
    //    _t_newr(rp,ROLE);
    //    _t_newr(rp,SOURCE);
    _t_newr(e,PATTERN);
    _t_news(e,GOAL,some_process);

    T *a = _t_news(0,ACTION,RESPOND);
    _stx_replace(stx,d,a);
    spec_is_str_equal(t2s(d),"(some_interaction (EXPECT (PATTERN) (ACTION:RESPOND)))");
    _t_free(d);
    _t_free(stx);
    _t_free(a);
}


void testSemtrex() {
    _stxSetup();
    //testMakeFA();
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
    testSemtrexReplace();
}
