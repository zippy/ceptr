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
    int key;
    Symbol noun;
	// Xaddr* type;
	// Symbol type;
	// Symbol name;
} Xaddr;

typedef struct {
    Xaddr noun;
    int offset;
} Offset;

typedef struct {    
	Xaddr namedElement;
	char* label;
} NounSurface;


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
    PatternSpec specs[DEFAULT_ARRAY_SIZE];
    Xaddr xaddrs[DEFAULT_CACHE_SIZE];
    char cache[DEFAULT_CACHE_SIZE];
	int current_xaddr;
    int last_spec;
	int cache_index;
} Data;

typedef struct {
    SemStackFrame semStack[STACK_SIZE];
    int semStackPointer;
    char valStack[DEFAULT_CACHE_SIZE];
    int valStackPointer;
	Xaddr patternSpecXaddr;
    Data data;
} Receptor;
// 
// typdef struct {
// 	int size;
// 	Symbol patternName;
// } ArrayHeader;
// 
// void *array_get(){
// 	int size = 0;
// 	return _array_get(r,array,n,&size);
// }
// 
// void *_array_get(Receptor *r, void *array, int n, int * size){
// 	ArrayHeader * ah = (ArrayHeader *) array;
// 	if (n < 0 || n >= ah->size) {
// 		raise_error("array index out of bounds: %d", n);
// 	}
// 	*size = getPatternSpec(r, ah->patternName)->size;
// 	return ((char *)array) + *size * n;
// }
// 
// 
// void *array_set(Receptor *r, void * array, int n, void * surface){
// 	int size;
// 	void *array = _array_get(r,array,n,&size);
// 	
// 	memcpy(array_get(r, array, n), surface, size);
// }
// 
// PatternSpec *getPatternSpec(Receptor *r, Symbol patternName){
//     int i;
//     for (i=0; i<DEFAULT_ARRAY_SIZE; i++) {
// 	if (r->data.specs[i].name == patternName) {
// 	    return &r->data.specs[i];
// 	}
//     }
//     raise_error("pattern not found: %d\n", patternName);
// }
// 
// int getOffset(PatternSpec *ps, Symbol name) {
//     int i;
//     for (i=0; ps->children[i].noun != 0; i++) {
// 	if (ps->children[i].noun->name == name) {
// 	    return ps->children[i].offset;
// 	}
//     }
//     printf("in getOffset for patterSpec %d\n", ps->name);
//     raise_error("offset not found: %d\n", name);
// }
// 
// Noun *newNoun(Receptor *r,char label[], Symbol patternName) {
//     // FIXME do something useful with label;
//     if ((r->data.last_noun +1) == DEFAULT_ARRAY_SIZE) {
// 	return 0;
//     }
//     r->data.nouns[++r->data.last_noun].name = new_symbol(r);
//     r->data.nouns[r->data.last_noun].patternName = patternName;
//     return &r->data.nouns[r->data.last_noun];
// }
// 
// Noun *getNoun(Receptor *r, Symbol name){
//     int i;
//     for (i=0; i<DEFAULT_ARRAY_SIZE; i++) {
// 	if (r->data.nouns[i].name == name) {
// 	    return &r->data.nouns[i];
// 	}
//     }
//     raise_error("noun not found: %d\n", name);
// }
// 
// Process *getProcess(PatternSpec *ps, FunctionName name){
//     int i;
//     for (i=0; i<DEFAULT_ARRAY_SIZE; i++) {
// 	if (ps->processes[i].name == name) {
// 	    return &ps->processes[i];
// 	}
//     }
// }
// 
// PatternSpec* _makeBasePatternSpec(Receptor *r, Symbol name, size_t size, int processCount, Process processes[]) {
//     PatternSpec *ps = &r->data.specs[r->data.last_spec];
// 	Xaddr ps_xaddr = { name, }
//     ps->name = name;
//     ps->size = size;
//     int i;
//     for (i=0; i<processCount; i++) {
// 	ps->processes[i] = processes[i];
//     }
//     return ps;
// }
// 
// PatternSpec* makeBasePatternSpec(Receptor *r, Symbol name, size_t size, int processCount, Process processes[]) {
//     ++r->data.last_spec;
//     _makeBasePatternSpec(r,name,size,processCount,processes);
// }
// 
// PatternSpec* makePatternSpec(Receptor *r, Symbol name, int childCount, Symbol children[], int processCount, Process processes[]) {
//     int i;
//     Noun *n;
//     ++r->data.last_spec;
//     PatternSpec *ps = &r->data.specs[r->data.last_spec];
//     Offset *o;
//     size_t current_size = 0;
//     for (i=0; i<childCount; i++) {
// 	n = getNoun(r,children[i]);
// 	o = &ps->children[i];
// 	o->noun = n;
// 	o->offset = current_size;
// 	current_size += getPatternSpec(r,n->patternName)->size;
//     }
//     _makeBasePatternSpec(r,name, current_size, processCount, processes);
// }

#define NULL_SURFACE 0
enum Symbols {
    NOUN_SPEC = -3, CSPEC=-2, PATTERN_SPEC=-1
};

size_t _get_noun_size(Receptor *r, Symbol noun){	
// 
	if (noun == PATTERN_SPEC){
		return sizeof(PatternSpec);
	}
	Xaddr* elementXaddr = &((NounSurface*)&r->data.cache[noun])->namedElement;
	if (elementXaddr->noun == PATTERN_SPEC) {
		return ((PatternSpec *)&r->data.cache[elementXaddr->key])->size;		
	} 
	raise_error("unknown noun type %d\n", elementXaddr->noun);
}

Symbol op_new_noun(Receptor *r, Symbol xaddr_key, Symbol xaddr_type, char* label) {
	NounSurface ns;
	ns.namedElement.key = xaddr_key;
	ns.namedElement.noun = xaddr_type;
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

Symbol op_new(Receptor *r, Xaddr elementType) {
	size_t current_index = r->data.cache_index;
	r->data.cache_index += _get_noun_size(r, elementType.noun);	
	r->data.current_xaddr++;
	r->data.xaddrs[r->data.current_xaddr].key = current_index;
	r->data.xaddrs[r->data.current_xaddr].noun = elementType.noun;
	return current_index;
}

int _op_set(Receptor *r, int xaddr_key, Symbol xaddr_noun, void* value) {	
    void *surface = &r->data.cache[xaddr_key];
	printf("copying %d\n", _get_noun_size(r, xaddr_noun));
    memcpy(surface, value, _get_noun_size(r, xaddr_noun));	
	return xaddr_key;
}

int op_set(Receptor *r,Xaddr xaddr, void *value){
	return _op_set(r, xaddr.key, xaddr.noun, value);
}
// // 
// // PatternSpec* walk_path(Receptor *r,Xaddr xaddr, Symbol* path, int* offset ){
// //     PatternSpec *ps = getPatternSpec(r,xaddr.noun->patternName);
// //     *offset = 0;
// //     int i=0;
// //     while (path[i]!=TERMINATOR) {
// // 	*offset += getOffset(ps, path[i]);
// // 	ps = getPatternSpec(r, getNoun(r,path[i])->patternName );
// // 	i++;
// //     }
// //     return ps;
// // }
// 
// void* op_setpath(Receptor *r,Xaddr xaddr, Symbol* path, void *value){
//     int offset;
//     PatternSpec *ps = walk_path(r,xaddr, path, &offset);
//     void *surface = &r->data.cache[xaddr.key+offset];
//     return memcpy(surface, value, ps->size);
// }
// 
// void* op_getpath(Receptor *r,Xaddr xaddr, Symbol* path){
//     int offset;
//     walk_path(r,xaddr, path, &offset);
//     return &r->data.cache[xaddr.key+offset];
// }
// 
void *op_get(Receptor *r,Xaddr xaddr){
//     if (r->data.xaddrs[xaddr.key] == 0  ||
// 	r->data.xaddrs[xaddr.key]->name != xaddr.noun->name) {
// 	return 0;
//     }
//     return &r->data.cache[xaddr.key];
}
// 
// int op_exec(Receptor *r,Xaddr xaddr, FunctionName processName){
//     Process *p = getProcess(getPatternSpec(r,xaddr.noun->patternName), processName);
//     (*p->function)(r,xaddr);
// }
// 
int op_push_pattern(Receptor *r,Symbol patternName, void* surface){
//     SemStackFrame *ssf = &r->semStack[++r->semStackPointer];
//     ssf->type = PATTERN;
//     ssf->size = getPatternSpec(r,patternName)->size;
//     memcpy(&r->valStack[r->valStackPointer], surface, ssf->size);
//     r->valStackPointer += ssf->size;
}

int proc_int_inc(Receptor *r,Xaddr this) {
    void *surface = op_get(r,this);
    ++*(int*)(surface);
    return 1;
}

int proc_int_add(Receptor *r,Xaddr this){
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

	r->data.cache_index = 0;
    r->semStackPointer= -1;
    r->valStackPointer= 0;
	r->patternSpecXaddr.key = PATTERN_SPEC;
	r->patternSpecXaddr.noun = CSPEC;
	r->data.current_xaddr = -1;

	
	Symbol INT = op_new_noun(r, PATTERN_SPEC, CSPEC, "INT");
	
	PatternSpec int_ps = {
		INT, sizeof(int), {}, {{ INC, &proc_int_inc }, { ADD, &proc_int_add }}
	};
	
	Xaddr int_noun_xaddr = { INT, PATTERN_SPEC };
	Symbol INT_PS = op_new(r, int_noun_xaddr);
	
	_op_set(r, INT_PS, PATTERN_SPEC, &int_ps);
	
	printf("INT_PS %d\n", INT_PS);
	
	Symbol X = op_new_noun(r, INT_PS, PATTERN_SPEC, "X");
	Symbol Y = op_new_noun(r, INT_PS, PATTERN_SPEC, "Y");
	
	Symbol POINT = op_new_noun(r, PATTERN_SPEC, CSPEC, "POINT");
	
	PatternSpec point_ps = {
		POINT, sizeof(int)*2, {{{X, -3}, 0}, {{Y, -3}, sizeof(int)}}, {}
	};
	Xaddr point_noun_xaddr = { POINT, PATTERN_SPEC };
	Symbol POINT_PS = op_new(r, point_noun_xaddr);
	
	_op_set(r, POINT_PS, PATTERN_SPEC, &point_ps);
	
	// 
	// 
	// 
	// Symbol X = op_new_noun(r, INT, "X"); //_op_set(r, op_new(r, INT),   INT, &x_ns);
	// 
	// NounSurface y_ns = { INT, "Y" };
	// Symbol Y = _op_set(r, op_new(r, INT),   INT, &y_ns);
	// 
	// Symbol POINT = op_new_noun(r, PATTERN_SPEC);
	// PatternSpec point_ps = {
	// 	POINT, sizeof(int)*2, { X, Y }, {}
	// };
	// _op_set(r, POINT, PATTERN_SPEC, *point_ps);
	// 
	// 
	// int here_val[2] = { 2, 3 };
	// NounSurface here_ns = { POINT, "HERE" };
	// Symbol HERE = op_set(r, op_new(r, POINT), POINT, &here_ns);
	// op_set(r, op_new(r, HERE), HERE, &here_val);
	// 
	

	// Symbol A = _op_set(r, op_new(r, POINT),   POINT, "A");
	// Symbol B = _op_set(r, op_new(r, POINT),   POINT, "B");
	// 
	// Symbol LINE = op_new(r, PATTERN_SPEC);
	// PatternSpec line_ps = {
	// 	LINE, sizeof(int)*4, { A, B }, {}
	// };
	// _op_set(r, LINE, PATTERN_SPEC, *line_ps);

}



#endif
