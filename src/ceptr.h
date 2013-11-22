#ifndef _CEPTR_H
#define _CEPTR_H

#define DEFAULT_CACHE_SIZE 100000
#define STACK_SIZE 10000
#define OPERANDS_SIZE (sizeof(Xaddr) * 2)

#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

char error[255];

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

#define SYMBOL_PATH_TERMINATOR 0xFFFF
#define BUFFER_SIZE 10000

enum FunctionNames {
    PRINT, INC, ADD, INSTANCE_NEW, INSTANCE_SIZE
};

enum Symbols {
    ROOT = -1, CSPEC_NOUN = -2, XADDR_NOUN = -3, NOUN_NOUN = -4, CSTRING_NOUN = -5
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

enum Opcodes {
    RETURN, PUSH_IMMEDIATE
};

typedef int Opcode;

typedef struct {
    Opcode opcode;
    char operands[OPERANDS_SIZE];
} Instruction;

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

    int (*function)(Receptor *, void *);
} Process;


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
// algorithm for finding size:
// if my spec has an instance_size_proc, use that.
// else crawl up the stack to see if any ancestor has a value_size_proc
//   if you reach root, fail.
//

int sem_check(Receptor *r, Xaddr xaddr) {
    if (xaddr.noun == -4) {
        return 1;
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


//TODO: refactor out the code in here that walks noun-types.  This code is seen in dump_xaddrs and also in array_nth

#define GET_SIZE 0

//FIXME: should we allow check for surface == 0 and raise an error for those cases where it matters?
size_t _newsize_of_named_surface(Receptor *r, Symbol noun, void *surface) {
    Symbol nounType;
    ElementSurface *spec_surface = spec_surface_for_noun(r, &nounType, noun);
    Process *p = getProcess(spec_surface, GET_SIZE);
    if (p) {
        return (*p->function)(r, spec_surface);
    } else {
        dump_xaddrs(r);
        raise_error2("couldn't find size function for noun %d in spec surface %d\n", noun, spec_surface->name);
        return 0;
    }
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

Symbol preop_new_noun(Receptor *r, Xaddr xaddr, char *label) {

    size_t current_index = r->data.cache_index;
    NounSurface *ns = (NounSurface *)&r->data.cache[current_index];
    ns->specXaddr.key = xaddr.key;
    ns->specXaddr.noun = xaddr.noun;
    memcpy(&ns->label,label,strlen(label)+1);
    r->data.cache_index += get_noun_size(ns);

    _record_existence(r, current_index, NOUN_NOUN);

    return current_index;
}

void preop_set(Receptor *r, Xaddr xaddr, void *value) {
    void *surface = &r->data.cache[xaddr.key];
    if (!sem_check(r, xaddr)) {
        raise_error("I do not think that word (%d) means what you think it means!\n", xaddr.noun);
    }
    size_t size = size_of_named_surface(r, xaddr.noun, value);

    memcpy(surface, value, size);
}

Xaddr preop_new(Receptor *r, Symbol noun, void *surface) {
    size_t current_index = r->data.cache_index;
    r->data.cache_index += size_of_named_surface(r, noun, surface);
    _record_existence(r, current_index, noun);
    Xaddr new_xaddr = {current_index, noun};
    preop_set(r, new_xaddr, surface);
    return new_xaddr;
}


void _copy_processes(ElementSurface *dest_surface, int process_count, Process *processes) {
    int i;
    dest_surface->process_count = process_count;
    Process *p = &dest_surface->processes;
    for (i = 0; i < process_count; i++) {
        p->name = processes[i].name;
        p->function = processes[i].function;
        p++;
    }
}

int _init_element(Receptor *r, char *label, Xaddr element_spec, ElementSurface *es, int processCount, Process *processes) {
    es->name = preop_new_noun(r, element_spec, label);
    _copy_processes(es, processCount, processes);
    return es->name;
}




void *stack_peek(Receptor *r, Symbol *name, void **surface) {
    assert(r->semStackPointer >= 0);
    SemStackFrame *ssf = &r->semStack[r->semStackPointer];
    *name = ssf->noun;
    *surface = &r->valStack[r->valStackPointer - ssf->size];
}

void stack_pop_unchecked(Receptor *r, Symbol *name, void *surface) {
    SemStackFrame *ssf = &r->semStack[r->semStackPointer];
    *name = ssf->noun;
    memcpy(surface, &r->valStack[r->valStackPointer - ssf->size], ssf->size);
    r->valStackPointer -= ssf->size;
    r->semStackPointer--;
}

void stack_push(Receptor *r, Symbol name, void *surface) {
    SemStackFrame *ssf = &r->semStack[++r->semStackPointer];
    ssf->noun = name;
    ssf->size = size_of_named_surface(r, name, surface);
    memcpy(&r->valStack[r->valStackPointer], surface, ssf->size);
    r->valStackPointer += ssf->size;
}

void stack_pop(Receptor *r, Symbol expectedNoun, void *surface) {
    Symbol actualNoun;
    stack_pop_unchecked(r, &actualNoun, surface);
    assert( actualNoun == expectedNoun );
}

void dump_stack(Receptor *r) {
    int i, v = 0;
    char *unknown = "<unknown>";
    char *label;
    ElementSurface *ps;
    SemStackFrame *ssf;
    for (i = 0; i <= r->semStackPointer; i++) {
        ssf = &r->semStack[i];
        printf("\nStack frame: %d is a %s(%d) size:%d\n", i, label_for_noun(r, ssf->noun), ssf->noun, (int)ssf->size);
//        printf("   Value:");
//        dump_pattern_value(r, ps, &r->valStack[v]);
//        v += r->semStack[i].size;
        printf("\n");
    }
}
//
//void op_new(r) {
//
//}
//

void op_new(Receptor *r, Symbol noun, void *surface) {
    Xaddr x = preop_new(r,noun,surface);
    stack_push(r,XADDR_NOUN,&x);
}

void proc_cspec_instance_new(Receptor *r, Xaddr invokee) {
    char label[BUFFER_SIZE];
    char ps[BUFFER_SIZE];
    stack_pop(r, CSTRING_NOUN, label);
    memset(ps, 0, BUFFER_SIZE);
    Process *p = 0;
    Symbol newNoun = _init_element(r, &label[0], r->rootXaddr, (ElementSurface *) ps, 0, p);
    op_new(r, newNoun, ps);
}


void op_invoke(Receptor *r, Xaddr invokee, FunctionName function) {
    // record call on stack?
    switch(invokee.noun) {
        case CSPEC_NOUN:
            switch(function){
                case INSTANCE_NEW:
                    proc_cspec_instance_new(r, invokee);
            }
    }
}


void *preop_get_array_nth(Receptor *r, int index, Xaddr rX) {
    void *surface = surface_for_xaddr(r, rX);
    int length = _preop_get_array_length(surface);
    if (index >= length) {
        raise_error2("index %d into array %d greater than length\n", rX.key, index);
    }
    surface += sizeof(int);
    if (index > 0) {

        Symbol nounSpecType;
        ElementSurface *rs = spec_surface_for_noun(r, &nounSpecType, rX.noun);
        Symbol typeTypeNoun = spec_noun_for_noun(r, nounSpecType);
        if (typeTypeNoun != r->arrayNoun) {
            raise_error2("xaddr points to a %d, expected array(%d)\n", nounSpecType, r->arrayNoun);
        }
        Symbol repsNoun = REPS_GET_NOUN(rs);
        Symbol arrayItemType;
        ElementSurface *es = spec_surface_for_noun(r, &arrayItemType, repsNoun);
        typeTypeNoun = spec_noun_for_noun(r, nounSpecType);
        int size;
        if (typeTypeNoun == r->patternNoun) {
            size = pattern_get_size(es);
            surface += size * index;
        }
        else if (typeTypeNoun == r->arrayNoun) {
            while (index--) {
                surface += size_of_named_surface(r, repsNoun, surface);
            }
        }
        else {raise_error("bad array item type %d\n", arrayItemType);}
    }
    return surface;
}

Xaddr _preop_new_rep(Receptor *r, Xaddr rep_type, Symbol rep_type_noun, char *label, Symbol repsNoun, int processCount, Process *processes) {
    char rs[BUFFER_SIZE];
    memset(rs, 0, BUFFER_SIZE);
    Symbol new_noun = _init_element(r, label, rep_type, (ElementSurface *) rs, processCount, processes);
    REPS_SET_NOUN(rs, repsNoun);
    return preop_new(r, new_noun, rs);
}

Xaddr preop_new_array(Receptor *r, char *label, Symbol repsNoun, int processCount, Process *processes) {
    return _preop_new_rep(r, r->arraySpecXaddr, r->arrayNoun, label, repsNoun, processCount, processes);
}

Xaddr preop_new_pattern(Receptor *r, char *label, int child_count_or_size, Xaddr *children, int processCount, Process *processes) {
    char ps[BUFFER_SIZE];
    memset(ps, 0, BUFFER_SIZE);
    int i;
    Symbol newNoun = _init_element(r, label, r->patternSpecXaddr, (ElementSurface *) ps, processCount, processes);

    if (children == 0) {
        pattern_set_size(ps, child_count_or_size);
    } else {
        NounSurface *noun;
        ElementSurface *child_pattern_surface;
        int size = 0;
        Offset *pschildren = PATTERN_GET_CHILDREN(ps);

        for (i = 0; i < child_count_or_size; i++) {
            if (children[i].noun == NOUN_NOUN) {
                noun = (NounSurface *) surface_for_xaddr(r, children[i]);
                child_pattern_surface = element_surface_for_xaddr(r, noun->specXaddr);
            } else if (children[i].noun == r->patternNoun) {
                child_pattern_surface = element_surface_for_xaddr(r, children[i]);
            } else {
                raise_error("Unknown child element type %d\n", children[i].noun);
            }
            pschildren[i].noun.key = children[i].key;
            pschildren[i].noun.noun = children[i].noun;
            pschildren[i].offset = size;
            size += pattern_get_size(child_pattern_surface);
        }
        pattern_set_size(ps, size);
        PATTERN_SET_CHILDREN_COUNT(ps, child_count_or_size);

    }
    return preop_new(r, newNoun, ps);
}

#define NULL_XADDR(x) ((x.noun)==(x.key))

int getOffset(ElementSurface *ps, Symbol name) {
    Offset *o = PATTERN_GET_CHILDREN(ps);
    while (!NULL_XADDR(o->noun)) {
        if (o->noun.key == name) {
            return o->offset;
        }
        o++;
    }
    raise_error2("offset not found for: %d in getOffset for patternSpec %d\n", name, ps->name);
}

int proc_int_inc(Receptor *r, void *this) {
    ++*(int *) (this);
    return 0;
}

int proc_int_add(Receptor *r, void *this) {
    // semCheck please
    int *stackSurface = (int *) &r->valStack[r->valStackPointer - r->semStack[r->semStackPointer].size];
    *stackSurface = *(int *) this + *stackSurface;
    return 0;
}

int proc_int_print(Receptor *r, void *this) {
    printf("%d", *(int *) this);
}

int proc_point_print(Receptor *r, void *this) {
    printf("%d,%d", *(int *) this, *(((int *) this) + 1));
}

int proc_line_print(Receptor *r, void *this) {
    int *surface = (int *) this;
    printf("[%d,%d - %d,%d] ", *surface, *(surface + 1), *(surface + 2), *(surface + 3));
}

typedef struct {
    Symbol name;
    int valueOffset;
    // pad to size of full operand
    char padding[OPERANDS_SIZE  - sizeof(Symbol) - sizeof(int)];
} ImmediatePatternOperand;

//FIXME: this is needs to be implemented as a scape, not a linear scan of all Xaddrs!!
Symbol getSymbol(Receptor *r, char *label) {
    NounSurface *ns;
    Symbol noun;
    int i;
    for (i = 0; i <= r->data.current_xaddr; i++) {
        if (r->data.xaddrs[i].noun == NOUN_NOUN) {
            noun = r->data.xaddrs[i].key;
            ns = (NounSurface *) &r->data.cache[noun];
            if (!strcmp(label, &ns->label)) {
                return noun;
            }
        }
    }
}


void init_data(Receptor *r) {
    int i;
    for (i = 0; i < DEFAULT_CACHE_SIZE; i++) r->data.xaddr_scape[i] = ROOT;

    r->data.cache_index = 0;
    r->data.current_xaddr = -1;
}

void init_elements(Receptor *r) {
    r->rootXaddr.key = ROOT;
    r->rootXaddr.noun = CSPEC_NOUN;

    stack_push(r, CSTRING_NOUN, &"PATTERN");
    op_invoke(r, r->rootXaddr, INSTANCE_NEW);
    stack_pop(r,XADDR_NOUN,&r->patternSpecXaddr);
    r->patternNoun = (element_surface_for_xaddr(r, r->patternSpecXaddr))->name;

    stack_push(r, CSTRING_NOUN, &"ARRAY");
    op_invoke(r, r->rootXaddr, INSTANCE_NEW);
    stack_pop(r,XADDR_NOUN,&r->arraySpecXaddr);
    r->arrayNoun = (element_surface_for_xaddr(r, r->arraySpecXaddr))->name;
//    Symbol *_;
//    stack_pop_named_surface(r, _, &r->patternSpecXaddr);

//    r->patternSpecXaddr = proc_cspec_instance_new(r, "PATTERN");
//

//    r->arraySpecXaddr = proc_cspec_instance_new(r, "ARRAY");

}

void init_stack(Receptor *r) {
    r->semStackPointer = -1;
    r->valStackPointer = 0;
}

void init_base_types(Receptor *r) {
    // ********************** bootstrap built in types

    // INT
    Process int_processes[] = {
        {PRINT, &proc_int_print},
        {INC, &proc_int_inc},
        {ADD, &proc_int_add}
    };
    r->intPatternSpecXaddr = preop_new_pattern(r, "INT", sizeof(int), 0, 3, int_processes);

    // POINT
    Symbol X = preop_new_noun(r, r->intPatternSpecXaddr, "X");
    Symbol Y = preop_new_noun(r, r->intPatternSpecXaddr, "Y");

    Process point_processes[] = {
        {PRINT, &proc_point_print}
    };

    // LINE
    Xaddr point_children[2] = {{X, NOUN_NOUN}, {Y, NOUN_NOUN}};
    r->pointPatternSpecXaddr = preop_new_pattern(r, "POINT", 2, point_children, 1, point_processes);

    Symbol A = preop_new_noun(r, r->pointPatternSpecXaddr, "A");
    Symbol B = preop_new_noun(r, r->pointPatternSpecXaddr, "B");

    Xaddr line_children[2] = {{A, NOUN_NOUN}, {B, NOUN_NOUN}};

    Process line_processes[] = {
        {PRINT, &proc_line_print}
    };

    r->linePatternSpecXaddr = preop_new_pattern(r, "LINE", 2, line_children, 1, line_processes);
}


void init(Receptor *r) {
    init_stack(r);
//    init_processing(r);
    init_data(r);
    init_elements(r);
    init_base_types(r);
}

// utilities


void hexDump(char *desc, void *addr, int len) {
    int i;
    unsigned char buff[17];
    unsigned char *pc = addr;

    // Output description if given.
    if (desc != NULL)
        printf("%s:\n", desc);

    // Process every byte in the data.
    for (i = 0; i < len; i++) {
        // Multiple of 16 means new line (with line offset).

        if ((i % 16) == 0) {
            // Just don't print ASCII for the zeroth line.
            if (i != 0)
                printf("  %s\n", buff);

            // Output the offset.
            printf("  %04x ", i);
        }

        // Now the hex code for the specific character.
        printf(" %02x", pc[i]);

        // And store a printable ASCII character for later.
        if ((pc[i] < 0x20) || (pc[i] > 0x7e))
            buff[i % 16] = '.';
        else
            buff[i % 16] = pc[i];
        buff[(i % 16) + 1] = '\0';
    }
}

void dump_children_array(Offset *children, int count) {
    int i = 0;
    while (i < count) {
        if (i != 0) {
            printf(",");
        }
        printf("{ %d, %d }(%d)", children[i].noun.key, children[i].noun.noun, children[i].offset);
        i++;
    }
}


void dump_process_array(Process *process, int count) {
    int i = 0;
    while (i < count) {
        if (i != 0) {
            printf(",");
        }
        printf("{ %d, %zu }", process[i].name, (size_t) process[i].function);
        i++;
    }
}

void dump_reps_spec(Receptor *r, void *surface) {
    ElementSurface *rs = (ElementSurface *)surface;

    printf("%s\n", "Array");
    printf("    name: %s(%d)\n", label_for_noun(r, rs->name), rs->name);
    int noun = REPS_GET_NOUN(rs);
    printf("    repsNoun: %s(%d) \n", label_for_noun(r, noun), noun);
//    dump_process_array(rs->processes);
    printf("\n");
}

void dump_spec_spec(Receptor *r, void *surface) {
    ElementSurface *ps = (ElementSurface *)surface;
    printf("Spec\n");
    printf("    name: %s(%d)\n", label_for_noun(r, ps->name), ps->name);
    printf("\n    %d processes: ", ps->process_count);
    dump_process_array(&ps->processes, ps->process_count);
    printf("\n");
}


void dump_pattern_spec(Receptor *r, void *surface) {
    ElementSurface *ps = (ElementSurface *)surface;

    printf("Pattern\n");
    printf("    name: %s(%d)\n", label_for_noun(r, ps->name), ps->name);
    printf("    size: %d\n", (int) pattern_get_size(ps));
    int count = PATTERN_GET_CHILDREN_COUNT(ps);
    printf("    %d children: ", count);
    dump_children_array(PATTERN_GET_CHILDREN(ps), count);
    printf("\n    %d processes: ", ps->process_count);
    dump_process_array(&ps->processes, ps->process_count);
    printf("\n");
}

void dump_pattern_value(Receptor *r, void *pattern_surface, void *surface) {
    ElementSurface *ps = (ElementSurface *)pattern_surface;
    Process *print_proc;
    print_proc = getProcess(ps, PRINT);
    if (print_proc) {
        (*print_proc->function)(r, surface);
    } else {
        hexDump("hexDump of surface", surface, pattern_get_size(ps));
    }
}

void dump_array_value(Receptor *r, ElementSurface *rs, void *surface) {
    int count = *(int *) surface;
    surface += sizeof(int);
    Symbol arrayItemType;
    Symbol repsNoun = REPS_GET_NOUN(rs);
    ElementSurface *es = spec_surface_for_noun(r, &arrayItemType, repsNoun);
    Symbol typeTypeNoun = spec_noun_for_noun(r, arrayItemType);
    int size;
    if (typeTypeNoun == r->patternNoun) {
        printf("%s(%d) array of %d %s(%d)s\n", label_for_noun(r, rs->name), rs->name, count, label_for_noun(r, repsNoun), repsNoun);
        size = pattern_get_size(es);
        while (count > 0) {
            printf("    ");
            dump_pattern_value(r, es, surface);
            printf("\n");
            surface += size;
            count--;
        }
    }
    else if (typeTypeNoun == r->arrayNoun) {
        printf("array of %d %s(%d) arrays\n", count, label_for_noun(r, rs->name), rs->name);
        while (count > 0) {
            printf("    ");
            dump_array_value(r, es, surface);
            surface += size_of_named_surface(r, repsNoun, surface);
            count--;
        }
    }
}


void dump_noun(Receptor *r, NounSurface *ns) {
    printf("Noun      { %d, %5d } %s", ns->specXaddr.key, ns->specXaddr.noun, &ns->label);
}

void dump_xaddr(Receptor *r, Xaddr xaddr, int indent_level) {
    Symbol typeNoun = spec_noun_for_xaddr(r, xaddr);

    ElementSurface *es;
    NounSurface *ns;
    void *surface;
    int key = xaddr.key;
    int noun = xaddr.noun;

    if (noun == NOUN_NOUN) {
        dump_noun(r, (NounSurface *) &r->data.cache[key]);
    } else if (typeNoun == CSPEC_NOUN) {
        dump_spec_spec(r, &r->data.cache[key]);

    } else if (typeNoun == r->patternNoun) {
        dump_pattern_spec(r, &r->data.cache[key]);

    } else if (typeNoun == r->arrayNoun) {
        dump_reps_spec(r, &r->data.cache[key]);

    } else {
        Symbol typeTypeNoun = spec_noun_for_noun(r, typeNoun);
        switch (typeTypeNoun) {
            case NOUN_NOUN:
                ns = (NounSurface *) &r->data.cache[key];
                dump_noun(r, ns);
                break;
            default:
                surface = surface_for_xaddr(r, xaddr_for_noun(noun));
                ns = (NounSurface *) surface;
                printf("%s : ", &ns->label);
                es = element_surface_for_xaddr(r, ns->specXaddr);
                if (typeTypeNoun == r->patternNoun) {
                    dump_pattern_value(r, es, surface_for_xaddr(r, xaddr));
                } else if (typeTypeNoun == r->arrayNoun) {
                    dump_array_value(r, es, surface_for_xaddr(r, xaddr));
                }
        }
    }
}

void dump_xaddrs(Receptor *r) {
    int i;
    NounSurface *ns;
    void *surface;
    for (i = 0; i <= r->data.current_xaddr; i++) {
        printf("Xaddr { %5d, %5d } - ", r->data.xaddrs[i].key, r->data.xaddrs[i].noun);
        dump_xaddr(r, r->data.xaddrs[i], 0);
        printf("\n");
    }
}



//
//ElementSurface *walk_path(Receptor *r, Xaddr xaddr, Symbol *path, int *offset) {
//    ElementSurface *ps = _get_noun_pattern_spec(r, xaddr.noun);
//    *offset = 0;
//    int i = 0;
//    while (path[i] != SYMBOL_PATH_TERMINATOR) {
//        *offset += getOffset(ps, path[i]);
//        ps = _get_noun_pattern_spec(r, path[i]);
//        i++;
//    }
//    return ps;
//}


//
//void *preop_set_by_path(Receptor *r, Xaddr xaddr, Symbol *path, void *value) {
//    int offset;
//    ElementSurface *ps = walk_path(r, xaddr, path, &offset);
//    void *surface = &r->data.cache[xaddr.key + offset];
//    return memcpy(surface, value, pattern_get_size(ps));
//}
//
//void *preop_get_by_path(Receptor *r, Xaddr xaddr, Symbol *path) {
//    int offset;
//    walk_path(r, xaddr, path, &offset);
//    return &r->data.cache[xaddr.key + offset];
//}

//
//int preop_exec(Receptor *r, Xaddr xaddr, FunctionName processName) {
//    ElementSurface *ps = _get_noun_pattern_spec(r, xaddr.noun);
//    Process *p = getProcess(ps, processName);
//    return (*p->function)(r, preop_get(r, xaddr));
//}
//

//
//
//void dump_stack(Receptor *r) {
//    int i, v = 0;
//    char *unknown = "<unknown>";
//    char *label;
//    ElementSurface *ps;
//    for (i = 0; i <= r->semStackPointer; i++) {
//        Xaddr type = r->semStack[i].type;
//        if (type.noun == r->patternNoun) {
//            ps = (ElementSurface *) &r->data.cache[type.key];
//            label = noun_label(r, ps->name);
//
//        }
//        else {
//            label = unknown;
//        }
//        printf("\nStack frame: %d is a %s({%d,%d}) size:%d\n", i, label, type.key, type.noun, r->semStack[i].size);
//        printf("   Value:");
//        dump_pattern_value(r, ps, &r->valStack[v]);
//        v += r->semStack[i].size;
//        printf("\n");
//    }
//
//}
//
//
//int run(Receptor *r, Instruction *instructions, void *values) {
//    int counter = 0;
//    ImmediatePatternOperand *op;
//    while (1) {
//        switch (instructions[counter].opcode) {
//            case RETURN:
//                return 0;
//            case PUSH_IMMEDIATE:
//                op = (ImmediatePatternOperand *) &instructions[counter].operands;
//                preop_push_pattern(r, op->name, (char *) values + op->valueOffset);
//                break;
//        }
//        counter++;
//    }
//}

//
//ElementSurface *_get_reps_pattern_spec(Receptor *r, ElementSurface *rs) {
//    return _get_noun_pattern_spec(r, REPS_GET_NOUN(rs));
//}
//

//
//ElementSurface *_get_noun_pattern_spec(Receptor *r, Symbol noun) {
//    Xaddr *elementXaddr = &((NounSurface *) &r->data.cache[noun])->elementType;
//
//    if (elementXaddr->noun == r->patternNoun) {
//        return (ElementSurface *) &r->data.cache[elementXaddr->key];
//    } else {
//        Xaddr specXaddr = xaddr_get_type(r, *elementXaddr);
//        if (specXaddr.noun == r->patternNoun) {
//            return (ElementSurface *) &r->data.cache[specXaddr.key];
//        }
//    }
//    raise_error2("noun (%d) named item (%d) is not a pattern\n", noun, elementXaddr->noun);
//}

//
//int proc_pattern_spec_get_size(Receptor *r, Symbol noun, ElementSurface *spec_surface, void *surface) {
//    return get_pattern_spec_size((ElementSurface *) surface);
//}

#endif
