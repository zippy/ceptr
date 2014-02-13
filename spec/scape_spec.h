#include "../src/ceptr.h"

bool str_match() {
}

void testNewScape() {
    Receptor tr;init(&tr);Receptor *r = &tr;
    Xaddr cmdPatternSpecXaddr = command_init(r);
    Symbol CMD = preop_new_noun(r, cmdPatternSpecXaddr, "CMD");

    Symbol CMD_STR = getSymbol(r,"CMD_STR");

    Scape *s = _new_scape("command_text",CMD,CMD_STR,CSTRING_NOUN,str_match);
    spec_is_str_equal(s->name,"command_text");
    spec_is_equal(s->key_source,CMD_STR);
    spec_is_equal(s->data_source,CMD);
    spec_is_equal(s->key_geometry,CSTRING_NOUN);
    spec_is_long_equal((long)s->matchfn,(long)str_match);

    Xaddr my_c = make_command(r,CMD,"go","g",0);
    Xaddr c = scape_lookup(s,"go");
    spec_is_equal(my_c.noun,c.noun);
    spec_is_equal(my_c.key,c.key);
}

void testScape() {
    testNewScape();
}
