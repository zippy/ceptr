#ifndef _CEPTR_H
#define _CEPTR_H

#define DEFAULT_CACHE_SIZE 100000
#define STACK_SIZE 10000
#define OPERANDS_SIZE (sizeof(Xaddr) * 2)

#include <signal.h>
#include <string.h>
#include <stdio.h>

char error[255];

#define raise_error(error_msg, val)        \
    printf(error_msg, val);            \
    raise(SIGINT);
#define raise_error2(error_msg, val1,val2)        \
    printf(error_msg, val1,val2);                \
    raise(SIGINT);

typedef int Symbol;

#define SYMBOL_PATH_TERMINATOR 0xFFFF
#define STRING_TERMINATOR 0xFFFFFFFF
#define ESCAPE_STRING_TERMINATOR 0xFFFFFFFE

enum FunctionNames {
    PRINT, INC, ADD
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
    Xaddr type;
    int size;
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
    int semStackPointer;
    char valStack[DEFAULT_CACHE_SIZE];
    int valStackPointer;
    //built in xaddrs:
    Xaddr patternSpecXaddr;
    Xaddr arraySpecXaddr;
    Xaddr stringSpecXaddr;
    Xaddr intPatternSpecXaddr;
    Xaddr pointPatternSpecXaddr;
    Xaddr linePatternSpecXaddr;
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
    Symbol patternNoun;
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

enum Symbols {
    NULL_SYMBOL = -1, CSPEC = -2, NOUN_SPEC = -3, PATTERN_SPEC = -4,  ARRAY_SPEC = -5, STRING_SPEC = -6
};

void dump_xaddrs(Receptor *r);

void *_get_noun_element_spec(Receptor *r, Symbol *nounType, Symbol noun){
    Xaddr *elementXaddr = &((NounSurface *) &r->data.cache[noun])->namedElement;
    *nounType = elementXaddr->noun;
    return &r->data.cache[elementXaddr->key];
}

ElementSurface *_get_noun_pattern_spec(Receptor *r, Symbol noun) {
    Xaddr *elementXaddr = &((NounSurface *) &r->data.cache[noun])->namedElement;
    if (elementXaddr->noun == PATTERN_SPEC) {
        return (ElementSurface *) &r->data.cache[elementXaddr->key];
    }
    raise_error2("noun (%d) named item (%d) is not a pattern\n", noun, elementXaddr->noun);
}

int sem_check(Receptor *r, Xaddr xaddr) {
    Symbol noun = r->data.xaddr_scape[xaddr.key];
    return (noun == xaddr.noun);
}

void *op_get(Receptor *r, Xaddr xaddr) {
    if (!sem_check(r, xaddr)) return 0;
    return &r->data.cache[xaddr.key];
}

#define element_header_size(h) (sizeof(ElementSurface)-sizeof(Process)+(((ElementSurface *)h)->process_count)*sizeof(Process))
#define skip_elem_header(h) (element_header_size(h) + (void *)h)
#define REPS_GET_PATTERN_NOUN(reps) (((RepsBody *)skip_elem_header(reps))->patternNoun)
#define REPS_SET_PATTERN_NOUN(reps,n) (((RepsBody *)skip_elem_header(reps))->patternNoun = n)
#define PATTERN_GET_SIZE(pat) (((PatternBody *)skip_elem_header(pat))->size)
#define PATTERN_GET_CHILDREN_COUNT(pat) (((PatternBody *)skip_elem_header(pat))->children_count)
#define PATTERN_SET_CHILDREN_COUNT(pat,count) (((PatternBody *)skip_elem_header(pat))->children_count=count)
#define PATTERN_SET_SIZE(pat,s) (((PatternBody *)skip_elem_header(pat))->size = s)
#define PATTERN_GET_CHILDREN(pat) (&(((PatternBody *)skip_elem_header(pat))->children))

ElementSurface *_get_reps_pattern_spec(Receptor *r,ElementSurface *rs) {
    return _get_noun_pattern_spec(r,REPS_GET_PATTERN_NOUN(rs));
}

size_t _get_pattern_spec_size(ElementSurface *es) {
    return element_header_size(es) + sizeof(PatternBody) - sizeof(Offset) + sizeof(Offset) * PATTERN_GET_CHILDREN_COUNT(es);
}

#define pattern_size(pat) ((void *)(pat))

size_t _get_noun_size(Receptor *r, Symbol noun, void *surface) {
    if (noun == PATTERN_SPEC) {
	return _get_pattern_spec_size((ElementSurface *)surface);
    } else if ((noun == ARRAY_SPEC)||(noun == STRING_SPEC)) {
        return element_header_size(surface) + sizeof(RepsBody);
    }
    Symbol nounType;
    ElementSurface *spec_surface = _get_noun_element_spec(r, &nounType, noun);
    ElementSurface *ps;
    int rep_size;
    int size;
    switch(nounType) {
        case PATTERN_SPEC:
            return PATTERN_GET_SIZE(spec_surface);
            break;
        case ARRAY_SPEC:
	    ps = _get_reps_pattern_spec(r,spec_surface);
            return sizeof(int) + (*(int *)surface) * PATTERN_GET_SIZE(ps) ;
            break;
        case STRING_SPEC:
	    ps = _get_reps_pattern_spec(r,spec_surface);
	    rep_size = PATTERN_GET_SIZE(ps);
	    size = 0;
	    while(*(int *)surface != STRING_TERMINATOR) {
		if (*(int *)surface == ESCAPE_STRING_TERMINATOR) {
		    surface += sizeof(int);
		    size +=sizeof(int);
		}
		size += rep_size;
		surface += rep_size;
	    }
	    return sizeof(int) + size;
	    break;
    }
    raise_error2("unknown noun type %d for noun %d\n", nounType, noun);
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
    r->data.xaddr_scape[current_index] = NOUN_SPEC;
    return current_index;
}

void op_set(Receptor *r, Xaddr xaddr, void *value) {
    void *surface = &r->data.cache[xaddr.key];
    if (!sem_check(r, xaddr)) {
        raise_error("I do not think that word (%d) means what you think it means!", xaddr.noun);
    }
    size_t size = _get_noun_size(r, xaddr.noun, value);
    memcpy(surface, value, size);
}

Xaddr op_new(Receptor *r, Symbol noun, void *surface) {
    size_t current_index = r->data.cache_index;
    r->data.cache_index += _get_noun_size(r, noun, surface);
    r->data.current_xaddr++;
    r->data.xaddrs[r->data.current_xaddr].key = current_index;
    r->data.xaddrs[r->data.current_xaddr].noun = noun;
    Xaddr new_xaddr = {current_index, noun};
    r->data.xaddr_scape[current_index] = noun;
    op_set(r, new_xaddr, surface);
    return new_xaddr;
}

void _init_element(Receptor *r, char *label,Symbol element_spec,ElementSurface *es,int processCount, Process *processes) {
    int i;
    Xaddr elemX;
    elemX.key = element_spec;
    elemX.noun = CSPEC;

    es->name = op_new_noun(r, elemX, label);
    es->process_count = processCount;
    Process *p = &es->processes;
    for (i = 0; i < processCount; i++) {
        p->name = processes[i].name;
        p->function = processes[i].function;
	p++;
    }
}

#define BUFFER_SIZE 10000
Xaddr _op_new_rep(Receptor *r, Symbol rep_type, char *label, Symbol patternNoun, int processCount, Process *processes){
    char rs[BUFFER_SIZE];
    memset(rs, 0, BUFFER_SIZE);
    _init_element(r,label,rep_type,(ElementSurface *)rs,processCount,processes);
    REPS_SET_PATTERN_NOUN(rs,patternNoun);
    return op_new(r, rep_type, rs);
}

Xaddr op_new_string(Receptor *r, char *label, Symbol patternNoun, int processCount, Process *processes){
    return _op_new_rep(r,STRING_SPEC,label,patternNoun,processCount,processes);
}

Xaddr op_new_array(Receptor *r, char *label, Symbol patternNoun, int processCount, Process *processes){
    return _op_new_rep(r,ARRAY_SPEC,label,patternNoun,processCount,processes);
}

Xaddr op_new_pattern(Receptor *r, char *label, int childCount, Xaddr *children, int processCount, Process *processes) {
    char ps[BUFFER_SIZE];
    memset(ps, 0, BUFFER_SIZE);
    int i;
    _init_element(r,label,PATTERN_SPEC,(ElementSurface *)ps,processCount,processes);

    if (children == 0) {
        PATTERN_SET_SIZE(ps,childCount);
    } else {
        NounSurface *noun;
        ElementSurface *child_pattern_surface;
	int size = 0;
	Offset *pschildren = PATTERN_GET_CHILDREN(ps);
        for (i = 0; i < childCount; i++) {
            if (children[i].noun == NOUN_SPEC) {
                noun = (NounSurface *) op_get(r, children[i]);
                child_pattern_surface = (ElementSurface *) op_get(r, noun->namedElement);
            } else if (children[i].noun == PATTERN_SPEC) {
                child_pattern_surface = (ElementSurface *) op_get(r, children[i]);
            } else {
                raise_error("Unknown child element type %d", children[i].noun);
            }
            pschildren[i].noun.key = children[i].key;
            pschildren[i].noun.noun = children[i].noun;
            pschildren[i].offset = size;
            size += PATTERN_GET_SIZE(child_pattern_surface);
        }
	PATTERN_SET_SIZE(ps,size);
	PATTERN_SET_CHILDREN_COUNT(ps,childCount);
    }
return op_new(r, PATTERN_SPEC, ps);
}

ElementSurface *_get_pattern_spec(Receptor *r, Symbol patternName) {
    Xaddr px = {patternName, PATTERN_SPEC};
    return (ElementSurface *) op_get(r, px);
}

int op_exec(Receptor *r, Xaddr xaddr, FunctionName processName) {
    ElementSurface *ps = _get_noun_pattern_spec(r, xaddr.noun);
    Process *p = getProcess(ps, processName);
    return (*p->function)(r, op_get(r, xaddr));
}

int op_push_pattern(Receptor *r, Symbol patternName, void *surface) {
    SemStackFrame *ssf = &r->semStack[++r->semStackPointer];
    ssf->type.key = patternName;
    ssf->type.noun = PATTERN_SPEC;
    ElementSurface *ps = _get_pattern_spec(r, patternName);
    ssf->size = PATTERN_GET_SIZE(ps);
    memcpy(&r->valStack[r->valStackPointer], surface, ssf->size);
    r->valStackPointer += ssf->size;
}

#define NULL_XADDR(x) ((x.noun)==(x.key))

int getOffset(ElementSurface *ps, Symbol name) {
    int i;
    Offset *o = PATTERN_GET_CHILDREN(ps);
    while (!NULL_XADDR(o->noun)) {
        if (o->noun.key == name) {
            return o->offset;
        }
	o++;
    }
    raise_error2("offset not found for: %d in getOffset for patternSpec %d\n", name, ps->name);
}

ElementSurface *walk_path(Receptor *r, Xaddr xaddr, Symbol *path, int *offset) {
    ElementSurface *ps = _get_noun_pattern_spec(r, xaddr.noun);
    *offset = 0;
    int i = 0;
    while (path[i] != SYMBOL_PATH_TERMINATOR) {
        *offset += getOffset(ps, path[i]);
        ps = _get_noun_pattern_spec(r, path[i]);
        i++;
    }
    return ps;
}

void *op_setpath(Receptor *r, Xaddr xaddr, Symbol *path, void *value) {
    int offset;
    ElementSurface *ps = walk_path(r, xaddr, path, &offset);
    void *surface = &r->data.cache[xaddr.key + offset];
    return memcpy(surface, value, PATTERN_GET_SIZE(ps));
}

void *op_getpath(Receptor *r, Xaddr xaddr, Symbol *path) {
    int offset;
    walk_path(r, xaddr, path, &offset);
    return &r->data.cache[xaddr.key + offset];
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

//FIXME: this is needs to be implemented as a scape, not a linear scan of all Xaddrs!!
Symbol getSymbol(Receptor *r, char *label) {
    NounSurface *ns;
    Symbol noun;
    int i;
    for (i = 0; i <= r->data.current_xaddr; i++) {
        if (r->data.xaddrs[i].noun == NOUN_SPEC) {
            noun = r->data.xaddrs[i].key;
            ns = (NounSurface *) &r->data.cache[noun];
            if (!strcmp(label, ns->label)) {
                return noun;
            }
        }
    }
}

void init(Receptor *r) {
    int i;
    for (i = 0; i < DEFAULT_CACHE_SIZE; i++) r->data.xaddr_scape[i] = NULL_SYMBOL;

    r->data.cache_index = 0;
    r->semStackPointer = -1;
    r->valStackPointer = 0;
    r->patternSpecXaddr.key = PATTERN_SPEC;
    r->patternSpecXaddr.noun = CSPEC;
    r->arraySpecXaddr.key = ARRAY_SPEC;
    r->arraySpecXaddr.noun = CSPEC;
    r->stringSpecXaddr.key = STRING_SPEC;
    r->stringSpecXaddr.noun = CSPEC;
    r->data.current_xaddr = -1;


    // ********************** bootstrap built in types

    // INT
    Process int_processes[] = {
        {PRINT, &proc_int_print},
        {INC, &proc_int_inc},
        {ADD, &proc_int_add}
    };
    r->intPatternSpecXaddr = op_new_pattern(r, "INT", sizeof(int), 0, 3, int_processes);

    // POINT
    Symbol X = op_new_noun(r, r->intPatternSpecXaddr, "X");
    Symbol Y = op_new_noun(r, r->intPatternSpecXaddr, "Y");

    Process point_processes[] = {
        {PRINT, &proc_point_print}
    };

    // LINE
    Xaddr point_children[2] = {{X, NOUN_SPEC}, {Y, NOUN_SPEC}};
    r->pointPatternSpecXaddr = op_new_pattern(r, "POINT", 2, point_children, 1, point_processes);

    Symbol A = op_new_noun(r, r->pointPatternSpecXaddr, "A");
    Symbol B = op_new_noun(r, r->pointPatternSpecXaddr, "B");

    Xaddr line_children[2] = {{A, NOUN_SPEC}, {B, NOUN_SPEC}};

    Process line_processes[] = {
        {PRINT, &proc_line_print}
    };

    r->linePatternSpecXaddr = op_new_pattern(r, "LINE", 2, line_children, 1, line_processes);

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

void dump_children_array(Offset *children,int count) {
    int i = 0;
    while (i < count) {
        if (i != 0) {
            printf(",");
        }
        printf("{ %d, %d }(%d)", children[i].noun.key, children[i].noun.noun, children[i].offset);
        i++;
    }
}


void dump_process_array(Process *process,int count) {
    int i = 0;
    while (i < count) {
        if (i != 0) {
            printf(",");
        }
        printf("{ %d, %zu }", process[i].name, (size_t) process[i].function);
        i++;
    }
}

void dump_reps_spec(Receptor *r, ElementSurface *rs,char * type){
    printf("%s Spec\n",type);
    printf("    name: %s(%d)\n", noun_label(r, rs->name), rs->name);
    printf("    patternNoun: %d \n", REPS_GET_PATTERN_NOUN(rs));
//    dump_process_array(rs->processes);
    printf("\n");
}

void dump_pattern_spec(Receptor *r, ElementSurface *ps) {
    printf("Pattern Spec\n");
    printf("    name: %s(%d)\n", noun_label(r, ps->name), ps->name);
    printf("    size: %d\n", (int) PATTERN_GET_SIZE(ps));
    int count = PATTERN_GET_CHILDREN_COUNT(ps);
    printf("    %d children: ",count);
    dump_children_array(PATTERN_GET_CHILDREN(ps),count);
    printf("\n    %d processes: ",ps->process_count);
    dump_process_array(&ps->processes,ps->process_count);
    printf("\n");
}

void dump_pattern_value(Receptor *r, ElementSurface *ps, void *surface) {
    Process *print_proc;
    print_proc = getProcess(ps, PRINT);
    if (print_proc) {
        (*print_proc->function)(r, surface);
    } else {
        hexDump("hexDump of surface", surface, PATTERN_GET_SIZE(ps));
    }
}

void dump_array_value(Receptor *r, ElementSurface *rs, void *surface) {
    int count = *(int*)surface;
    surface += sizeof(int);
    ElementSurface *ps = _get_reps_pattern_spec(r,rs);
    Symbol patternNoun = REPS_GET_PATTERN_NOUN(rs);
    printf(" %s(%d) array of %d %s(%d)s\n",noun_label(r,rs->name),rs->name,count,noun_label(r,patternNoun),patternNoun );
    while (count > 0) {
	printf("    ");
	dump_pattern_value(r,ps,surface);
	printf("\n");
	surface += PATTERN_GET_SIZE(ps);
	count--;
    }
}

void dump_string_value(Receptor *r, ElementSurface *rs, void *surface) {
    int count = 0;
    ElementSurface *ps = _get_reps_pattern_spec(r,rs);
    Symbol patternNoun = REPS_GET_PATTERN_NOUN(rs);
    printf(" %s(%d) string of %s(%d)\n",noun_label(r,rs->name),rs->name,noun_label(r,patternNoun),patternNoun);
    while (*(int *)surface != STRING_TERMINATOR) {
	if (*(int *)surface == ESCAPE_STRING_TERMINATOR) {
	    surface += sizeof(int);
	}
	printf("    ");
	dump_pattern_value(r,ps,surface);
	printf("\n");
	surface += PATTERN_GET_SIZE(ps);
	count++;
    }
    printf(" %d elements found\n",count);
}


void dump_noun(Receptor *r, NounSurface *ns) {
    printf("Noun      { %d, %5d } %s", ns->namedElement.key, ns->namedElement.noun, ns->label);
}

void dump_xaddr(Receptor *r, Xaddr xaddr, int indent_level) {
    int i;
    ElementSurface *es;
    NounSurface *ns;
    void *surface;
    int key = xaddr.key;
    int noun = xaddr.noun;
    switch (noun) {
        case PATTERN_SPEC:
	    dump_pattern_spec(r, (ElementSurface *)&r->data.cache[key]);
            break;
        case NOUN_SPEC:
            ns = (NounSurface *) &r->data.cache[key];
            dump_noun(r, ns);
            break;
        case ARRAY_SPEC:
            dump_reps_spec(r, (ElementSurface *)&r->data.cache[key], "Array");
	    break;
        case STRING_SPEC:
	    dump_reps_spec(r, (ElementSurface *)&r->data.cache[key], "String");
	    break;
        default:
            surface = op_get(r, noun_to_xaddr(noun));
            ns = (NounSurface *) surface;
            printf("%s : ", ns->label);
	    es = (ElementSurface *) op_get(r, ns->namedElement);
	    switch(ns->namedElement.noun) {
	    case PATTERN_SPEC:
		dump_pattern_value(r, es, op_get(r, xaddr));
		break;
	    case ARRAY_SPEC:
		dump_array_value(r,es,op_get(r,xaddr));
		break;
	    case STRING_SPEC:
		dump_string_value(r,es,op_get(r,xaddr));
		break;
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

void dump_stack(Receptor *r) {
    int i, v = 0;
    char *unknown = "<unknown>";
    char *label;
    ElementSurface *ps;
    for (i = 0; i <= r->semStackPointer; i++) {
        Xaddr type = r->semStack[i].type;
        if (type.noun == PATTERN_SPEC) {
            ps = (ElementSurface *) &r->data.cache[type.key];
            label = noun_label(r, ps->name);

        }
        else {
            label = unknown;
        }
        printf("\nStack frame: %d is a %s({%d,%d}) size:%d\n", i, label, type.key, type.noun, r->semStack[i].size);
        printf("   Value:");
        dump_pattern_value(r, ps, &r->valStack[v]);
        v += r->semStack[i].size;
        printf("\n");
    }

}

#endif
