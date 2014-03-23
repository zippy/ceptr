#include "../src/ceptr.h"

void testParse() {
    Tnode *t = _t_parse("(FLOW:IF (BOOLEAN:TRUE) (INTEGER:1)(INTEGER:2))");
    spec_is_equal(_t_children(t),3);
    spec_is_equal(_t_noun(t),FLOW_NOUN);
    spec_is_equal(*(int *)_t_surface(t),F_IF);
    Tnode *t1 = _t_get_child(t,1);
    spec_is_equal(_t_noun(t1),BOOLEAN_NOUN);
    spec_is_equal(*(int *)_t_surface(t1),TRUE_VALUE);
    Tnode *t2 = _t_get_child(t,2);
    spec_is_equal(_t_noun(t2),INTEGER_NOUN);
    spec_is_equal(*(int *)_t_surface(t2),1);
    Tnode *t3 = _t_get_child(t,3);
    spec_is_equal(_t_noun(t3),INTEGER_NOUN);
    spec_is_equal(*(int *)_t_surface(t3),2);
}
