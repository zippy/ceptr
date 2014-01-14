#ifndef _CEPTR_H
#define _CEPTR_H

#define DEFAULT_CACHE_SIZE 100000
#define STACK_SIZE 10000
#define OPERANDS_SIZE (sizeof(Xaddr) * 2)

#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#define raise_error0(error_msg)        \
    printf(error_msg);            \
    raise(SIGINT);
#define raise_error(error_msg, val)        \
    printf(error_msg, val);            \
    raise(SIGINT);
#define raise_error2(error_msg, val1,val2)        \
    printf(error_msg, val1,val2);                \
    raise(SIGINT);

typedef int Symbol;

/*  NEXT STEPS!

* fix arrays / arrays of arrays
* finish preop_new_pattern -> op_invoke(r->patternSpecXaddr, INSTANCE_NEW) refactor

== replace calls to preop_new with op_invoke($X, INSTANCE_NEW)  (in test code)
    - making sure we have INSTANCE_NEW available on all specs

== replace calls to preop_new_noun -> op_invoke(NOUN_NOUN, INSTANCE_NEW)
    DONE:- create a NOUN_SPEC cspec with cases in appropriate switch statements

== when creating a new noun, ask the spec for the size function to add to the size_table.
e.g.
    in init_element
        - add into the ElementSurface for element we add instance size function
        - creating a noun we copy size function from the spec into the size_table for the noun.

 */


#define BUFFER_SIZE 10000

enum FunctionNames {
    INSTANCE_SIZE, INSTANCE_NEW, PRINT, INC, ADD
};

enum Symbols {
    CSPEC = -1, CSPEC_NOUN = -2, XADDR_NOUN = -3,
    CSTRING_NOUN = -4, PATTERN_SPEC_DATA_NOUN = -5,  ROOT = -100
};

typedef int FunctionName;

typedef struct {
    int key;
    Symbol noun;
} Xaddr;

typedef struct {
    Xaddr noun;
    int offset;
} Offset;

typedef struct {
    Xaddr specXaddr;
    char label;
} NounSurface;


typedef struct {
    Symbol noun;
    size_t size;
} SemStackFrame;

typedef struct {
    Xaddr xaddrs[DEFAULT_CACHE_SIZE];
    Symbol xaddr_scape[DEFAULT_CACHE_SIZE];
    char cache[DEFAULT_CACHE_SIZE];
    int current_xaddr;
    size_t cache_index;
} Data;

typedef struct {
    FunctionName name;
    int (*function)(void *);
} UntypedProcess;

typedef struct {
    Symbol name;
    int process_count;
    UntypedProcess processes;
} ElementSurface;

typedef struct {
    SemStackFrame semStack[STACK_SIZE];
    Symbol nounStack[STACK_SIZE];
    int semStackPointer;
    char valStack[DEFAULT_CACHE_SIZE];
    int valStackPointer;
    ElementSurface rootSurface;

    //built in xaddrs:
    Xaddr rootXaddr;
    Xaddr cspecXaddr;
    Xaddr nounSpecXaddr;
    Xaddr patternSpecXaddr;
    Xaddr arraySpecXaddr;
    Xaddr intPatternSpecXaddr;
    Xaddr pointPatternSpecXaddr;
    Xaddr linePatternSpecXaddr;

    Data data;
} Receptor;


typedef struct {
    FunctionName name;
    int (*function)(Receptor *);
} YagnArgsProcess;

typedef struct {
    FunctionName name;
    int (*function)(Receptor *, Symbol, void *, void *);
} Process;

typedef struct {
    FunctionName name;
    int (*function)(Receptor *, void *);
} LegacyProcess;


typedef struct {
    size_t size;
    int children_count;
    Offset children;
} PatternBody;

typedef struct {
    char *label;
    int child_count;
    Xaddr *children;
    int process_count;
    Process *processes;
} PatternSpecData;

typedef struct {
    Symbol noun;
} RepsBody;

void dump_xaddrs(Receptor *r);


// General Types of things that can be pointed to by Xaddrs:
//
// "Root" - Root of semantic tree- A thing which has no spec.
//
// "Spec" - Anything of which instances can be made (defines a new_proc)
//   also, in the relative sense means 1 level up the semantic tree, aka what kind of things is this?
//
//   7.spec == Age
//   Age.spec == Int
//   Int.spec == Pattern
//   Pattern.spec == Cspec
//
//
// "Instance" - opposite of a spec, 1 level down the semantic tree
//
// "Value" - a leaf in the semantic tree- something which is never a spec for something else (does not define new)
//
// new_instance_proc;
//
// instance_size_proc:
// value_size_proc:
//
// algorithm for finding size:  spec should always have an INSTANCE_SIZE proc
//
//

//
#include "util.h"

//
#include "data.h"

//
#include "semtree.h"


/**
*
*  size_of_named_surface(Receptor *r, Symbol instanceNoun, void *surface, Symbol specNoun)
*    if specXaddr is a known case
*      do that
*    else
*      size_of_named_surface(..args..,  spec(specXaddr))
*
*/



// this should go somewhere once the dependencies are better resolved.  right now it depends on everything.
size_t size_of_named_surface(Receptor *r, Symbol instanceNoun, void *surface) {
    size_t result = 0;
    if (instanceNoun <= 0){
        switch (instanceNoun) {
            case XADDR_NOUN:
                result = sizeof(Xaddr);
                break;
            case CSPEC_NOUN:
                result = (size_t) 0;
                break;
            case CSTRING_NOUN:
                result = strlen((char *) surface) + 1;
                break;
            case PATTERN_SPEC_DATA_NOUN:
                result = sizeof(PatternSpecData);
                break;
            default:
                raise_error("can't get size of instance of %d \n", instanceNoun);
        }
    } else {
        result = (*size_table_get(spec_noun_for_noun(r, instanceNoun)))(r, instanceNoun, surface);
    }
    printf("size_of_named_surface %d = %ld\n", instanceNoun, result);
    return result;
}


//
#include "stack.h"

//
#include "preops.h"

// maybe if I put useless comments here, AppCode won't try to fold up all the include statements.  super crazy-making
#include "ops.h"

//
#include "element.h"

//
#include "builtins/noun.h"

//
#include "builtins/pattern.h"

//
#include "builtins/array.h"

//
#include "builtins/int.h"

//
#include "builtins/cspec.h"



//  This really should be in ops.h.
//  it's here until we implement fake surfaces for the builtin specs so that we don't refer to the procs by name in op_invoke.
void op_invoke(Receptor *r, Xaddr invokee, FunctionName function) {
    // record call on stack?
    if (invokee.key < 0) {
        switch (invokee.noun) {
            case CSPEC_NOUN:
                switch (function) {
                    case INSTANCE_NEW:
                        cspec_proc_instance_new(r);
                        return;
                }
        }
    } else {
        ElementSurface *surface;
        Process *p;
        surface = element_surface_for_xaddr(r, invokee);
        p = getProcess(surface, function);
        if (p) {
            (((YagnArgsProcess *) p)->function)(r);
            return;
        }
    }
    raise_error2("No function %d for key %d\n", function, invokee.key);
}


// receptor init
#include "init.h"

// dump_*
#include "dump.h"

#endif
