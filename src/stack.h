#include "ceptr.h"


void stack_peek_unchecked(Receptor *r, Symbol *name, void **surface) {
    assert( _t_children(r->sem_stack) > 0);
    int last = _t_children(r->sem_stack);
    SemStackFrame *ssf = _t_get_child_surface(r->sem_stack,last);
    *name = ssf->noun;
    *surface = &ssf->surface;
}

void stack_pop_unchecked(Receptor *r, Symbol *name, void *surface) {
    int last = _t_children(r->sem_stack);
    Tnode *s = _t_get_child(r->sem_stack,last);
    SemStackFrame *ssf = _t_surface(s);
    *name = ssf->noun;
    memcpy(surface, &ssf->surface, ssf->size);
    _t_free(s);
    r->sem_stack->child_count--;
}

void stack_peek(Receptor *r, Symbol expectedNoun, void **surface){
    Symbol actualNoun;
    stack_peek_unchecked(r, &actualNoun, surface);
    assert( actualNoun == expectedNoun );
}

void stack_pop(Receptor *r, Symbol expectedNoun, void *surface) {
    Symbol actualNoun;
    stack_pop_unchecked(r, &actualNoun, surface);
    assert( actualNoun == expectedNoun );
}

void stack_push(Receptor *r, Symbol name, void *surface) {
    size_t size = size_of_named_surface(r, name, surface);
    Tnode *s = _t_new(r->sem_stack,SEMSTACK_FRAME_NOUN,0,size+sizeof(SemStackFrame)-sizeof(int));

    SemStackFrame *ssf = _t_surface(s);
    ssf->noun = name;
    ssf->size = size;
    memcpy(&ssf->surface, surface, size);
}

/*
void stack_dump(Receptor *r) {
    int i, v = 0;
    char *unknown = "<unknown>";
    char *label;
    ElementSurface *ps;
    SemStackFrame *ssf;
    for (i = 0; i <= r->semStackPointer; i++) {
        ssf = &r->semStack[i];
        printf("\nStack frame: %d is a %s(%d) size:%d\n", i, label_for_noun(r, ssf->noun), ssf->noun, (int)ssf->size);
        printf("\n");
    }
    }*/

void stack_init(Receptor *r) {
    r->sem_stack = _t_new_root(SEMSTACK_FRAMES_ARRAY_NOUN);
}

void stack_free(Receptor *r) {
    _t_free(r->sem_stack);
}
