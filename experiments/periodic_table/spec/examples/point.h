#include "../../src/ceptr.h"

int proc_point_print(Receptor *r, Symbol noun, void *this) {
    printf("%d,%d", *(int *) this, *(((int *) this) + 1));
    return 0;
}

Xaddr initPoint(Receptor *r) {
    stack_push(r, XADDR_NOUN, &r->intPatternSpecXaddr);
    stack_push(r, CSTRING_NOUN, &"X");
    op_invoke(r, r->nounSpecXaddr, INSTANCE_NEW);
    Xaddr X;
    stack_pop(r, XADDR_NOUN, &X);
    //    Symbol X = preop_new_noun(r, r->intPatternSpecXaddr, "X");

    stack_push(r, XADDR_NOUN, &r->intPatternSpecXaddr);
    stack_push(r, CSTRING_NOUN, &"Y");
    op_invoke(r, r->nounSpecXaddr, INSTANCE_NEW);
    Xaddr Y;
    stack_pop(r, XADDR_NOUN, &Y);
    //    Symbol Y = preop_new_noun(r, r->intPatternSpecXaddr, "Y");

    Process point_processes[] = {
        {PRINT, (processFn) proc_point_print}
    };
    Xaddr point_children[2] = {X, Y};
    return preop_new_pattern(r, "POINT", 2, point_children, 1, point_processes);

}

void testPoint() {
    Receptor tr;init(&tr);Receptor *r = &tr;
    Xaddr pointPatternSpecXaddr = initPoint(r);

    Symbol HERE = preop_new_noun(r, pointPatternSpecXaddr, "HERE");
    int value[2] = {777, 422};
    Xaddr here_xaddr = preop_new(r, HERE, &value);
    int *v = surface_for_xaddr(r, here_xaddr);
    spec_is_true(*v == 777 && *(v + 1) == 422);
}
