#include "../../src/ceptr.h"

void testPatternHelpers() {
    Receptor tr; init(&tr); Receptor *r = &tr;

    //**** pattern_get_size
    Symbol MY_INT = preop_new_noun(r, r->intPatternSpecXaddr, "MY_INT");
    spec_is_equal(pattern_get_size(r,MY_INT,0),4); // don't need an actual surface to get a pattern's size


    //*** pattern child functions
    Xaddr cmdPatternSpecXaddr = command_init(r);
    char buf[1000];

    Symbol CMD = preop_new_noun(r, cmdPatternSpecXaddr, "CMD");
    Symbol CMD_STR = getSymbol(r,"CMD_STR");
    Symbol CMD_ALIAS_STR = getSymbol(r,"CMD_ALIAS_STR");

    Symbol nounType;
    ElementSurface *ps = spec_surface_for_noun(r, &nounType, CMD);

    spec_is_equal(_pattern_child_offset(ps,CMD_STR),0);
    spec_is_equal(_pattern_child_offset(ps,CMD_ALIAS_STR),256);
    _pattern_set_child(r,ps,buf,CMD_STR,"go");
    _pattern_set_child(r,ps,buf,CMD_ALIAS_STR,"g");
    spec_is_str_equal(_pattern_get_child_surface(ps,buf,CMD_STR),"go");
    spec_is_str_equal(_pattern_get_child_surface(ps,buf,CMD_ALIAS_STR),"g");

}

//TODO: fill out tests
void testPattern(){
    testPatternHelpers();
}
