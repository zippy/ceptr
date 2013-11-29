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

== replace calls to peop_new_noun -> op_invoke(NOUN_NOUN, INSTANCE_NEW)
    - create a NOUN_SPEC cspec with cases in appropriate switch statements

 */


#define BUFFER_SIZE 10000

enum FunctionNames {
    INSTANCE_SIZE, INSTANCE_NEW, PRINT, INC, ADD
};

enum Symbols {
    ROOT = -1, CSPEC_NOUN = -2, XADDR_NOUN = -3, NOUN_NOUN = -4, CSTRING_NOUN = -5, PATTERN_SPEC_DATA_NOUN = -6
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
    int cache_index;
} Data;

typedef struct {
    SemStackFrame semStack[STACK_SIZE];
    Symbol nounStack[STACK_SIZE];
    int semStackPointer;
    char valStack[DEFAULT_CACHE_SIZE];
    int valStackPointer;

    //built in xaddrs:
    Xaddr patternSpecXaddr;
    Xaddr arraySpecXaddr;
    Xaddr intPatternSpecXaddr;
    Xaddr pointPatternSpecXaddr;
    Xaddr linePatternSpecXaddr;
    Xaddr rootXaddr;
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
    Symbol name;
    int process_count;
    Process processes;
} ElementSurface;

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


Process *getProcess(ElementSurface *es, FunctionName name) {
    int i = es->process_count;
    Process *p = &es->processes;
    while (i--) {
        if (p->name == name) {
            return p;
        }
        p++;
    }
    return 0;
}


void dump_xaddrs(Receptor *r);



// function naming convention:  <returned-thing>_for_<given-thing>
//
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

int xaddr_eq(Xaddr x1, Xaddr x2) {
    return x1.key == x2.key && x1.noun == x2.noun;
}

int sem_check(Receptor *r, Xaddr xaddr) {
    if (xaddr.key < 0) {
        if (xaddr.noun == -4) {
            return 1;
        }
        if (xaddr_eq(xaddr, r->rootXaddr)) {
            return 1;
        }
    }
    Symbol noun = r->data.xaddr_scape[xaddr.key];
    return (noun == xaddr.noun);
}

void *surface_for_xaddr(Receptor *r, Xaddr xaddr) {
    if (!sem_check(r, xaddr)) {
        raise_error2("semcheck failed getting xaddr { %d, %d }\n", xaddr.key, xaddr.noun );
    }
    return &r->data.cache[xaddr.key];
}

Xaddr xaddr_for_noun(Symbol noun) {
    Xaddr nounXaddr = {noun, NOUN_NOUN};
    return nounXaddr;
}

void *surface_for_noun(Receptor *r, Symbol noun) {
    return surface_for_xaddr(r, xaddr_for_noun(noun));
}

NounSurface *noun_surface_for_noun(Receptor *r, Symbol noun) {
    return (NounSurface *) surface_for_noun(r, noun);
}

ElementSurface *element_surface_for_xaddr(Receptor *r, Xaddr element) {
    return (ElementSurface *) surface_for_xaddr(r, element);
}

Xaddr spec_xaddr_for_noun(Receptor *r, Symbol noun){
    if (noun == r->rootXaddr.noun) {
        raise_error0("no spec for the root xaddr\n");
        return r->rootXaddr;
    } else if (noun == NOUN_NOUN) {
        return r->rootXaddr; // FIXME: should be nounSpecXaddr
    } else if (noun == XADDR_NOUN) {
        return r->rootXaddr; // FIXME: should be xaddrSpecXaddr
    } else {
        return noun_surface_for_noun(r, noun)->specXaddr;
    }
}

Xaddr spec_xaddr_for_xaddr(Receptor *r, Xaddr element) {
    return spec_xaddr_for_noun(r, element.noun);
}

Symbol spec_noun_for_xaddr(Receptor *r, Xaddr element) {
    return spec_xaddr_for_xaddr(r, element).noun;
}

Symbol spec_noun_for_noun(Receptor *r, Symbol noun) {
    return spec_xaddr_for_noun(r, noun).noun;
}

void *spec_surface_for_noun(Receptor *r, Symbol *nounType, Symbol noun) {
    void *surface = surface_for_xaddr(r, spec_xaddr_for_noun(r, noun));
    *nounType = spec_noun_for_noun(r, noun);
    return surface;
}

char *label_for_noun(Receptor *r, Symbol noun) {
    switch (noun) {
        case XADDR_NOUN:
            return "Xaddr";
        case NOUN_NOUN:
            return "Noun";
        case CSPEC_NOUN:
            return "Cspec";
        case CSTRING_NOUN:
            return "Cstring";
        default:
            return &noun_surface_for_noun(r, noun)->label;
    }
}

size_t get_noun_size(NounSurface *ns) {
    return sizeof(NounSurface)+strlen(&ns->label);
}

#define ELEMENT_HEADER_SIZE(h) (sizeof(ElementSurface)-sizeof(Process)+(((ElementSurface *)h)->process_count)*sizeof(Process))
#define SKIP_ELEM_HEADER(h) (ELEMENT_HEADER_SIZE(h) + (void *)h)
#define REPS_GET_NOUN(reps) (((RepsBody *)SKIP_ELEM_HEADER(reps))->noun)
#define REPS_SET_NOUN(reps,n) (((RepsBody *)SKIP_ELEM_HEADER(reps))->noun = n)
#define PATTERN_GET_CHILDREN_COUNT(pat) (((PatternBody *)SKIP_ELEM_HEADER(pat))->children_count)
#define PATTERN_SET_CHILDREN_COUNT(pat,count) (((PatternBody *)SKIP_ELEM_HEADER(pat))->children_count=count)
#define PATTERN_GET_CHILDREN(pat) (&(((PatternBody *)SKIP_ELEM_HEADER(pat))->children))
//#define PATTERN_GET_SIZE(pat) (((PatternBody *)skip_elem_header(pat))->size)
//#define PATTERN_SET_SIZE(pat,s) (((PatternBody *)skip_elem_header(pat))->size = s)


size_t element_header_size(void *h) {
    return sizeof(ElementSurface) - sizeof(Process) + (((ElementSurface *) h)->process_count) * sizeof(Process);
}

size_t skip_elem_header(void *element_surface) {
    return element_header_size(element_surface) + (void *) element_surface;
}

size_t pattern_get_size(void *pattern_surface) {
    return ((PatternBody *) skip_elem_header(pattern_surface))->size;
}

void pattern_set_size(void *pattern_surface, size_t size) {
    ((PatternBody *) skip_elem_header(pattern_surface))->size = size;
}

size_t get_pattern_spec_size(ElementSurface *es) {
    return ELEMENT_HEADER_SIZE(es) + sizeof(PatternBody) - sizeof(Offset) + sizeof(Offset) * PATTERN_GET_CHILDREN_COUNT(es);
}

#define _preop_get_array_length(surface) (*((int*)surface))

int preop_get_array_length(Receptor *r, Xaddr rX) {
    return _preop_get_array_length(surface_for_xaddr(r, rX));
}


int proc_pattern_get_size(Receptor *r, Symbol noun, ElementSurface *spec_surface, void *surface) {
    return pattern_get_size(spec_surface);
}


int proc_array_get_size(Receptor *r, Symbol noun, ElementSurface *spec_surface, void *surface) {
    int length = _preop_get_array_length(surface);
    int size = sizeof(int);
    Symbol arrayItemType;
    int rep_size;

    Symbol repsNoun = REPS_GET_NOUN(spec_surface);
    ElementSurface *es = spec_surface_for_noun(r, &arrayItemType, repsNoun);
    Symbol typeTypeNoun = spec_noun_for_noun(r, arrayItemType);
    if (typeTypeNoun == r->patternNoun) {
        size += length * pattern_get_size(es);
    }
    else if (typeTypeNoun == r->arrayNoun) {
        surface += sizeof(int);
        while (length--) {
            Symbol itemType;
            ElementSurface *item_spec_surface = spec_surface_for_noun(r, &itemType, repsNoun);
            rep_size = proc_array_get_size(r, repsNoun, item_spec_surface, surface);
            size += rep_size;
            surface += rep_size;
        }
    }
        //TODO: handle arrays of strings
    else {raise_error2("illegal noun (%d) as array element type for %d\n", arrayItemType, noun);}
    return size;
}



size_t size_of_named_surface(Receptor *r, Symbol name, void *surface) {
    Xaddr spec_xaddr;
    switch (name) {
        case XADDR_NOUN:
            return sizeof(Xaddr);
        case CSPEC_NOUN:
            return 0;
        case CSTRING_NOUN:
            return strlen((char *) surface) + 1;
        case PATTERN_SPEC_DATA_NOUN:
            return sizeof(PatternSpecData);
        default:
            spec_xaddr = spec_xaddr_for_noun(r, name);
            if (spec_xaddr.noun == CSPEC_NOUN) {
                return element_header_size(surface);

            } else if (spec_xaddr.noun == r->patternNoun) {
                return get_pattern_spec_size(surface);

            } else if (spec_xaddr.noun == r->arrayNoun) {
                return element_header_size(surface) + sizeof(RepsBody);


            } else {
                Symbol type_type_noun = spec_noun_for_noun(r, spec_xaddr.noun);
                Symbol _;
                ElementSurface *spec_surface = spec_surface_for_noun(r, &_, name);
                Process *p = getProcess(spec_surface, INSTANCE_SIZE);
printf("instance size %d\n", INSTANCE_SIZE);

                if (p) {
                    return (*p->function)(r, name, spec_surface, surface);
                } else {
                    dump_xaddrs(r);
                    raise_error2("couldn't find size function for noun %d in spec surface %d\n", _, name);
                    return 0;
                }

                if (type_type_noun == r->patternNoun) {
                    return proc_pattern_get_size(r, name, spec_surface, surface);

                } else if (type_type_noun == r->arrayNoun) {
                    return proc_array_get_size(r, name, spec_surface, surface);
                }

            }
            raise_error2("unknown noun type %d for noun %d\n", spec_xaddr.noun, name);
            return 0;
    }
}

void _record_existence(Receptor *r, size_t current_index, Symbol noun) {
    r->data.current_xaddr++;
    r->data.xaddrs[r->data.current_xaddr].key = current_index;
    r->data.xaddrs[r->data.current_xaddr].noun = noun;
    r->data.xaddr_scape[current_index] = noun;
}

void add_processes(ElementSurface *dest_surface, int process_count, Process *source_p) {
    printf("add_processes count %d\n", process_count);
    Process *dest_p = &dest_surface->processes;
    dest_p += dest_surface->process_count;
    dest_surface->process_count += process_count;
    while (process_count-- > 0) {
        dest_p->name = source_p->name;
        dest_p->function = source_p->function;
        source_p++;
        dest_p++;
    }
}

int _init_element(Receptor *r, char *label, Xaddr element_spec, ElementSurface *es, int processCount, Process *processes) {
    es->name = preop_new_noun(r, element_spec, label);
    ElementSurface *spec_surface = element_surface_for_xaddr(r,element_spec);

    es->processes = spec_surface->processes;
    es->process_count = 1;
    add_processes(es, processCount, processes);
    return es->name;
}

//
#include "stack.h"

//
#include "preops.h"

// maybe if I put useless comments here, AppCode won't try to fold up all the include statements.  super crazy-making
#include "ops.h"

//
#include "procs.h"


//  This really should be in ops.h.
//  it's here until we implement fake surfaces for the builtin specs so that we don't refer to the procs by name in op_invoke.
void op_invoke(Receptor *r, Xaddr invokee, FunctionName function) {
    // record call on stack?
    if (invokee.key < 0) {
        switch (invokee.noun) {
            case CSPEC_NOUN:
                switch (function) {
                    case INSTANCE_NEW:
                        proc_cspec_instance_new(r);
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
    raise_error2("No function %d for key %d", function, invokee.key);
}

// receptor init
#include "init.h"

// dump_*
#include "dump.h"

#endif
