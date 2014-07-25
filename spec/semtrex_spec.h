#include "../src/ceptr.h"
#include "../src/hashfn.h"
#include "../src/semtrex.h"
#define OTHER_TEST_SYMBOL (TEST_SYMBOL+999)


/*
Semtrex BNF:

<STX>            ::= <union> | <simple-STX>
<union>          ::= <STX> "|" <simple-STX>
<simple-STX>     ::= <concatenation> | <basic-STX>
<concatenation>	 ::=  <simple-STX> "," <basic-STX>
<basic-STX>	 ::= <star> | <plus> | <elementary-STX>
<star>	         ::= <elementary-STX> "*"
<plus>	         ::= <elementary-STX> "+"
<elementary-STX> ::= <group> | <any> | <symbol-tree>
<group>	         ::= "(" <STX> ")"
<any>	         ::= "."

*/

Tnode *_makeTestTree1() {
    Tnode *t = _t_new(0,0,"t",2);
    Tnode *t1 = _t_new(t,1,"t1",3);
    Tnode *t11 = _t_new(t1,11,"t11",4);
    Tnode *t111 = _t_new(t11,111,"t111",5);
    Tnode *t2 = _t_new(t,2,"t2",3);
    Tnode *t21 = _t_new(t2,21,"t21",4);
    Tnode *t22 = _t_new(t2,22,"t22",4);
    Tnode *t3 = _t_new(t,3,"t3",3);
    Tnode *t4 = _t_new(t,4,"t4",3);
    return t;
}

Tnode *_makeTestSemtrex1(Tnode *t) {
    // make a tree that matches same root and first two children as test tree1
    Tnode *s = _t_newi(0,SEMTREX_SYMBOL_LITERAL,0);
    Tnode *s1 = _t_newi(s,SEMTREX_SYMBOL_LITERAL,1);
    Tnode *s11 = _t_newi(s1,SEMTREX_SYMBOL_LITERAL,11);
    Tnode *s111 = _t_newi(s11,SEMTREX_SYMBOL_LITERAL,111);
    Tnode *s2 = _t_newi(s,SEMTREX_SYMBOL_LITERAL,2);
    Tnode *s3 = _t_newi(s,SEMTREX_SYMBOL_LITERAL,3);
    return s;
}

char buf[10000];
void __t_dump(Tnode *t,int level,char *buf) {
    Symbol s = _t_symbol(t);
    sprintf(buf," (%d",s);
    if (s == TEST_SYMBOL)
	sprintf(buf+strlen(buf),":%s",(char *)_t_surface(t));
    else if (s == SEMTREX_SYMBOL_LITERAL)
	sprintf(buf+strlen(buf),":%d",*(int *)_t_surface(t));

    for(int i=1;i<=_t_children(t);i++) __t_dump(_t_child(t,i),level+1,buf+strlen(buf));
    sprintf(buf+strlen(buf),")");
}

#define spec_state_equal(sa,st,tt,s) \
    spec_is_equal(sa->type,st);\
    spec_is_equal(sa->transition,tt);\
    spec_is_equal(sa->symbol,s);\
    spec_is_ptr_equal(sa->out1,NULL);


void testMakeFA() {
    Tnode *t = _makeTestTree1();
    Tnode *s = _makeTestSemtrex1(t);
    __t_dump(s,0,buf);
    puts(buf);

    SState *sa = _s_makeFA(s);
    spec_state_equal(sa,StateSymbol,TransitionDown,0);

    SState *s1 = sa->out;
    spec_state_equal(s1,StateSymbol,TransitionDown,1);

    SState *s2 = s1->out;
    spec_state_equal(s2,StateSymbol,TransitionDown,11);

    SState *s3 = s2->out;
    spec_state_equal(s3,StateSymbol,-2,111);

    SState *s4 = s3->out;
    spec_state_equal(s4,StateSymbol,TransitionNextChild,2);
/*
    SState *s5 = s4->out;
    spec_state_equal(s5,StateSymbol,TransitionNextChild,3);

    SState *s6 = s5->out;
    spec_state_equal(s6,StateMatch,TransitionNextChild,0);

    spec_is_ptr_equal(s6->out,NULL);
*/


    _s_freeFA(sa);
    _t_free(t);
    _t_free(s);
}

void testMatchTrees() {
    Tnode *t = _makeTestTree1();
    Tnode *s = _makeTestSemtrex1(t);

    spec_is_true(_t_match(s,t));

    Tnode *s2 = _t_newi(s,SEMTREX_SYMBOL_LITERAL,OTHER_TEST_SYMBOL);

    spec_is_true(!_t_match(s,t));

    _t_free(t);
    _t_free(s);
}

void testSemtrex() {
    testMatchTrees();
    testMakeFA();
}
