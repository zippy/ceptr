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

Symbol _make_star_loc(Receptor *r){
    return op_new_noun(r, r->pointPatternSpecXaddr, "STAR_LOCATION");
}

Xaddr _make_constellation(Receptor *r,Symbol STAR_LOCATION,Symbol *CONSTELLATION,int surface[]) {
    Xaddr starLocArray = op_new_array(r, "STAR_LOCATION_ARRAY", STAR_LOCATION, 0, 0);
    *CONSTELLATION = op_new_noun(r, starLocArray, "CONSTELLATION");
    return op_new(r, *CONSTELLATION, surface);
}

Xaddr _make_zodiac(Receptor *r,Symbol CONSTELLATION,Symbol *ZODIAC,void *sky) {
    Xaddr constellationArray = op_new_array(r,"CONSTELLATION_ARRAY",CONSTELLATION,0,0);
    *ZODIAC = op_new_noun(r, constellationArray, "ZODIAC");
    return op_new(r, *ZODIAC, sky);
}

void testArray() {
    Receptor tr;init(&tr);Receptor *r = &tr;
    Symbol CONSTELLATION;
    int stars[] = { 3,   1,2,  10, 20,  100, 200 };
    Symbol STAR_LOC = _make_star_loc(r);
    Xaddr orion = _make_constellation(r,STAR_LOC,&CONSTELLATION,stars);

    int *pointSurface = (int *) op_get_array_nth(r, 1, orion);
    spec_is_true(*pointSurface == 10);
    spec_is_true(*(pointSurface+1) == 20);
    spec_is_true(op_get_array_length(r,orion) == 3);

    Symbol ZODIAC;
    int sky[] = {3,   2, 2,3, 40,50,   1,100,101,  4, 11,22, 33,44, 55,66, 77,88 };
    Xaddr myZodiac = _make_zodiac(r,CONSTELLATION,&ZODIAC,sky);
    int *arraySurface = (int *) op_get_array_nth(r, 2, myZodiac);
    spec_is_true(_get_noun_size(r,ZODIAC,sky) == 18*sizeof(int));
    spec_is_true(_op_get_array_length(arraySurface) == 4);
}

int *_make_string(Receptor *r,Symbol STAR_LOCATION,Symbol *CONSTELLATION){
    Xaddr starLocString = op_new_string(r, "STAR_LOCATION_STRING", STAR_LOCATION, 0, 0);
    *CONSTELLATION = op_new_noun(r, starLocString, "CONSTELLATION");

    struct {
        int point1X;
        int point1Y;
	int escape;
        int point2X;
        int point2Y;
	int escape2;
        int point3X;
        int point3Y;
        int term;
    } orion = {  1,2,  ESCAPE_STRING_TERMINATOR,-1, 20,  ESCAPE_STRING_TERMINATOR,-2, 200, STRING_TERMINATOR };
    Xaddr orionXaddr = op_new(r, *CONSTELLATION, &orion);
    return (int *)op_get(r, orionXaddr);
}

void testString() {
    Receptor tr;init(&tr);Receptor *r = &tr;
    Symbol CONSTELLATION;
    int * orionSurface = _make_string(r,_make_star_loc(r),&CONSTELLATION);
    spec_is_true(*orionSurface == 1);
    spec_is_true(*(orionSurface+8) == STRING_TERMINATOR);
    spec_is_true(_get_noun_size(r,CONSTELLATION,orionSurface) == sizeof(int)*9);
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

    val = op_get_by_path(r, itsLine, path);
    spec_is_true(*val == 4);

    op_set_by_path(r, itsLine, path, &seven);
    val = op_get_by_path(r, itsLine, path);
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
    Symbol CONSTELLATION;
    Symbol SL = _make_star_loc(r);
    int stars[] = { 3,   1,2,  10, 20,  100, 200 };
    _make_constellation(r,SL,&CONSTELLATION,stars);
    Symbol ZODIAC;
    int sky[] = {3,   2, 2,3, 40,50,   1,100,101,  4, 11,22, 33,44, 55,66, 77,88 };
    Xaddr myZodiac = _make_zodiac(r,CONSTELLATION,&ZODIAC,sky);
    _make_string(r,SL,&CONSTELLATION);
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
    testString();
    testRun();
    int i;
    if (spec_failures > 0) {
        printf("\n%d out of %d specs failed:\n", spec_failures,spec_total);
        for (i = 0; i < spec_failures; i++) {
            printf("%s\n", failures[i]);
        }
    }
    else {
        printf("\nAll %d specs pass\n", spec_total);
    }

    return 0;
}
