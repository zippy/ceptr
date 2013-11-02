#ifndef _CEPTR_H
#define _CEPTR_H

#define DEFAULT_ARRAY_SIZE 100
#define DEFAULT_CACHE_SIZE 10000
#define STACK_SIZE 10000
#define OPERANDS_SIZE (sizeof(Xaddr) * 2)

#include <signal.h>
#include <string.h>
#include <stdio.h>

char error[255];

#define raise_error(error_msg, val)        \
    printf(error_msg, val);            \
    raise(SIGINT);

typedef int Symbol;

enum FunctionNames {
    INC, ADD, PRINT
};

typedef int FunctionName;

enum Frametypes {
    XADDR, PATTERN
};

typedef int Frametype;

typedef struct {
    int key;
    Symbol noun;
} Xaddr;

typedef struct {
    Xaddr noun;
    int offset;
} Offset;

typedef struct {
    Xaddr namedElement;
    char *label;
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
    Frametype type;
    int size;
} SemStackFrame;

typedef struct {
    Xaddr xaddrs[DEFAULT_CACHE_SIZE];
    char cache[DEFAULT_CACHE_SIZE];
    int current_xaddr;
    int cache_index;
} Data;

typedef struct {
    SemStackFrame semStack[STACK_SIZE];
    int semStackPointer;
    char valStack[DEFAULT_CACHE_SIZE];
    int valStackPointer;
    //built in xaddrs:
    Xaddr patternSpecXaddr;
    Xaddr intPatternSpecXaddr;
    Xaddr pointPatternSpecXaddr;
    Data data;
} Receptor;

typedef struct {
    FunctionName name;
    int (*function)(Receptor*, Xaddr);
} Process;

typedef struct {
    Symbol name;
    size_t size;
    Offset children[DEFAULT_ARRAY_SIZE];
    Process processes[DEFAULT_ARRAY_SIZE];
} PatternSpec;


Process *getProcess(PatternSpec *ps, FunctionName name) {
    int i;
    for (i = 0; i < DEFAULT_ARRAY_SIZE; i++) {
        if (ps->processes[i].name == name) {
            return &ps->processes[i];
        }
    }
    return 0;
}

enum Symbols {
    NOUN_SPEC = -3, CSPEC = -2, PATTERN_SPEC = -1
};

PatternSpec * _get_noun_pattern_spec(Receptor *r, Symbol noun) {
    Xaddr *elementXaddr = &((NounSurface *) &r->data.cache[noun])->namedElement;
    if (elementXaddr->noun == PATTERN_SPEC) {
        return (PatternSpec *) &r->data.cache[elementXaddr->key];
    }
    raise_error("nouns named item (%d) is not a pattern\n", elementXaddr->noun);
}

size_t _get_noun_size(Receptor *r, Symbol noun) {
    if (noun == PATTERN_SPEC) {
        return sizeof(PatternSpec);
    }
    PatternSpec *ps = _get_noun_pattern_spec(r,noun);
    return ps->size;
}

void *op_get(Receptor *r, Xaddr xaddr) {
    return &r->data.cache[xaddr.key];
}

Symbol op_new_noun(Receptor *r, Xaddr xaddr, char *label) {
    NounSurface ns;
    ns.namedElement.key = xaddr.key;
    ns.namedElement.noun = xaddr.noun;
    ns.label = label;
    size_t current_index = r->data.cache_index;
    void *surface = &r->data.cache[current_index];
    memcpy(surface, &ns, sizeof(NounSurface));
    r->data.cache_index += sizeof(NounSurface);

    r->data.current_xaddr++;
    r->data.xaddrs[r->data.current_xaddr].key = current_index;
    r->data.xaddrs[r->data.current_xaddr].noun = NOUN_SPEC;
    return current_index;
}

void op_set(Receptor *r, Xaddr xaddr, void *value) {
    void *surface = &r->data.cache[xaddr.key];
    memcpy(surface, value, _get_noun_size(r, xaddr.noun));
}

Xaddr op_new(Receptor *r, Symbol noun, void *surface) {
    size_t current_index = r->data.cache_index;
    r->data.cache_index += _get_noun_size(r, noun);
    r->data.current_xaddr++;
    r->data.xaddrs[r->data.current_xaddr].key = current_index;
    r->data.xaddrs[r->data.current_xaddr].noun = noun;
    Xaddr new_xaddr = {current_index, noun};
    op_set(r, new_xaddr, surface);
    return new_xaddr;
}

Xaddr op_new_pattern(Receptor *r, char *label, int childCount, Xaddr *children, int processCount, Process *processes) {
    PatternSpec ps;
    memset(&ps, 0, sizeof(PatternSpec));
    int i;
    ps.name = op_new_noun(r, r->patternSpecXaddr, label);
    if (children == 0) {
        ps.size = childCount;
    } else {
        NounSurface *noun;
        PatternSpec *cps;
        ps.size = 0;
        for (i = 0; i < childCount; i++) {
            if (children[i].noun == NOUN_SPEC) {
                noun = (NounSurface *) op_get(r, children[i]);
                cps = (PatternSpec *) op_get(r, noun->namedElement);
            } else if (children[i].noun == PATTERN_SPEC) {
                cps = (PatternSpec *) op_get(r, children[i]);
            } else {
                raise_error("Unkown child element type %d", children[i].noun);
            }
            ps.children[i].noun.key = children[i].key;
            ps.children[i].noun.noun = children[i].noun;
            ps.children[i].offset = ps.size;
            ps.size += cps->size;
        }
    }
    for (i = 0; i < processCount; i++) {
        ps.processes[i].name = processes[i].name;
        ps.processes[i].function = processes[i].function;
    }

    return op_new(r, PATTERN_SPEC, &ps);
}

int op_exec(Receptor *r,Xaddr xaddr, FunctionName processName){
    PatternSpec *ps = _get_noun_pattern_spec(r,xaddr.noun);
    Process *p = getProcess(ps, processName);
    return (*p->function)(r,xaddr);
}

int op_push_pattern(Receptor *r, Symbol patternName, void *surface) {
    //     SemStackFrame *ssf = &r->semStack[++r->semStackPointer];
    //     ssf->type = PATTERN;
    //     ssf->size = getPatternSpec(r,patternName)->size;
    //     memcpy(&r->valStack[r->valStackPointer], surface, ssf->size);
    //     r->valStackPointer += ssf->size;
}

int proc_int_inc(Receptor *r, Xaddr this) {
    void *surface = op_get(r, this);
    ++*(int *) (surface);
    return 1;
}

int proc_int_add(Receptor *r, Xaddr this) {
    int *surface = (int *) op_get(r, this);
    // semCheck please
    int *stackSurface = (int *) &r->valStack[r->valStackPointer - r->semStack[r->semStackPointer].size];
    *stackSurface = *surface + *stackSurface;
    return 1;
}

int proc_int_print(Receptor *r, Xaddr this) {
    int *surface = (int *) op_get(r, this);
    printf("%d", *surface);
}

int proc_point_print(Receptor *r, Xaddr this) {
    int *surface = (int *) op_get(r, this);
    printf("%d,%d", *surface, *(surface + 1));
}

typedef struct {
    Symbol name;
    int valueOffset;
    // pad to size of full operand
    char padding[OPERANDS_SIZE  - sizeof(Symbol) - sizeof(int)];
} ImmediatePatternOperand;

int run(Receptor *r, Instruction *instructions, void *values) {
    int counter = 0;
    ImmediatePatternOperand *op;
    while (1) {
        switch (instructions[counter].opcode) {
            case RETURN:
                return;
            case PUSH_IMMEDIATE:
                op = (ImmediatePatternOperand *) &instructions[counter].operands;
                op_push_pattern(r, op->name, (char *) values + op->valueOffset);
                break;
        }
        counter++;
    }
}

void init(Receptor *r) {

    r->data.cache_index = 0;
    r->semStackPointer = -1;
    r->valStackPointer = 0;
    r->patternSpecXaddr.key = PATTERN_SPEC;
    r->patternSpecXaddr.noun = CSPEC;
    r->data.current_xaddr = -1;


    // ********************** bootstrap built in types

    // INT
    Process int_processes[] = {
	{ PRINT, &proc_int_print},
	{ INC, &proc_int_inc },
	{ ADD, &proc_int_add }
    };
    r->intPatternSpecXaddr = op_new_pattern(r, "INT", sizeof(int), 0, 3, int_processes);

    //POINT
    Symbol X = op_new_noun(r, r->intPatternSpecXaddr, "X");
    Symbol Y = op_new_noun(r, r->intPatternSpecXaddr, "Y");

    Process point_processes[] = {
	{ PRINT, &proc_point_print }
    };

    Xaddr point_children[2] = {{X, NOUN_SPEC}, {Y, NOUN_SPEC}};
    r->pointPatternSpecXaddr = op_new_pattern(r, "POINT", 2, point_children, 1, point_processes);

    Symbol A = op_new_noun(r, r->pointPatternSpecXaddr, "A");
    Symbol B = op_new_noun(r, r->pointPatternSpecXaddr, "B");

    Xaddr line_children[2] = {{A, NOUN_SPEC}, {B, NOUN_SPEC}};
    Xaddr line_ps_xaddr = op_new_pattern(r, "LINE", 2, line_children, 0, 0);

}

// utilities

Xaddr noun_to_xaddr(Symbol noun) {
    Xaddr nounXaddr = {noun, NOUN_SPEC};
    return nounXaddr;
}

char *noun_label(Receptor *r, Symbol noun) {
    NounSurface *ns = (NounSurface *) op_get(r, noun_to_xaddr(noun));
    return ns->label;
}


#endif
