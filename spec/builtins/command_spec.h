#include "../../src/ceptr.h"

int cmd_func(Receptor *r) {
    stack_push(r, CSTRING_NOUN, "cmd_func");
}

void testCommand() {
    Receptor tr;init(&tr);Receptor *r = &tr;

    Xaddr cmdPatternSpecXaddr = command_init(r);

    Symbol CMD = preop_new_noun(r, cmdPatternSpecXaddr, "CMD");

    Xaddr my_c = make_command(r,CMD,"go","g",cmd_func);

    char *s = surface_for_xaddr(r,my_c);
    spec_is_str_equal(s,"go");

    Symbol nounType;
    ElementSurface *ps = spec_surface_for_noun(r, &nounType, CMD);

    void *fs =_pattern_get_child_surface(ps,s,getSymbol(r,"CMD_PROCESS"));
    spec_is_long_equal(*(long *)fs,cmd_func);

    op_invoke(r,my_c,RUN);
    char *peek_surface;
    stack_peek(r, CSTRING_NOUN, (void **)&peek_surface);
    spec_is_true( strcmp("cmd_func", peek_surface) == 0);

}
