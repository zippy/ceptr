#include "../../src/ceptr.h"

int proc_line_print(Receptor *r, Symbol noun, void *this) {
    int *surface = (int *) this;
    printf("[%d,%d - %d,%d] ", *surface, *(surface + 1), *(surface + 2), *(surface + 3));
    return 0;
}


Xaddr initLine(Receptor *r) {
    // LINE
    Xaddr pointPatternSpecXaddr = initPoint(r);

    Symbol A = preop_new_noun(r, pointPatternSpecXaddr, "A");
    Symbol B = preop_new_noun(r, pointPatternSpecXaddr, "B");


    Xaddr line_children[2] = {{A, r->nounSpecXaddr.noun}, {B, r->nounSpecXaddr.noun}};

    Process line_processes[] = {
        {PRINT, (processFn) proc_line_print}
    };

    return preop_new_pattern(r, "LINE", 2, line_children, 1, line_processes);
}


void testLine() {
    Receptor tr;init(&tr);Receptor *r = &tr;
    Xaddr linePatternSpecXaddr = initLine(r);
    int myLine[4] = {1, 2, 3, 4};
    Symbol inTheSand = preop_new_noun(r, linePatternSpecXaddr, "in the sand");
    Xaddr itsLine = preop_new(r, inTheSand, &myLine);

    void *surface = surface_for_xaddr(r, itsLine);
    spec_is_true(
        *(int *) (surface) == 1 &&
            *(int *) (surface + 4) == 2 &&
            *(int *) (surface + 8) == 3 &&
            *(int *) (surface + 12) == 4
    );
}
