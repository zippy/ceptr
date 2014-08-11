#ifndef _CEPTR_SEMTREX_H
#define _CEPTR_SEMTREX_H

#include "tree.h"

enum {StateSymbol,StateAny,StateValue,StateSplit,StateMatch,StateGroup};
typedef int StateType;

enum {TransitionNextChild=0,TransitionUp=-1,TransitionDown=1};
typedef int TransitionType;

typedef void *ValueData;

#define GroupOpen 0x1000
#define GroupClose 0



// The Svalue represents the surface of a semtrex value literal.
// "value" is not a pointer to another memory location but just a key to the beginning for where the value starts in this struct.
// ValueData is not a pointer to some other memory space because in general, tree surfaces have to hold the actual data, not pointers
// to other parts of memory, because they can't be followed by ceptr utilities that manipulate trees, which include serialization, freeing,
// cloning, etc.
struct Svalue {
    Symbol symbol;      // Symbol is the semantic type.  Here we're storing the integer that represents the semantic type.
			// For example, FIRST_NAME
    size_t length;	// For example, 5 (including terminating null)
    ValueData value;	// For example, "t\0" which fits in an "int".  Otherwise, Svalue needs to be malloc'd to hold extra bytes.
};
typedef struct Svalue Svalue;


struct Sgroup {
    int id;
    int type;           // is this an open or close group state
};
typedef struct Sgroup Sgroup;

// different state types need to store different kinds of values so
// we put them in a union/
// NB cool coding hack: the value item of this list include a Symbol as it's first items so that we can share the code that matches on symbol!!
typedef union STypeData STypeData;
union STypeData
{
    Symbol symbol;  // Symbol to match on for StateSymbol type states
    Svalue value;   // Value data to match on for StateValue type states
    Sgroup group;   // Group data for matching for StateGroup type states
};

struct SState {
    StateType type;
    struct SState *out;
    TransitionType transition;
    struct SState *out1;
    int lastlist;
    int _did; // used only for printing out state diagrams to prevent looping.
    STypeData data;
};
typedef struct SState SState;

SState * _s_makeFA(Tnode *s,int *statesP);
void _s_freeFA(SState *s);
int _t_match(Tnode *semtrex,Tnode *t);
int _t_matchr(Tnode *semtrex,Tnode *t,Tnode **r);
Tnode *_t_get_match(Tnode *result,Symbol group);
#endif
