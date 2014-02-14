#include "../src/ceptr.h"

void testNewScape() {
    Receptor tr;init(&tr);Receptor *r = &tr;
    spec_is_equal(r->scape_count,0);
    ScapeID si = new_scape(r,"scape_name",1,2,3,0);
    spec_is_equal(r->scape_count,1);
    Scape *s = get_scape(r,si);
    spec_is_str_equal(s->name,"scape_name");
    spec_is_equal(s->data_source,1);
    spec_is_equal(s->key_source,2);
    spec_is_equal(s->key_geometry,3);
    spec_is_ptr_equal((void *)s->matchfn,(void *)surface_match);
}

void testScapeContents() {
    Scape *s = _new_scape("scape_name",1,2,3,0);
    Xaddr x = {1,2};
    spec_is_equal(s->item_count,0);
    _add_scape_item(s,"fish",6,x);
    spec_is_equal(s->item_count,1);
    ScapeItem *si = s->items[0];
    spec_is_long_equal((long)si->surface_len,(long)6);
    spec_is_str_equal((char*)&si->surface,"fish");
    spec_is_equal(si->xaddr.key,1);
    spec_is_equal(si->xaddr.noun,2);
}

bool str_match(void *match_surface,size_t match_len, void *key_surface, size_t key_len) {
    char *m = (char *)match_surface;
    char *k = (char *)key_surface;
    return (strcmp(m,k) == 0);
}
void testScapeLookup() {
    Receptor tr;init(&tr);Receptor *r = &tr;
    Xaddr cmdPatternSpecXaddr = command_init(r);
    Symbol CMD = preop_new_noun(r, cmdPatternSpecXaddr, "CMD");

    Symbol CMD_STR = getSymbol(r,"CMD_STR");
    ScapeID si = new_scape(r,"scape_name",CMD,CMD_STR,CSTRING_NOUN,str_match);
    Scape *s = get_scape(r,si);

    Xaddr c = _scape_lookup(s,"go",3);
    spec_is_equal(c.noun,-1);
    spec_is_equal(c.key,-1);

    Xaddr my_c = make_command(r,CMD,"go","g",0);
    c = _scape_lookup(s,"go",3);
    spec_is_equal(c.noun,my_c.noun);
    spec_is_equal(c.key,my_c.key);
}

void testScape() {
    testNewScape();
    testScapeContents();
    testScapeLookup();
}
