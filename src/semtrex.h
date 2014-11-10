/**
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
 *
 * @copyright Copyright (C) 2013-2014, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 */


#ifndef _CEPTR_SEMTREX_H
#define _CEPTR_SEMTREX_H

#include "tree.h"

enum StateType {StateSymbol,StateSymbolExcept,StateAny,StateValue,StateSplit,StateMatch,StateGroupOpen,StateGroupClose,StateDescend,StateWalk,StateNot};
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
    TransitionDown=1,        ///< move to child in the match tree
};
typedef int TransitionType;

#define GroupOpen 0x1000
#define GroupClose 0

typedef struct SState SState;

/**
 * data for group open FSA state
 */
typedef struct SgroupOpen {
    SemanticID symbol;  ///< the symbol that describes the group semantically
    int uid;             ///< unique id for the group
} SgroupOpen;

/**
 * data for group close FSA state
 */
typedef struct SgroupClose {
    SState *openP;      /// pointer to the Open state
} SgroupClose;


typedef struct Svalue {
    int not;
    T *value;
} Svalue;
/**
 * Different state types need to store different kinds of values so we put them in a union
 *
 */
typedef union STypeData
{
    Symbol symbol;  ///< Symbol to match on for StateSymbol type states
    Svalue value;   ///< Value data to match on for StateValue type states
    SgroupOpen groupo;   ///< Group data for matching for StateGroup type states
    SgroupClose groupc;   ///< Group data for matching for StateGroup type states
} STypeData;

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

SState * _stx_makeFA(T *s,int *statesP);
void _stx_freeFA(SState *s);
int _t_match(T *semtrex,T *t);
int G_debug_match;
Defs *G_d;
T *G_ts,*G_te;
int _t_matchr(T *semtrex,T *t,T **r);
T *_t_get_match(T *result,Symbol group);
T *_t_embody_from_match(Defs *defs,T *match,T *t);
char * _dump_semtrex(Defs defs,T *s,char *buf);
T *makeASCIITree(char *c);
T *parseSemtrex(Defs *d,char *stx);

T *__stxcv(T *stxx,char c);
T *__stxcvm(T *stxx,int not,int count,...);

T *asciiT_toi(T* asciiT,T* match,T *t,Symbol s);
T *asciiT_tos(T* asciiT,T* match,T *t,Symbol s);
T *asciiT_toc(T* asciiT,T* match,T *t,Symbol s);


#define DT(l,t) puts("\n" #l ":");__t_dump(0,t,0,buf);puts(buf);

#endif

/** @}*/
