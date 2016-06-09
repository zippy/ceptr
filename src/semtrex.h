/**
 * @defgroup semtrex Semantic Tree Regular Expressions
 *
 * @brief Semtrex provides a language for pattern matching on semantic trees similar to regular expressions for matching on text strings.
 * @{

 * @file semtrex.h
 * @brief Semantic tree regular expression header file
 *
 * @copyright Copyright (C) 2013-2016, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 */


#ifndef _CEPTR_SEMTREX_H
#define _CEPTR_SEMTREX_H

#include "tree.h"

enum StateType {StateSymbol,StateAny,StateValue,StateSplit,StateMatch,StateGroupOpen,StateGroupClose,StateDescend,StateWalk,StateNot};
typedef int StateType;

/**
 * The possible transitions in the match tree when advancing through states in the FSA.
 *
 * In an old fashioned regex, the transition is implicit because it's
 * always "NextCharacter". For semtrex we need to expand the possibilities.
 * NOTE: the actual value stored in the SState structure may be a negative number less
 * than -1, because you can pop up multiple level, but you never pop down more than once
 */
enum {TransitionDown=1,TransitionNone=0x8000};
#define isTransitionPop(t) (t<0)
#define isTransitionNext(t) (t==0)

typedef int TransitionType;

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

#define LITERAL_NOT 0x01
#define LITERAL_SET 0x02
typedef struct Svalue {
    int flags;
    T *values;
} Svalue;

typedef struct Sliteral {
    int flags;
    T *symbols;
} Sliteral;


/**
 * Different state types need to store different kinds of values so we put them in a union
 *
 */
typedef union STypeData
{
    Sliteral symbol;  ///< Symbol to match on for StateSymbol type states
    Svalue value;   ///< Value data to match on for StateValue type states
    SgroupOpen groupo;   ///< Group data for matching for StateGroup type states
    SgroupClose groupc;   ///< Group data for matching for StateGroup type states
} STypeData;

/**
 * This struct holds the data for each state of the FSA generated to match a tree
 * NOTE: the transition MUST come right after the out value or it will break the trick
 * in patch() which allows us to get the right popping value
 */
struct SState {
    struct SState *out;         ///< which state to go to next
    TransitionType transition;  ///< will be: TransitionNextChild=0,TransitionDown=1, or a negative number which means pop up that many levels plus advance to next child.
    StateType type;             ///< what type of state this is
    struct SState *out1;        ///< which alternate state to go to next in the case this is a Split state
    TransitionType transition1; ///< will be: TransitionNextChild=0,TransitionUp=-1,TransitionDown=1
    StateType type_;             ///< copy of state type needed for patch to grab (far too tricky)

    int _did;                   ///< used to hold a mark when freeing and printing out FSA to prevent looping.
    STypeData data;             ///< a union to hold the data for which ever type of SState this is
};
SState *G_cur_stx_state;  // global for highlighting the current state when doing an stx FSA dump

SState * _stx_makeFA(T *s,int *statesP);
void _stx_freeFA(SState *s);
int _t_match(T *semtrex,T *t);
int _t_matchr(T *semtrex,T *t,T **r);
T *_stx_get_matched_node(Symbol s,T *match_results,T *match_tree,int *sibs);
void _stx_replace(T *semtrex,T *t,T *replace);
T *_t_get_match(T *result,Symbol group);
T *__t_embody_from_match(SemTable *sem,T *match,T *t);
T *_t_embody_from_match(SemTable *sem,T *match,Symbol group,T *t);
char * _dump_semtrex(SemTable *sem,T *s,char *buf);
T *makeASCIITree(char *c);
T *parseSemtrex(SemTable *sem,char *stx);
T *_stx_results2sem_map(SemTable *sem,T *match_results,T *match_tree);

T *__stxcv(T *stxx,char c);
T *__stxcvm(T *stxx,int not,int count,...);

T *asciiT_toi(T* asciiT,T* match,T *t,Symbol s);
T *asciiT_tol(T* asciiT,T* match,T *t,Symbol s);
T *asciiT_tof(T* asciiT,T* match,T *t,Symbol s);
T *asciiT_tos(T* asciiT,T* match,T *t,Symbol s);
T *asciiT_toc(T* asciiT,T* match,T *t,Symbol s);

T *wrap(T *tokens,T *results, Symbol contents_s, Symbol open_s);

/// macro to add a single symbol literal to semtrex tree
#define _sl(t,s) __sl(t,false,1,s)

/// macro to add a single symbol literal not to semtrex tree
#define _sln(t,s) __sl(t,true,1,s)

T *__sl(T *p, bool not,int count, ...);

/// debugging macro for quickly dumping out a semtrex text string
#define DS(l,t) {char buf[1000];puts("\n" #l ":");_dump_semtrex(G_sem,t,buf);puts(buf);}

#endif

void __stx_dump(SState *s,char *buf);
char * _stx_dump(SState *s,char *buf);
void stx_dump(T *s);
char G_stx_dump_buf[10000];
/** @}*/
