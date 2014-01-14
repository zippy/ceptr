#include "../src/ceptr.h"

void testStack() {
    Receptor tr;Receptor *r = &tr;
    Symbol noun;
    char *peek_surface;
    stack_init(r);
    spec_is_true(r->valStackPointer == 0);
    spec_is_true(r->semStackPointer == -1);
    stack_push(r, CSTRING_NOUN, "Hello, Stack");
    spec_is_true(r->valStackPointer == 13);
    spec_is_true(r->semStackPointer == 0);

    stack_peek_unchecked(r, &noun, (void **)&peek_surface);
    spec_is_true( noun == CSTRING_NOUN );
    spec_is_true( strcmp("Hello, Stack", peek_surface) == 0);

    stack_peek(r, CSTRING_NOUN, (void **)&peek_surface);
    spec_is_true( strcmp("Hello, Stack", peek_surface) == 0);

    char pop_surface[BUFFER_SIZE];
    stack_pop(r, CSTRING_NOUN, pop_surface);
    spec_is_true( strcmp("Hello, Stack", pop_surface) == 0 );
    spec_is_true(r->valStackPointer == 0);
    spec_is_true(r->semStackPointer == -1);
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
    stack_dump(r);
}

