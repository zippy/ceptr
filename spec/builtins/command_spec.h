#include "../../src/ceptr.h"


void testCommand() {
    Receptor tr;init(&tr);Receptor *r = &tr;

    Xaddr cmdPatternSpecXaddr = command_init(r);
    char buf[1000];

    Symbol CMD = preop_new_noun(r, cmdPatternSpecXaddr, "CMD");
    Symbol CMD_STR = getSymbol(r,"CMD_STR");
    Symbol CMD_ALIAS_STR = getSymbol(r,"CMD_ALIAS_STR");

    Symbol nounType;
    ElementSurface *ps = spec_surface_for_noun(r, &nounType, CMD);

    _pattern_set_child(r,ps,buf,CMD_STR,"go");
    _pattern_set_child(r,ps,buf,CMD_ALIAS_STR,"g");
    stack_push(r,CMD,buf);
    op_new(r);
    Xaddr my_c;
    stack_pop(r,XADDR_NOUN,&my_c);
    char *s = surface_for_xaddr(r,my_c);
    spec_is_str_equal(s,"go");
}
