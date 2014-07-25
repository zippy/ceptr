#include "label.h"
#include "tree.h"
#include "uthash.h"
#include "hashfn.h"

#define MAX_PATH_DEPTH 10

struct table_elem {
    Label label;                    /* key */
    int path[MAX_PATH_DEPTH];
    UT_hash_handle hh;         /* makes this structure hashable */
};

struct table_elem *table = NULL;

Label str2label(char* s) {
    return hashfn(s,strlen(s));
}

void _labelSet(Label l,int *path){
    struct table_elem *e;

    e = malloc(sizeof(struct table_elem));
    e->label = l;
    _t_pathcpy(e->path,path);

    HASH_ADD_INT(table,label,e);
}

int *_labelGet(Label l){
    struct table_elem *e;
    HASH_FIND_INT( table, &l, e );
    return e->path;
}

void labelSet(char *s,int *path) {
    _labelSet(str2label(s),path);
}

int *labelGet(char *s) {
    return _labelGet(str2label(s));
}

// TODO: label delete
