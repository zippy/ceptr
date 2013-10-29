#ifndef _CEPTR_H
#define _CEPTR_H

#define DEFAULT_ARRAY_SIZE 100
#define DEFAULT_CACHE_SIZE 10000
#define STACK_SIZE 10000
#define TERMINATOR 0xFFFF
#include <signal.h>
#include <string.h>
#include <stdio.h>


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
	Symbol patternName;	 //  Symbol patternSpecName;
} Noun;

typedef struct {
	Noun *noun;
	int offset;
} Offset;

typedef struct {
	int key;
	Noun *noun;
} Xaddr;

typedef struct {
	FunctionName name;
	int (*function)(Xaddr);
} Process;

typedef struct {
	Symbol name;
	size_t size;
	Offset children[DEFAULT_ARRAY_SIZE];
	Process processes[DEFAULT_ARRAY_SIZE];
} PatternSpec;

typedef struct {
	Frametype type;
	int size;	
} SemStackFrame;

SemStackFrame semStack[STACK_SIZE];
int semStackPointer = -1;

char valStack[DEFAULT_CACHE_SIZE];	
int valStackPointer = 0;
Symbol last_symbol = _LAST;

Noun noun_DATA[DEFAULT_ARRAY_SIZE] = {
	{ X, INT },
	{ Y, INT },
	{ A, POINT },
	{ B, POINT }
};

PatternSpec spec_DATA[DEFAULT_ARRAY_SIZE];

Noun *xaddr_SCAPE[DEFAULT_CACHE_SIZE];

char surfaces_TMP[DEFAULT_CACHE_SIZE];

char cache_DATA[DEFAULT_CACHE_SIZE];

int last_noun = 3;
int last_spec = -1;

char error[255];

#define raise_error(error_msg, val)\
printf(error_msg, val);\
raise(SIGINT);

Symbol new_symbol() {
	return last_symbol++;
}

void *createSurface(Noun *whatFor) {
	return surfaces_TMP;
}

PatternSpec *getPatternSpec(Symbol patternName){
	int i;
	for (i=0; i<DEFAULT_ARRAY_SIZE; i++) {
		if (spec_DATA[i].name == patternName) {
			return &spec_DATA[i];
		}
	}
	raise_error("pattern not found: %d", patternName);
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

Noun *newNoun(char label[], Symbol patternName) {
	// FIXME do something useful with label;
	if ((last_noun +1) == DEFAULT_ARRAY_SIZE) {
		return 0;
	}
	noun_DATA[++last_noun].name = new_symbol();
	noun_DATA[last_noun].patternName = patternName;
	return &noun_DATA[last_noun];
}

Noun *getNoun(Symbol name){
	int i;
	for (i=0; i<DEFAULT_ARRAY_SIZE; i++) {
		if (noun_DATA[i].name == name) {
			return &noun_DATA[i];
		}
	}
	raise_error("noun not found: %d", name);
}

Process *getProcess(PatternSpec *ps, FunctionName name){
	int i;
	for (i=0; i<DEFAULT_ARRAY_SIZE; i++) {
		if (ps->processes[i].name == name) {
			return &ps->processes[i];
		}
	}
}


PatternSpec* _makeBasePatternSpec(Symbol name, size_t size, int processCount, Process processes[]) {
	PatternSpec *ps = &spec_DATA[last_spec];
	ps->name = name;
	ps->size = size;
	int i;
	for (i=0; i<processCount; i++) {
		ps->processes[i] = processes[i];
	}	
	return ps;
}

PatternSpec* makeBasePatternSpec(Symbol name, size_t size, int processCount, Process processes[]) {
	++last_spec;
	_makeBasePatternSpec(name,size,processCount,processes);
}

PatternSpec* makePatternSpec(Symbol name, int childCount, Symbol children[], int processCount, Process processes[]) {
	int i;
	Noun *n;
	++last_spec;
	PatternSpec *ps = &spec_DATA[last_spec];
	Offset *o;
	size_t current_size = 0;
	for (i=0; i<childCount; i++) {
		n = getNoun(children[i]);
		o = &ps->children[i];
		o->noun = n;
		o->offset = current_size;
		current_size += getPatternSpec(n->patternName)->size;		
	}
	_makeBasePatternSpec(name, current_size, processCount, processes);
}


void* op_set(Xaddr xaddr, void *value){
	PatternSpec *ps = getPatternSpec(xaddr.noun->patternName);
	// FIXME: semfault if not
	xaddr_SCAPE[xaddr.key] = xaddr.noun;
	void *surface = &cache_DATA[xaddr.key];
	return memcpy(surface, value, ps->size);
}

PatternSpec* walk_path(Xaddr xaddr, Symbol* path, int* offset ){
	PatternSpec *ps = getPatternSpec(xaddr.noun->patternName);
	*offset = 0;
	int i=0;
	while (path[i]!=TERMINATOR) {
		*offset += getOffset(ps, path[i]);		
		ps = getPatternSpec( getNoun(path[i])->patternName );
		i++;
	}	
	return ps;	
}

void* op_setpath(Xaddr xaddr, Symbol* path, void *value){
	int offset;
	PatternSpec *ps = walk_path(xaddr, path, &offset);
	void *surface = &cache_DATA[xaddr.key+offset];	
	return memcpy(surface, value, ps->size);
}

void* op_getpath(Xaddr xaddr, Symbol* path){
	int offset;
	walk_path(xaddr, path, &offset);
	return &cache_DATA[xaddr.key+offset];	
}

void *op_get(Xaddr xaddr){
	if (xaddr_SCAPE[xaddr.key] == 0  ||
		xaddr_SCAPE[xaddr.key]->name != xaddr.noun->name) {
		return 0;
	}
	return &cache_DATA[xaddr.key];
}

int op_exec(Xaddr xaddr, FunctionName processName){
	Process *p = getProcess(getPatternSpec(xaddr.noun->patternName), processName);
	(*p->function)(xaddr);
}

int op_push_pattern(Symbol patternName, void* surface){
	SemStackFrame *ssf = &semStack[++semStackPointer];
	ssf->type = PATTERN;
	ssf->size = getPatternSpec(patternName)->size;
	memcpy(&valStack[valStackPointer], surface, ssf->size);
	valStackPointer += ssf->size;
}

int proc_inc(Xaddr this) {
	void *surface = op_get(this);
	++*(int*)(surface);
	return 1;
}

int proc_add(Xaddr this){
	int *surface = (int*) op_get(this);
	// semCheck please
	int *stackSurface = (int*) &valStack[ valStackPointer - semStack[semStackPointer].size ];
	*stackSurface = *surface + *stackSurface;
	return 1;
}

void init() { 
	Process pr[2] = {{ INC, &proc_inc }, { ADD, &proc_add }};
	makeBasePatternSpec(INT, sizeof(Symbol), 2, pr);

	Symbol pointChildren[2] = { X, Y };
	makePatternSpec(POINT, 2, pointChildren, 0, 0);
	
	Symbol lineChildren[2] = { A, B };
	makePatternSpec(LINE, 2, lineChildren, 0, 0);	
}

#endif


