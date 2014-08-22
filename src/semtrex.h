/**
 * @file semtrex.h
 * @brief Semantic tree regular expressions
 */

#ifndef _CEPTR_SEMTREX_H
#define _CEPTR_SEMTREX_H

#include "tree.h"

enum FSAStates {StateSymbol,StateAny,StateValue,StateSplit,StateMatch,StateGroup};
typedef int StateType;

/**
 * The possible transitions in the match tree when advancing through states in the FSA.
 *
 * In an old fashioned regex, the transition is implicit because it's
 * always "NextCharacter" for semtrex we need to expand the possibilities.
 * NOTE: the actual value stored in the SState structure may be a negative number less
 * than -1, because you can pop up multiple level, but you never pop down more than once
 */
enum FSAStateTransitions {
    TransitionNextChild=0,  ///< advance to next sibling in the match tree
    TransitionUp=-1,        ///< move up the tree in the match tree
    TransitionDown=1        ///< move to child in the match tree
};
typedef int TransitionType;

typedef void *ValueData;

#define GroupOpen 0x1000
#define GroupClose 0

/**
 * The Svalue represents the surface of a semtrex value literal.
 *
 * "value" is not a pointer to another memory location but just a key to the beginning for where the value starts in this struct.
 * ValueData is not a pointer to some other memory space because in general, tree surfaces have to hold the actual data, not pointers
 * to other parts of memory, because they can't be followed by ceptr utilities that manipulate trees, which include serialization, freeing,
 * cloning, etc.
 */
struct Svalue {
    Symbol symbol;      ///< Symbol is the semantic type.  Here we're storing the integer that represents the semantic type. For example, FIRST_NAME
    size_t length;	///< For example, 5 (including terminating null)
    ValueData value;	///< For example, "t\0" which fits in an "int".  Otherwise, Svalue needs to be malloc'd to hold extra bytes.
};
typedef struct Svalue Svalue;

struct Sgroup {
    int id;
    int type;           ///< is this an open or close group state
};
typedef struct Sgroup Sgroup;

/**
 * Different state types need to store different kinds of values so we put them in a union
 *
 * @note cool coding hack: the value item of this list include a Symbol as it's first items so that we can share the code that matches on symbol!!
 */
typedef union STypeData STypeData;
union STypeData
{
    Symbol symbol;  ///< Symbol to match on for StateSymbol type states
    Svalue value;   ///< Value data to match on for StateValue type states
    Sgroup group;   ///< Group data for matching for StateGroup type states
};

struct SState {
    StateType type;
    struct SState *out;
    TransitionType transition;  ///< will be: TransitionNextChild=0,TransitionUp=-1,TransitionDown=1
    struct SState *out1;
    int _did; ///< used only for freeing and printing out FSA to prevent looping.
    STypeData data;
};
typedef struct SState SState;

SState * _s_makeFA(Tnode *s,int *statesP);
void _s_freeFA(SState *s);
int _t_match(Tnode *semtrex,Tnode *t);
int _t_matchr(Tnode *semtrex,Tnode *t,Tnode **r);
Tnode *_t_get_match(Tnode *result,Symbol group);
#endif
