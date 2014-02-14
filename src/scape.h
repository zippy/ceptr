#include "ceptr.h"

bool surface_match(void *match_surface,size_t match_len, void *key_surface, size_t key_len) {
    char *m = (char *)match_surface;
    char *k = (char *)key_surface;
    if (key_len != match_len) return false;
    if (key_len == 0) return true;
    while (key_len-- > 0) {
	if (*(m++) != (*(k++))) return false;
    }
    return true;
}

Scape *_new_scape(char *name,Symbol data_source,Symbol key_source, Symbol key_geometry, ScapeMatchFn matchfn) {
    Scape *s = malloc(sizeof(Scape));
    if (s != NULL) {
	strcpy(s->name,name); //TODO: make more dynamic
	s->data_source = data_source;
	s->key_source = key_source;
	s->key_geometry = key_geometry;
	s->matchfn = (matchfn == 0)?surface_match:matchfn;
	s->item_count = 0;
    }
    return s;
}

ScapeID new_scape(Receptor *r,char *name,Symbol data_source,Symbol key_source, Symbol key_geometry, ScapeMatchFn matchfn) {
    if (r->scape_count == MAX_SCAPES) {raise_error0("No more scapes can be allocated\n");}
    r->scapes[r->scape_count] = _new_scape(name,data_source,key_source,key_geometry,matchfn);
    return r->scape_count++;
}

void _delete_scape_items(Scape *s) {
    while(s->item_count-- > 0) {
	free(s->items[s->item_count]);
    }
}

void delete_scapes(Receptor *r) {
    while(r->scape_count-- > 0) {
	Scape *s = r->scapes[r->scape_count];
	_delete_scape_items(s);
	free(s);
    }
}

Scape *get_scape(Receptor *r,ScapeID i) {
    return r->scapes[i];
}

Xaddr _scape_lookup(Scape *s,void *match_surface,size_t match_len) {
    ScapeMatchFn f = s->matchfn;
    for(int i=0;i<s->item_count;i++) {
	ScapeItem *si = s->items[i];
	if ((*f)(match_surface,match_len,&si->surface,si->surface_len))
	    return si->xaddr;
    }
    Xaddr x = {-1,-1};
    return x;
}

void init_scapes(Receptor *r) {
    r->scape_count = 0;
}

void _add_scape_item(Scape *s,void *surface,size_t surface_len,Xaddr xaddr) {
    if (s->item_count == MAX_SCAPE_ITEMS) {raise_error0("No more scape items can be allocated\n");}
    ScapeItem *si = malloc(sizeof(ScapeItem)+surface_len);
    if (si != NULL) {
	si->xaddr = xaddr;
	si->surface_len = surface_len;
	memcpy(&si->surface,surface,surface_len);
	s->items[s->item_count] = si;
	s->item_count++;
    }
}
