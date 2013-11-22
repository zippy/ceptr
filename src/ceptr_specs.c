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
    Symbol MY_INT = preop_new_noun(r, r->intPatternSpecXaddr, "MY_INT");
    int val = 7;
    Xaddr my_int_xaddr = preop_new(r, MY_INT, &val);
    int *v = surface_for_xaddr(r, my_int_xaddr);
    spec_is_true(*v == 7);
    val = 8;
    preop_set(r, my_int_xaddr, &val);
    v = surface_for_xaddr(r, my_int_xaddr);
    spec_is_true(*v == 8);
}

Symbol _make_star_loc(Receptor *r){
    return preop_new_noun(r, r->pointPatternSpecXaddr, "STAR_LOCATION");
}

Xaddr _make_constellation(Receptor *r,Symbol STAR_LOCATION,Symbol *CONSTELLATION,int surface[]) {
    Xaddr starLocArray = preop_new_array(r, "STAR_LOCATION_ARRAY", STAR_LOCATION, 0, 0);
    *CONSTELLATION = preop_new_noun(r, starLocArray, "CONSTELLATION");
    return preop_new(r, *CONSTELLATION, surface);
}

Xaddr _make_zodiac(Receptor *r,Symbol CONSTELLATION,Symbol *ZODIAC,void *sky) {
    Xaddr constellationArray = preop_new_array(r,"CONSTELLATION_ARRAY",CONSTELLATION,0,0);
    *ZODIAC = preop_new_noun(r, constellationArray, "ZODIAC");
    return preop_new(r, *ZODIAC, sky);
}

void testArray() {
    Receptor tr;init(&tr);Receptor *r = &tr;
    Symbol CONSTELLATION;
    int stars[] = { 3,   1,2,  10, 20,  100, 200 };
    Symbol STAR_LOC = _make_star_loc(r);
    Xaddr orion = _make_constellation(r,STAR_LOC,&CONSTELLATION,stars);

    int *pointSurface = (int *) preop_get_array_nth(r, 1, orion);
    spec_is_true(*pointSurface == 10);
    spec_is_true(*(pointSurface+1) == 20);
    spec_is_true(preop_get_array_length(r,orion) == 3);

    Symbol ZODIAC;
    int sky[] = {3,   2, 2,3, 40,50,   1,100,101,  4, 11,22, 33,44, 55,66, 77,88 };
    Xaddr myZodiac = _make_zodiac(r,CONSTELLATION,&ZODIAC,sky);
    int *arraySurface = (int *) preop_get_array_nth(r, 2, myZodiac);
    spec_is_true(size_of_named_surface(r,ZODIAC,sky) == 18*sizeof(int));
    spec_is_true(_preop_get_array_length(arraySurface) == 4);
}

void testPoint() {
     Receptor tr;init(&tr);Receptor *r = &tr;

    Symbol HERE = preop_new_noun(r, r->pointPatternSpecXaddr, "HERE");
    int value[2] = {777, 422};
    Xaddr here_xaddr = preop_new(r, HERE, &value);
    int *v = surface_for_xaddr(r, here_xaddr);
    spec_is_true(*v == 777 && *(v + 1) == 422);
}
//
//void testInc() {
//    Receptor tr;init(&tr);Receptor *r = &tr;
//    Symbol AGE = preop_new_noun(r, r->intPatternSpecXaddr, "Age");
//    int val = 7;
//    Xaddr age_xaddr = preop_new(r, AGE, &val);
//    preop_exec(r, age_xaddr, INC);
//    int *v = preop_get(r, age_xaddr);
//    spec_is_true(*v == 8);
//}
//
//void testAdd() {
//    Receptor tr;init(&tr);Receptor *r = &tr;
//    Symbol AGE = preop_new_noun(r, r->intPatternSpecXaddr, "Age");
//    int val = 7;
//    Xaddr age_xaddr = preop_new(r, AGE, &val);
//    val = 3;
//    preop_push_pattern(r, r->intPatternSpecXaddr.key, &val);
//    preop_exec(r, age_xaddr, ADD);
//    spec_is_true(*(int *) (&r->valStack[0]) == 10);
//}
//
//void testSemFault() {
//    Receptor tr;init(&tr);Receptor *r = &tr;
//    Xaddr badXaddr = {12, 22};
//    void *failSurface = preop_get(r, badXaddr);
//    spec_is_true(failSurface == 0);
//}

void testLine() {
    Receptor tr;init(&tr);Receptor *r = &tr;
    int myLine[4] = {1, 2, 3, 4};
    Symbol inTheSand = preop_new_noun(r, r->linePatternSpecXaddr, "in the sand");
    Xaddr itsLine = preop_new(r, inTheSand, &myLine);

    void *surface = surface_for_xaddr(r, itsLine);
    spec_is_true(
    *(int *) (surface) == 1 &&
        *(int *) (surface + 4) == 2 &&
        *(int *) (surface + 8) == 3 &&
        *(int *) (surface + 12) == 4
    );
}
//
//void testSymbolPath() {
//    Receptor tr; init(&tr); Receptor *r = &tr;
//    int myLine[4] = {1, 2, 3, 4};
//    Symbol inTheSand = preop_new_noun(r, r->linePatternSpecXaddr, "in the sand");
//    Xaddr itsLine = preop_new(r, inTheSand, &myLine);
//    Symbol B = getSymbol(r, "B");
//    Symbol Y = getSymbol(r, "Y");
//    Symbol path[3] = {B, Y, SYMBOL_PATH_TERMINATOR};
//    int *val;
//    int seven = 7;
//
//    val = preop_get_by_path(r, itsLine, path);
//    spec_is_true(*val == 4);
//
//    preop_set_by_path(r, itsLine, path, &seven);
//    val = preop_get_by_path(r, itsLine, path);
//    spec_is_true(*val == 7);
//
//    void *surface = preop_get(r, itsLine);
//    spec_is_true(*(((int *) surface) + 3) == 7);
//}
//
//void testRun() {
//    Receptor tr;init(&tr);Receptor *r = &tr;
//    int values[1] = {31415};
//    ImmediatePatternOperand i = {r->intPatternSpecXaddr.key, 0};
//    Instruction pushProgram[2];
//    pushProgram[0].opcode = PUSH_IMMEDIATE;
//    memcpy(&pushProgram[0].operands, &i, OPERANDS_SIZE);
//    pushProgram[1].opcode = RETURN;
//    int topOfStack = r->valStackPointer;
//    spec_is_true(r->valStackPointer == 0);
//    spec_is_true(r->semStackPointer == -1);
//    run(r, pushProgram, values);
//    spec_is_true(r->valStackPointer == 4);
//    spec_is_true(r->semStackPointer == 0);
//    spec_is_true(*(int *) (&r->valStack[topOfStack]) == 31415 );
//}

void test_xaddr_dump() {
    Receptor tr;init(&tr);Receptor *r = &tr;

    Symbol AGE = preop_new_noun(r, r->intPatternSpecXaddr, "Age");
    int val = 7;
    Xaddr age_xaddr = preop_new(r, AGE, &val);

    int myLine[4] = {1, 2, 3, 4};
    Symbol inTheSand = preop_new_noun(r, r->linePatternSpecXaddr, "in the sand");
    Xaddr itsLine = preop_new(r, inTheSand, &myLine);
    Symbol CONSTELLATION;
    Symbol SL = _make_star_loc(r);
    int stars[] = { 3,   1,2,  10, 20,  100, 200 };
    _make_constellation(r,SL,&CONSTELLATION,stars);
    dump_xaddrs(r);
}

void test_stack_dump() {
    Receptor tr;init(&tr);Receptor *r = &tr;
    int x = 22;
    Symbol AGE = preop_new_noun(r, r->intPatternSpecXaddr, "Age");
    stack_push(r, AGE, &x);
    stack_push(r, CSTRING_NOUN, "Hello, Stack");
    int myLine[4] = {1, 2, 3, 4};
    Symbol inTheSand = preop_new_noun(r, r->linePatternSpecXaddr, "in the sand");
    stack_push(r, inTheSand, &myLine);
    dump_stack(r);
}

void testStack() {
    Receptor tr;Receptor *r = &tr;
    Symbol noun;
    char *peek_surface;
    init_stack(r);
    spec_is_true(r->valStackPointer == 0);
    spec_is_true(r->semStackPointer == -1);
    stack_push(r, CSTRING_NOUN, "Hello, Stack");
    spec_is_true(r->valStackPointer == 13);
    spec_is_true(r->semStackPointer == 0);
    stack_peek(r, &noun, (void **)&peek_surface);
    spec_is_true( noun == CSTRING_NOUN );
    spec_is_true( strcmp("Hello, Stack", peek_surface) == 0);

    char pop_surface[BUFFER_SIZE];
    stack_pop(r, CSTRING_NOUN, pop_surface);
    spec_is_true( strcmp("Hello, Stack", pop_surface) == 0 );
    spec_is_true(r->valStackPointer == 0);
    spec_is_true(r->semStackPointer == -1);
}

void testInit() {
    Receptor tr;init(&tr);Receptor *r = &tr;
    spec_is_true( r->rootXaddr.key == ROOT );
    spec_is_true( r->rootXaddr.noun == CSPEC_NOUN );

    spec_is_true( r->patternSpecXaddr.noun == 0);
    spec_is_true( r->patternSpecXaddr.key == 19);
    spec_is_true( r->patternNoun == 0);

    spec_is_true( r->arraySpecXaddr.noun == 27);
    spec_is_true( r->arraySpecXaddr.key == 44);
    spec_is_true( r->arrayNoun == 27);
    spec_is_true(r->semStackPointer == -1);

}

void testCspecInstanceNew() {
    Receptor tr;init_data(&tr);init_stack(&tr);Receptor *r = &tr;
    r->rootXaddr.key = ROOT;
    r->rootXaddr.noun = CSPEC_NOUN;
    stack_push(r, CSTRING_NOUN, "FIRST_SPEC");
    proc_cspec_instance_new(r,r->rootXaddr);
    Symbol noun;
    Xaddr *xaddr;
    stack_peek(r, &noun, (void **)&xaddr);
    spec_is_true( noun == XADDR_NOUN );
    spec_is_true( xaddr->noun == 0 ); // first noun should be at surface 0
    NounSurface *ns = noun_surface_for_noun(r,0);
    spec_is_true(strcmp("FIRST_SPEC",&ns->label) == 0);
    spec_is_true(ns->specXaddr.key == ROOT);
    spec_is_true(ns->specXaddr.noun == CSPEC_NOUN);
}

int main(int argc, const char **argv) {
    printf("Running all tests...\n\n");
    testStack();
    testCspecInstanceNew();
    testInit();

    testInt();
    testPoint();
    testLine();
    testArray();

    test_xaddr_dump();
    test_stack_dump();

//    testInc();
//    testAdd();
//    testSemFault();
//    testSymbolPath();
//    testRun();
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
