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
    Xaddr nounSpecXaddr;
    Xaddr patternSpecXaddr;
    Xaddr arraySpecXaddr;
    Xaddr intPatternSpecXaddr;
    Xaddr pointPatternSpecXaddr;
    Xaddr linePatternSpecXaddr;
    Xaddr cspecXaddr;
    Xaddr rootXaddr;
    Symbol nounNoun;
    Symbol patternNoun;
    Symbol arrayNoun;

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

//
#include "element.h"

//
#include "builtins/noun.h"

//
#include "builtins/pattern.h"

//
#include "builtins/array.h"


/**
*
*  size_of_named_surface(Receptor *r, Symbol instanceNoun, void *surface, Symbol specNoun)
*    if specXaddr is a known case
*      do that
*    else
*      size_of_named_surface(..args..,  spec(specXaddr))
*
*/


typedef size_t (*sizeFunction)(Receptor *, Symbol, void *);
sizeFunction size_table[BUFFER_SIZE];

sizeFunction size_table_get(Symbol noun) {
    return size_table[noun];
}

void size_table_set(Symbol noun, sizeFunction func) {
    size_table[noun] = func;
}

// this should go somewhere once the dependencies are better resolved.  right now it depends on everything.
size_t size_of_named_surface(Receptor *r, Symbol instanceNoun, void *surface) {
//    if (instanceNoun < 0){
//        switch (instanceNoun) {
//            case XADDR_NOUN:
//                return sizeof(Xaddr);
//            case CSPEC_NOUN:
//                return (size_t) 0;
//            case CSTRING_NOUN:
//                return strlen((char *) surface) + 1;
//            case PATTERN_SPEC_DATA_NOUN:
//                return sizeof(PatternSpecData);
//            default:
//                raise_error("can't get size of instance %d \n", instanceNoun);
//        }
//    } else {
//        return (*size_table_get(instanceNoun))(r, instanceNoun, surface);
//    }
//    if (specNoun == ROOT) {
//    } else {
//        if (specNoun == 0 && instanceNoun == 0) {
//            return element_header_size(surface);
//
//        } else if (specNoun == r->nounNoun) {
//            return sizeof(NounSurface);
//
//        } else if (specNoun == CSPEC_NOUN) {
//            return element_header_size(surface);
//
//        } else if (specNoun == r->patternNoun) {
////            return get_pattern_spec_size(surface);
//        } else if (specNoun == r->intPatternSpecXaddr.noun) {
//            return pattern_get_size(surface);
//
//        }
//
//        } else if (specNoun == r->arrayNoun) {
//            return element_header_size(surface) + sizeof(RepsBody);
//
//        } else {
//            return size_of_named_surface(r, instanceNoun, surface, spec_noun_for_noun(r, specNoun));
//        }
//    }

    Xaddr spec_xaddr;
    switch (instanceNoun) {
        case XADDR_NOUN:
            return sizeof(Xaddr);
        case CSPEC_NOUN:
            return (size_t) 0;
        case CSTRING_NOUN:
            return strlen((char *) surface) + 1;
        case PATTERN_SPEC_DATA_NOUN:
            return sizeof(PatternSpecData);
        default:
            spec_xaddr = spec_xaddr_for_noun(r, instanceNoun);
            // special case for noun spec
            if (spec_xaddr.noun == 0 && instanceNoun == 0) {
                return element_header_size(surface);
            }
            else if (spec_xaddr.noun == CSPEC_NOUN) {
                return element_header_size(surface);

            } else if (spec_xaddr.noun == r->patternNoun) {
                return get_pattern_spec_size(surface);

            } else if (spec_xaddr.noun == r->arrayNoun) {
                return element_header_size(surface) + sizeof(RepsBody);


            } else {
                Symbol type_type_noun = spec_noun_for_noun(r, spec_xaddr.noun);
                Symbol _;
                ElementSurface *spec_surface = spec_surface_for_noun(r, &_, instanceNoun);
                Process *p = getProcess(spec_surface, INSTANCE_SIZE);
                printf("instance size %d\n", INSTANCE_SIZE);

                if (p) {
                    return (size_t)(*p->function)(r, instanceNoun, spec_surface, surface);
                } else {
                    dump_xaddrs(r);
                    raise_error2("couldn't find size function for noun %d in spec surface %d\n", _, instanceNoun);
                    return (size_t) 0;
                }

                if (type_type_noun == r->patternNoun) {
                    return (size_t) proc_pattern_get_size(r, instanceNoun, spec_surface, surface);

                } else if (type_type_noun == r->arrayNoun) {
                    return (size_t) proc_array_get_size(r, instanceNoun, spec_surface, surface);
                }

            }
            raise_error2("unknown noun type %d for noun %d\n", spec_xaddr.noun, instanceNoun);
            return (size_t) 0;
    }
}


//
#include "stack.h"

//
#include "preops.h"

// maybe if I put useless comments here, AppCode won't try to fold up all the include statements.  super crazy-making
#include "ops.h"

//
#include "procs.h"

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
