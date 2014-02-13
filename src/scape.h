#include "ceptr.h"

typedef bool (* ScapeMatchFn)();

typedef struct {
    char name[255];
    Symbol data_source;
    Symbol key_source;
    Symbol key_geometry;
    ScapeMatchFn matchfn;
} Scape;

Scape *_new_scape(char *name,Symbol data_source,Symbol key_source, Symbol key_geometry, ScapeMatchFn matchfn) {
    Scape *s = malloc(sizeof(Scape));
    if (s != NULL) {
	strcpy(s->name,name); //TODO: make more dynamic
	s->data_source = data_source;
	s->key_source = key_source;
	s->key_geometry = key_geometry;
	s->matchfn = matchfn;
    }
    return s;

}

Xaddr scape_lookup(Scape *s,void *key_surface) {
    Xaddr x;
    return x;
}
