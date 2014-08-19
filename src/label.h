#ifndef _CEPTR_LABEL_H
#define _CEPTR_LABEL_H
#include <stdint.h>
#include "uthash.h"

typedef uint32_t Label;
#define MAX_PATH_DEPTH 10

struct table_elem {
    Label label;                    /* key */
    int path[MAX_PATH_DEPTH];
    UT_hash_handle hh;         /* makes this structure hashable */
};
typedef struct table_elem table_elem;
typedef table_elem *LabelTable;

Label str2label(char *);
void labelSet(LabelTable *table, char *,int *path);
void _labelSet(LabelTable *table, Label l,int *path);
int *labelGet(LabelTable *table, char * l);
int *_labelGet(LabelTable *table, Label l);

void lableTableFree(LabelTable *table);

#endif
