#ifndef _CEPTR_LABEL_H
#define _CEPTR_LABEL_H
#include <stdint.h>

typedef uint32_t Label;

Label str2label(char *);
void labelSet(char *,int *path);
void _labelSet(Label l,int *path);
int *labelGet(char * l);
int *_labelGet(Label l);

#endif
