#include "ceptr.h"

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

ScapeID new_scape(Receptor *r,char *name,Symbol data_source,Symbol key_source, Symbol key_geometry, ScapeMatchFn matchfn) {
    if (r->scape_count == MAX_SCAPES) {raise_error0("No more scapes can be allocated\n");}
    r->scapes[r->scape_count] = _new_scape(name,data_source,key_source,key_geometry,matchfn);
    return r->scape_count++;
}

void delete_scapes(Receptor *r) {
    while(r->scape_count-- > 0) {
	free(r->scapes[r->scape_count]);
    }
}

Scape *get_scape(Receptor *r,ScapeID i) {
    return r->scapes[i];
}

Xaddr scape_lookup(Scape *s,void *key_surface) {
    Xaddr x;
    return x;
}

void init_scapes(Receptor *r) {
    r->scape_count = 0;
}
