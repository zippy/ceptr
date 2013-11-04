#include "ceptr.h"
#include <stdio.h>
#include <signal.h>
#include <assert.h>

#define MAX_FAILURES 1000
int spec_failures = 0;
int spec_total = 0;
char failures[MAX_FAILURES][255];

#define spec_is_true(x) spec_total++;if (x){putchar('.');} else {putchar('F');sprintf(failures[spec_failures++],"%s:%d expected %s to be true",__FUNCTION__,__LINE__,#x);}

void testInt() {
    Receptor tr; init(&tr); Receptor *r = &tr;
    Symbol MY_INT = op_new_noun(r, r->intPatternSpecXaddr, "MY_INT");
    int val = 7;
    Xaddr my_int_xaddr = op_new(r, MY_INT, &val);
    int *v = op_get(r, my_int_xaddr);
    spec_is_true(*v == 7);
    val = 8;
    op_set(r, my_int_xaddr, &val);
    v = op_get(r, my_int_xaddr);
    spec_is_true(*v == 8);
}

// PatternSpec   (  c-struct  )
// NamedPattern  (INT, POINT, LINE )
// PatternInstance  (AGE, IN_THE_SAND, HERE )

// ArraySpec      (  c-struct )
// NamedArray     (  CONSTELLATION[POINT] )
// ArrayInstance  (  receptors' semStacks )


void testArray() {
    Receptor tr;init(&tr);Receptor *r = &tr;
    Xaddr pointArray = op_new_array(r, "POINT_ARRAY", r->pointPatternSpecXaddr, 0, 0);
    Symbol CONSTELLATION = op_new_noun(r, pointArray, "CONSTELLATION");

    struct {
        int size;
        int point1X;
        int point1Y;
        int point2X;
        int point2Y;
        int point3X;
        int point3Y;
    } orion = { 3,   1,2,  10, 20,  100, 200 };
    Xaddr orionXaddr = op_new(r, CONSTELLATION, &orion);
    int *orionSurface = (int *)op_get(r, orionXaddr);
    spec_is_true(*orionSurface == 3);
    spec_is_true(*(orionSurface+2) == 2);
}

void testPoint() {
     Receptor tr;init(&tr);Receptor *r = &tr;

    Symbol HERE = op_new_noun(r, r->pointPatternSpecXaddr, "HERE");
    int value[2] = {777, 422};
    Xaddr here_xaddr = op_new(r, HERE, &value);
    int *v = op_get(r, here_xaddr);
    spec_is_true(*v == 777 && *(v + 1) == 422);
}

void testInc() {
    Receptor tr;init(&tr);Receptor *r = &tr;
    Symbol AGE = op_new_noun(r, r->intPatternSpecXaddr, "Age");
    int val = 7;
    Xaddr age_xaddr = op_new(r, AGE, &val);
    op_exec(r, age_xaddr, INC);
    int *v = op_get(r, age_xaddr);
    spec_is_true(*v == 8);
}

void testAdd() {
    Receptor tr;init(&tr);Receptor *r = &tr;
    Symbol AGE = op_new_noun(r, r->intPatternSpecXaddr, "Age");
    int val = 7;
    Xaddr age_xaddr = op_new(r, AGE, &val);
    val = 3;
    op_push_pattern(r, r->intPatternSpecXaddr.key, &val);
    op_exec(r, age_xaddr, ADD);
    spec_is_true(*(int *) (&r->valStack[0]) == 10);
}

void testSemFault() {
    Receptor tr;init(&tr);Receptor *r = &tr;
    Xaddr badXaddr = {12, 22};
    void *failSurface = op_get(r, badXaddr);
    spec_is_true(failSurface == 0);
}

void testLine() {
    Receptor tr;init(&tr);Receptor *r = &tr;
    int myLine[4] = {1, 2, 3, 4};
    Symbol inTheSand = op_new_noun(r, r->linePatternSpecXaddr, "in the sand");
    Xaddr itsLine = op_new(r, inTheSand, &myLine);

    void *surface = op_get(r, itsLine);
    spec_is_true(
    *(int *) (surface) == 1 &&
        *(int *) (surface + 4) == 2 &&
        *(int *) (surface + 8) == 3 &&
        *(int *) (surface + 12) == 4
    );
}

void testSymbolPath() {
    Receptor tr; init(&tr); Receptor *r = &tr;
    int myLine[4] = {1, 2, 3, 4};
    Symbol inTheSand = op_new_noun(r, r->linePatternSpecXaddr, "in the sand");
    Xaddr itsLine = op_new(r, inTheSand, &myLine);
    Symbol B = getSymbol(r, "B");
    Symbol Y = getSymbol(r, "Y");
    Symbol path[3] = {B, Y, SYMBOL_PATH_TERMINATOR};
    int *val;
    int seven = 7;

    val = op_getpath(r, itsLine, path);
    spec_is_true(*val == 4);

    op_setpath(r, itsLine, path, &seven);
    val = op_getpath(r, itsLine, path);
    spec_is_true(*val == 7);

    void *surface = op_get(r, itsLine);
    spec_is_true(*(((int *) surface) + 3) == 7);
}

void testRun() {
    Receptor tr;init(&tr);Receptor *r = &tr;
    int values[1] = {31415};
    ImmediatePatternOperand i = {r->intPatternSpecXaddr.key, 0};
    Instruction pushProgram[2];
    pushProgram[0].opcode = PUSH_IMMEDIATE;
    memcpy(&pushProgram[0].operands, &i, OPERANDS_SIZE);
    pushProgram[1].opcode = RETURN;
    int topOfStack = r->valStackPointer;
    spec_is_true(r->valStackPointer == 0);
    spec_is_true(r->semStackPointer == -1);
    run(r, pushProgram, values);
    spec_is_true(r->valStackPointer == 4);
    spec_is_true(r->semStackPointer == 0);
    spec_is_true(*(int *) (&r->valStack[topOfStack]) == 31415 );
}

void test_xaddr_dump() {
    Receptor tr;init(&tr);Receptor *r = &tr;
    int myLine[4] = {1, 2, 3, 4};
    Symbol inTheSand = op_new_noun(r, r->linePatternSpecXaddr, "in the sand");
    Xaddr itsLine = op_new(r, inTheSand, &myLine);
    Symbol AGE = op_new_noun(r, r->intPatternSpecXaddr, "Age");
    int val = 7;
    Xaddr age_xaddr = op_new(r, AGE, &val);
    dump_xaddrs(r);
}

void test_stack_dump() {
    Receptor tr;init(&tr);Receptor *r = &tr;
    int x = 22;
    op_push_pattern(r, r->intPatternSpecXaddr.key, &x);
    x = 44;
    op_push_pattern(r, r->intPatternSpecXaddr.key, &x);
    int myLine[4] = {1, 2, 3, 4};
    op_push_pattern(r, r->linePatternSpecXaddr.key, &myLine);
    dump_stack(r);
}

int main(int argc, const char **argv) {
    printf("Running all tests...\n\n");
    test_xaddr_dump();
    test_stack_dump();
    testInt();
    testPoint();
    testInc();
    testAdd();
    testSemFault();
    testLine();
    testSymbolPath();
    testArray();
    testRun();
    int i;
    if (spec_failures > 0) {
        printf("\n%d out of %d specs failed:\n", spec_total, spec_failures);
        for (i = 0; i < spec_failures; i++) {
            printf("%s\n", failures[i]);
        }
    }
    else {
        printf("\nAll %d specs pass\n", spec_total);
    }

    return 0;
}
