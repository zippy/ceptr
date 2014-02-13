#include "../src/ceptr.h"

bool str_match() {
}

void testScapeLookup() {
    Receptor tr;init(&tr);Receptor *r = &tr;
    Xaddr cmdPatternSpecXaddr = command_init(r);
    Symbol CMD = preop_new_noun(r, cmdPatternSpecXaddr, "CMD");

    Symbol CMD_STR = getSymbol(r,"CMD_STR");
    Scape *s = _new_scape("scape_name",CMD,CMD_STR,CSTRING_NOUN,str_match);

    Xaddr my_c = make_command(r,CMD,"go","g",0);
    Xaddr c = scape_lookup(s,"go");
    spec_is_equal(my_c.noun,c.noun);
    spec_is_equal(my_c.key,c.key);

}

void testNewScape() {
    Scape *s = _new_scape("scape_name",1,2,3,str_match);
    spec_is_str_equal(s->name,"scape_name");
    spec_is_equal(s->data_source,1);
    spec_is_equal(s->key_source,2);
    spec_is_equal(s->key_geometry,3);
    spec_is_ptr_equal((void *)s->matchfn,(void *)str_match);
}

void testScape() {
    testNewScape();
    testScapeLookup();
}
