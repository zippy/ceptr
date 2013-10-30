#ifndef _CEPTR_H
#define _CEPTR_H

#define DEFAULT_ARRAY_SIZE 100
#define DEFAULT_CACHE_SIZE 10000
#define STACK_SIZE 10000
#define TERMINATOR 0xFFFF
#define OPERANDS_SIZE (sizeof(Xaddr) * 2)
#include <signal.h>
#include <string.h>
#include <stdio.h>

char error[255];
#define raise_error(error_msg, val)		\
    printf(error_msg, val);			\
    raise(SIGINT);


enum Symbols {
    INT,POINT,X,Y,LINE,A,B,_LAST
};

typedef int Symbol;

enum FunctionNames {
    INC, ADD
};

typedef int FunctionName;
enum Frametypes {
    XADDR, PATTERN
};

typedef int Frametype;

typedef struct {
    Symbol name;
    Symbol patternName;
} Noun;

typedef struct {
    Noun *noun;
    int offset;
} Offset;

typedef struct {
    int key;
    Noun *noun;
} Xaddr;


enum Opcodes {
    RETURN,PUSH_IMMEDIATE
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

#define BASE_NOUNS 4
Noun base_noun_DATA[BASE_NOUNS] = {
    { X, INT },
    { Y, INT },
    { A, POINT },
    { B, POINT }
};

typedef struct {
    FunctionName name;
    int (*function)(void *,Xaddr);
} Process;

typedef struct {
    Symbol name;
    size_t size;
    Offset children[DEFAULT_ARRAY_SIZE];
    Process processes[DEFAULT_ARRAY_SIZE];
} PatternSpec;

typedef struct {
    SemStackFrame semStack[STACK_SIZE];
    int semStackPointer;

    char valStack[DEFAULT_CACHE_SIZE];
    int valStackPointer;
    Symbol last_symbol;

    Noun noun_DATA[DEFAULT_ARRAY_SIZE];

    PatternSpec spec_DATA[DEFAULT_ARRAY_SIZE];

    Noun * xaddr_SCAPE[DEFAULT_CACHE_SIZE];

    char cache_DATA[DEFAULT_CACHE_SIZE];

    int last_noun;
    int last_spec;
} Receptor;


Symbol new_symbol(Receptor *r) {
    return r->last_symbol++;
}

PatternSpec *getPatternSpec(Receptor *r, Symbol patternName){
    int i;
    for (i=0; i<DEFAULT_ARRAY_SIZE; i++) {
	if (r->spec_DATA[i].name == patternName) {
	    return &r->spec_DATA[i];
	}
    }
    raise_error("pattern not found: %d\n", patternName);
}

int getOffset(PatternSpec *ps, Symbol name) {
    int i;
    for (i=0; ps->children[i].noun != 0; i++) {
	if (ps->children[i].noun->name == name) {
	    return ps->children[i].offset;
	}
    }
    printf("in getOffset for patterSpec %d\n", ps->name);
    raise_error("offset not found: %d\n", name);
}

Noun *newNoun(Receptor *r,char label[], Symbol patternName) {
    // FIXME do something useful with label;
    if ((r->last_noun +1) == DEFAULT_ARRAY_SIZE) {
	return 0;
    }
    r->noun_DATA[++r->last_noun].name = new_symbol(r);
    r->noun_DATA[r->last_noun].patternName = patternName;
    return &r->noun_DATA[r->last_noun];
}

Noun *getNoun(Receptor *r, Symbol name){
    int i;
    for (i=0; i<DEFAULT_ARRAY_SIZE; i++) {
	if (r->noun_DATA[i].name == name) {
	    return &r->noun_DATA[i];
	}
    }
    raise_error("noun not found: %d\n", name);
}

Process *getProcess(PatternSpec *ps, FunctionName name){
    int i;
    for (i=0; i<DEFAULT_ARRAY_SIZE; i++) {
	if (ps->processes[i].name == name) {
	    return &ps->processes[i];
	}
    }
}

PatternSpec* _makeBasePatternSpec(Receptor *r, Symbol name, size_t size, int processCount, Process processes[]) {
    PatternSpec *ps = &r->spec_DATA[r->last_spec];
    ps->name = name;
    ps->size = size;
    int i;
    for (i=0; i<processCount; i++) {
	ps->processes[i] = processes[i];
    }
    return ps;
}

PatternSpec* makeBasePatternSpec(Receptor *r, Symbol name, size_t size, int processCount, Process processes[]) {
    ++r->last_spec;
    _makeBasePatternSpec(r,name,size,processCount,processes);
}

PatternSpec* makePatternSpec(Receptor *r, Symbol name, int childCount, Symbol children[], int processCount, Process processes[]) {
    int i;
    Noun *n;
    ++r->last_spec;
    PatternSpec *ps = &r->spec_DATA[r->last_spec];
    Offset *o;
    size_t current_size = 0;
    for (i=0; i<childCount; i++) {
	n = getNoun(r,children[i]);
	o = &ps->children[i];
	o->noun = n;
	o->offset = current_size;
	current_size += getPatternSpec(r,n->patternName)->size;
    }
    _makeBasePatternSpec(r,name, current_size, processCount, processes);
}


void* op_set(Receptor *r,Xaddr xaddr, void *value){
    PatternSpec *ps = getPatternSpec(r,xaddr.noun->patternName);
    // FIXME: semfault if not
    r->xaddr_SCAPE[xaddr.key] = xaddr.noun;
    void *surface = &r->cache_DATA[xaddr.key];
    return memcpy(surface, value, ps->size);
}

PatternSpec* walk_path(Receptor *r,Xaddr xaddr, Symbol* path, int* offset ){
    PatternSpec *ps = getPatternSpec(r,xaddr.noun->patternName);
    *offset = 0;
    int i=0;
    while (path[i]!=TERMINATOR) {
	*offset += getOffset(ps, path[i]);
	ps = getPatternSpec(r, getNoun(r,path[i])->patternName );
	i++;
    }
    return ps;
}

void* op_setpath(Receptor *r,Xaddr xaddr, Symbol* path, void *value){
    int offset;
    PatternSpec *ps = walk_path(r,xaddr, path, &offset);
    void *surface = &r->cache_DATA[xaddr.key+offset];
    return memcpy(surface, value, ps->size);
}

void* op_getpath(Receptor *r,Xaddr xaddr, Symbol* path){
    int offset;
    walk_path(r,xaddr, path, &offset);
    return &r->cache_DATA[xaddr.key+offset];
}

void *op_get(Receptor *r,Xaddr xaddr){
    if (r->xaddr_SCAPE[xaddr.key] == 0  ||
	r->xaddr_SCAPE[xaddr.key]->name != xaddr.noun->name) {
	return 0;
    }
    return &r->cache_DATA[xaddr.key];
}

int op_exec(Receptor *r,Xaddr xaddr, FunctionName processName){
    Process *p = getProcess(getPatternSpec(r,xaddr.noun->patternName), processName);
    (*p->function)(r,xaddr);
}

int op_push_pattern(Receptor *r,Symbol patternName, void* surface){
    SemStackFrame *ssf = &r->semStack[++r->semStackPointer];
    ssf->type = PATTERN;
    ssf->size = getPatternSpec(r,patternName)->size;
    memcpy(&r->valStack[r->valStackPointer], surface, ssf->size);
    r->valStackPointer += ssf->size;
}

int proc_inc(Receptor *r,Xaddr this) {
    void *surface = op_get(r,this);
    ++*(int*)(surface);
    return 1;
}

int proc_add(Receptor *r,Xaddr this){
    int *surface = (int*) op_get(r,this);
    // semCheck please
    int *stackSurface = (int*) &r->valStack[ r->valStackPointer - r->semStack[r->semStackPointer].size ];
    *stackSurface = *surface + *stackSurface;
    return 1;
}

typedef struct {
    Symbol name;
    int valueOffset;
    // pad to size of full operand
    char padding[OPERANDS_SIZE  - sizeof(Symbol) - sizeof(int)];
} ImmediatePatternOperand;

int run(Receptor *r,Instruction *instructions, void *values){
    int counter = 0;
    ImmediatePatternOperand *op;
    while(1) {
	switch(instructions[counter].opcode ) {
	case RETURN:
	    return;
	case PUSH_IMMEDIATE:
	    op = (ImmediatePatternOperand*)&instructions[counter].operands;
	    op_push_pattern(r,op->name, (char*)values + op->valueOffset);
	    break;
	}
	counter++;
    }
}

void init(Receptor *r) {

    r->last_noun = BASE_NOUNS-1;
    r->last_spec = -1;
    r->last_symbol = _LAST;
    r->semStackPointer= -1;
    r->valStackPointer= 0;
    int i;
    for(i=0;i<BASE_NOUNS;i++) r->noun_DATA[i]=base_noun_DATA[i];

    Process pr[2] = {{ INC, &proc_inc }, { ADD, &proc_add }};
    makeBasePatternSpec(r,INT, sizeof(Symbol), 2, pr);

    Symbol pointChildren[2] = { X, Y };
    makePatternSpec(r,POINT, 2, pointChildren, 0, 0);

    Symbol lineChildren[2] = { A, B };
    makePatternSpec(r,LINE, 2, lineChildren, 0, 0);
}



#endif
