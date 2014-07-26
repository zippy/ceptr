#include "../src/ceptr.h"
#include "../src/hashfn.h"
#include "../src/semtrex.h"

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

Tnode *_makeTestSemtrex1() {
    Tnode *s = _t_newi(0,SEMTREX_SYMBOL_LITERAL,0);
    Tnode *ss = _t_newi(s,SEMTREX_SEQUENCE,0);
    Tnode *s1 = _t_newi(ss,SEMTREX_SYMBOL_LITERAL,1);
    Tnode *s11 = _t_newi(s1,SEMTREX_SYMBOL_LITERAL,11);
    Tnode *s111 = _t_newi(s11,SEMTREX_SYMBOL_LITERAL,111);
    Tnode *s2 = _t_newi(ss,SEMTREX_SYMBOL_LITERAL,2);
    Tnode *s3 = _t_newi(ss,SEMTREX_SYMBOL_LITERAL,3);
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

static int dump_id = 99;


void __s_dump(SState *s) {
    if (s->id == dump_id) {printf("X");return;}
    s->id = dump_id;
    switch (s->type) {
    case StateMatch:
	printf("(M)");
	break;
    case StateSymbol:
	printf("(%d:%d)",s->symbol,s->transition);
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
    spec_is_equal(sa->symbol,s);\
    spec_is_ptr_equal(sa->out1,NULL);


void testMakeFA() {
    Tnode *s = _makeTestSemtrex1();

    int states = 0;
    SState *sa = _s_makeFA(s,&states);
    spec_is_equal(states,6);

    spec_state_equal(sa,StateSymbol,TransitionDown,0);

    SState *s1 = sa->out;
    spec_state_equal(s1,StateSymbol,TransitionDown,1);

    SState *s2 = s1->out;
    spec_state_equal(s2,StateSymbol,TransitionDown,11);

    SState *s3 = s2->out;
    spec_state_equal(s3,StateSymbol,-2,111);

    SState *s4 = s3->out;
    spec_state_equal(s4,StateSymbol,TransitionNextChild,2);

    SState *s5 = s4->out;
    spec_state_equal(s5,StateSymbol,TransitionUp,3);

    SState *s6 = s5->out;
    spec_is_equal(s6->type,StateMatch);

    spec_is_ptr_equal(s6->out,NULL);

    _s_freeFA(sa);
    _t_free(s);
}

void testMatchTrees() {
    Tnode *t = _makeTestTree1();
    Tnode *s = _makeTestSemtrex1(t);

    spec_is_true(_t_match(s,t));

    Tnode *s2 = _t_newi(_t_child(s,1),SEMTREX_SYMBOL_LITERAL,99);

    spec_is_true(!_t_match(s,t));

    _t_free(t);
    _t_free(s);
}

void testMatchOr() {
    Tnode *t = _makeTestTree1();
    Tnode *s = _t_newi(0,SEMTREX_OR,0);
    Tnode *s1 = _t_newi(s,SEMTREX_SYMBOL_LITERAL,1);
    Tnode *s11 = _t_newi(s1,SEMTREX_SYMBOL_LITERAL,11);
    Tnode *s2 = _t_newi(s,SEMTREX_SYMBOL_LITERAL,0);

    spec_is_true(_t_match(s,t));

    Tnode *s3 = _t_newi(s2,SEMTREX_SYMBOL_LITERAL,99);

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
    Tnode *s = _t_newi(0,SEMTREX_SYMBOL_LITERAL,0);
    Tnode *ss = _t_newi(s,SEMTREX_SEQUENCE,0);
    Tnode *sss = _t_newi(ss,SEMTREX_STAR,0);
    Tnode *s1 = _t_newi(sss,SEMTREX_SYMBOL_LITERAL,1);

    Tnode *t = _t_new(0,0,"t",2);
    spec_is_true(_t_match(s,t));

    Tnode *t1 = _t_new(t,1,"t1",3);
    spec_is_true(_t_match(s,t));

    Tnode *t1x = _t_new(t,1,"t1",3);
    Tnode *t1y = _t_new(t,2,"t2",3);
    spec_is_true(_t_match(s,t));

    Tnode *s2 = _t_newi(ss,SEMTREX_SYMBOL_LITERAL,2);
    spec_is_true(_t_match(s,t));
    (*(int *)_t_surface(s2)) = 3;

    spec_is_true(!_t_match(s,t));
    _t_free(t);
    _t_free(s);
}

void testMatchPlus() {
    Tnode *s = _t_newi(0,SEMTREX_SYMBOL_LITERAL,0);
    Tnode *ss = _t_newi(s,SEMTREX_SEQUENCE,0);
    Tnode *sss = _t_newi(ss,SEMTREX_PLUS,0);
    Tnode *s1 = _t_newi(sss,SEMTREX_SYMBOL_LITERAL,1);

    Tnode *t = _t_new(0,0,"t",2);
    spec_is_true(!_t_match(s,t));

    Tnode *t1 = _t_new(t,1,"t1",3);
    spec_is_true(_t_match(s,t));

    Tnode *t1x = _t_new(t,1,"t1",3);
    Tnode *t1y = _t_new(t,2,"t2",3);
    spec_is_true(_t_match(s,t));

    _t_free(t);
    _t_free(s);}

void testSemtrex() {
    testMakeFA();
    testMatchTrees();
    testMatchOr();
    testMatchAny();
    testMatchStar();
    testMatchPlus();

}
