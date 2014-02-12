#include "../ceptr.h"


int proc_command_print(Receptor *r, Symbol noun, void *this,void *ps){
    printf("String: %s; Alias:%s; Func:0x%08x",
	   (char *)_pattern_get_child_surface(ps, this,getSymbol(r,"CMD_STR")),
	   (char *)_pattern_get_child_surface(ps, this,getSymbol(r,"CMD_ALIAS_STR")),
	   (long *)_pattern_get_child_surface(ps, this,getSymbol(r,"CMD_PROCESS"))
	   );
}

int proc_command_run(Receptor *r, Symbol noun, void *this, void *ps){
    //TODO: gotta be a better way to get the symbol
    cfuncFn *fn =  _pattern_get_child_surface(ps, this,getSymbol(r,"CMD_PROCESS"));
    return (*fn)(r);
}

Xaddr make_command(Receptor *r,Symbol cmd_noun,char *str, char* alias, cfuncFn fn) {
    Symbol CMD_STR = getSymbol(r,"CMD_STR");
    Symbol CMD_ALIAS_STR = getSymbol(r,"CMD_ALIAS_STR");
    Symbol CMD_PROCESS = getSymbol(r,"CMD_PROCESS");

    Symbol nounType;
    ElementSurface *ps = spec_surface_for_noun(r, &nounType, cmd_noun);
    char buf[1000];

    _pattern_set_child(r,ps,buf,CMD_STR,str);
    _pattern_set_child(r,ps,buf,CMD_ALIAS_STR,alias);
    _pattern_set_child(r,ps,buf,CMD_PROCESS,&fn);
    stack_push(r,cmd_noun,buf);
    op_new(r);
    Xaddr c;
    stack_pop(r,XADDR_NOUN,&c);
    return c;
}

Xaddr command_init(Receptor *r) {

    Xaddr CMD_STR = new_noun(r, r->str255PatternSpecXaddr, "CMD_STR");
    Xaddr CMD_ALIAS_STR = new_noun(r, r->str255PatternSpecXaddr, "CMD_ALIAS_STR");
    Xaddr CMD_PROCESS = new_noun(r, r->cfuncPatternSpecXaddr, "CMD_PROCESS");

    Process processes[] = {
        {PRINT, (processFn)proc_command_print},
        {RUN, (processFn)proc_command_run},
    };

    Xaddr children[3] = {CMD_STR,CMD_ALIAS_STR,CMD_PROCESS};
    PatternSpecData psd = {
        "COMMAND", 3, children, 2, processes
    };

    stack_push(r, PATTERN_SPEC_DATA_NOUN, &psd);
    op_invoke(r, r->patternSpecXaddr, INSTANCE_NEW);
    Xaddr x;
    stack_pop(r, XADDR_NOUN, &x);
    return x;
}
