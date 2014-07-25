#ifndef _CEPTR_SEMTREX_H
#define _CEPTR_SEMTREX_H

#include "tree.h"

enum {StateSymbol,StateSplit,StateMatch};

typedef int StateType;

enum {TransitionNextChild=0,TransitionUp=-1,TransitionDown=1};
typedef int TransitionType;


struct SState {
    StateType type;
    TransitionType transition;
    Symbol symbol;
    struct SState *out;
    struct SState *out1;
    int lastlist;
};
typedef struct SState SState;

SState * _s_makeFA(Tnode *s,int *statesP);
void _s_freeFA(SState *s);
int _t_match(Tnode *semtrex,Tnode *t);

#endif
