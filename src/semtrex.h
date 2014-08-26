/**
 * @copyright Copyright (C) 2013-2014, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 *
 * @defgroup semtrex Semantic Tree Regular Expressions
 *
 * @brief Semtrex provides a language for pattern matching on semtantic trees

 Here is a pseudo BNF (what we are calling a TNF for Tree Normal Form)
 that describes how Semtrexs can be constructed:

 @code
 Semtrex ::= Root [ / Child]
 Root ::= Literal | Value | Or | Any | Question | Group
 Child ::=  Root | OneOrMore | ZeroOrMore | ZeroOrOne | Sequence
 OneOrMore ::= "+" / Semtrex
 ZeroOrMore ::= "*" / Semtrex
 ZeroOrOne ::= "?" / Semtrex
 Sequence ::= "," / Semtrex [, Semtrex]...
 Or ::= "|" / Semtrex1 , Semtrex2
 Literal ::= "LITERAL(<symbol>)" [ / Semtrex]
 Value ::= "VALUE(<symbol>)=<value>" [ / Semtrex]
 Any ::= "." [ / Semtrex]
 Group ::= "{<symbol>}" / Semtrex
 @endcode
 * @{

 * @file semtrex.h
 * @brief Semantic tree regular expression header file
 */


#ifndef _CEPTR_SEMTREX_H
#define _CEPTR_SEMTREX_H

#include "tree.h"

enum StateType {StateSymbol,StateAny,StateValue,StateSplit,StateMatch,StateGroup};
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

/**
 * data for group FSA state
 */
struct Sgroup {
    int id;             ///< the symbol that identifies this group
    int type;           ///< is this an open or close group state
};
typedef struct Sgroup Sgroup;

/**
 * Different state types need to store different kinds of values so we put them in a union
 *
 * @note cool coding hack: the value item of this list include a Symbol as it's first items so that we can share the code that matches on symbol!!
 */
union STypeData
{
    Symbol symbol;  ///< Symbol to match on for StateSymbol type states
    Svalue value;   ///< Value data to match on for StateValue type states
    Sgroup group;   ///< Group data for matching for StateGroup type states
};
typedef union STypeData STypeData;

/**
 * This struct holds the data for each state in of the FSA generated to match a tree
 */
struct SState {
    StateType type;             ///< what type of state this is
    struct SState *out;         ///< which state to go to next
    TransitionType transition;  ///< will be: TransitionNextChild=0,TransitionUp=-1,TransitionDown=1
    struct SState *out1;        ///< which alternate state to go to next in the case this is a Split state
    int _did;                   ///< used to hold a mark when freeing and printing out FSA to prevent looping.
    STypeData data;             ///< a union to hold the data for which ever type of SState this is
};
typedef struct SState SState;

SState * _s_makeFA(Tnode *s,int *statesP);
void _s_freeFA(SState *s);
int _t_match(Tnode *semtrex,Tnode *t);
int _t_matchr(Tnode *semtrex,Tnode *t,Tnode **r);
Tnode *_t_get_match(Tnode *result,Symbol group);

#endif

/** @}*/
