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
	s->items = _t_new_root();
    }
    return s;
}

int scape_count(Receptor *r) {
    return _t_children(r->scapes);
}

#define SCAPE_NOUN -105

ScapeID new_scape(Receptor *r,char *name,Symbol data_source,Symbol key_source, Symbol key_geometry, ScapeMatchFn matchfn) {
    Scape *s = _new_scape(name,data_source,key_source,key_geometry,matchfn);
    Tnode *t = _t_new(r->scapes,SCAPE_NOUN,s,sizeof(Scape));
    free(s); //TODO: it would be better if this didn't require two allocs.
    return scape_count(r);
}

void _delete_scape_items(Scape *s) {
    _t_free(s->items);
}

Scape *get_scape(Receptor *r,ScapeID i) {
    return _t_get_child_surface(r->scapes,i);
}

void delete_scapes(Receptor *r) {
    for (int i=1; i<=scape_count(r);i++) {
	Scape *s = get_scape(r,i);
	_delete_scape_items(s);
    }
    _t_free(r->scapes);
}

int scape_item_count(Scape *s) {
    return _t_children(s->items);
}

//TODO: refactor same as get_scape
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


void init_scapes(Receptor *r) {
    r->scapes = _t_new_root();
}

#define SCAPE_ITEM_NOUN -107
void _add_scape_item(Scape *s,void *surface,size_t surface_len,Xaddr xaddr) {
    ScapeItem *si = malloc(sizeof(ScapeItem)+surface_len);
    if (si != NULL) {
	si->xaddr = xaddr;
	si->surface_len = surface_len;
	memcpy(&si->surface,surface,surface_len);
	_t_new(s->items,SCAPE_ITEM_NOUN,si,sizeof(ScapeItem)+surface_len);
    }
}
