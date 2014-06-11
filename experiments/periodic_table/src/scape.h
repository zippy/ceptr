#include "ceptr.h"

bool str_match(void *match_surface,size_t match_len, void *key_surface, size_t key_len) {
    char *m = (char *)match_surface;
    char *k = (char *)key_surface;
    //    printf("comparing: %s to %s\n",m,k);
    return (strcmp(m,k) == 0);
}

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
	s->items = _t_new_root(SCAPE_ITEMS_ARRAY_NOUN);
    }
    return s;
}

int scape_count(Data *d) {
    return _t_children(d->scapes);
}

ScapeID new_scape(Data *d,char *name,Symbol data_source,Symbol key_source, Symbol key_geometry, ScapeMatchFn matchfn) {
    Scape *s = _new_scape(name,data_source,key_source,key_geometry,matchfn);
    Tnode *t = _t_new(d->scapes,SCAPE_NOUN,s,sizeof(Scape));
    free(s); //TODO: it would be better if this didn't require two allocs.
    return scape_count(d);
}

void _scape_items_free(Scape *s) {
    _t_free(s->items);
}

Scape *get_scape(Data *d,ScapeID i) {
    return _t_get_child_surface(d->scapes,i);
}

void scapes_free(Data *d) {
    for (int i=1; i<=scape_count(d);i++) {
	Scape *s = get_scape(d,i);
	_scape_items_free(s);
    }
    _t_free(d->scapes);
}

int scape_item_count(Scape *s) {
    return _t_children(s->items);
}

ScapeItem *get_scape_item(Scape *s,int id) {
    return _t_get_child_surface(s->items,id);
}

Xaddr _scape_lookup(Scape *s,void *match_surface,size_t match_len) {
    ScapeMatchFn f = s->matchfn;
    for(int i=1;i<=scape_item_count(s);i++) {
	ScapeItem *si = get_scape_item(s,i);
	if ((*f)(match_surface,match_len,&si->surface,si->surface_len))
	    return si->xaddr;
    }
    Xaddr x = {-1,-1};
    return x;
}

void scapes_init(Data *d) {
    d->scapes = _t_new_root(SCAPES_ARRAY_NOUN);
}

void _add_scape_item(Scape *s,void *surface,size_t surface_len,Xaddr xaddr) {
    ScapeItem *si = malloc(sizeof(ScapeItem)+surface_len);
    if (si != NULL) {
	si->xaddr = xaddr;
	si->surface_len = surface_len;
	memcpy(&si->surface,surface,surface_len);
	_t_new(s->items,SCAPE_ITEM_NOUN,si,sizeof(ScapeItem)+surface_len);
    }
}
