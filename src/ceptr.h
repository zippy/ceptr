#ifndef _CEPTR_H
#define _CEPTR_H

#define DEFAULT_ARRAY_SIZE 100
#define DEFAULT_CACHE_SIZE 10000
#include <signal.h>
#include <string.h>

enum Symbols {
	INT,POINT,X,Y,LINE,_LAST
};	

typedef int Symbol;

typedef struct {
	Symbol name;      
	Symbol patternName;	 //  Symbol patternSpecName;
} Noun;

typedef struct {
	Noun *noun;
	int offset;
} Offset;

typedef struct {
	Symbol name;
	size_t size;
	Offset children[DEFAULT_ARRAY_SIZE];
} PatternSpec;

typedef struct {
	int key;
	Noun *noun;
} Xaddr;


Symbol last_symbol = _LAST;

Noun noun_DATA[DEFAULT_ARRAY_SIZE] = {
	{ X, INT },
	{ Y, INT }
};

PatternSpec spec_DATA[DEFAULT_ARRAY_SIZE];

Noun *xaddr_SCAPE[DEFAULT_CACHE_SIZE];

char surfaces_TMP[DEFAULT_CACHE_SIZE];

char cache_DATA[DEFAULT_CACHE_SIZE];

int last_noun = 1;


Symbol new_symbol() {
	return last_symbol++;
}

void *createSurface(Noun *whatFor) {
	return surfaces_TMP;
}

Noun *getNoun(Symbol name){
	int i;
	for (i=0; i<DEFAULT_ARRAY_SIZE; i++) {
		if (noun_DATA[i].name == name) {
			return &noun_DATA[i];
		}
	}
}

PatternSpec *getPatternSpec(Symbol patternName){
	int i;
	for (i=0; i<DEFAULT_ARRAY_SIZE; i++) {
		if (spec_DATA[i].name == patternName) {
			return &spec_DATA[i];
		}
	}	
}

int getOffset(PatternSpec *ps, Symbol name) {
	int i;
	for (i=0; ps->children[i].noun != 0; i++) {
		if (ps->children[i].noun->name == name) {
			return ps->children[i].offset;
		}
	}		
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


void* set(Xaddr xaddr, void *value){
	PatternSpec *ps = getPatternSpec(xaddr.noun->patternName);
	xaddr_SCAPE[xaddr.key] = xaddr.noun;
	void *surface = &cache_DATA[xaddr.key];
	return memcpy(surface, value, ps->size);
}

void *get(Xaddr xaddr){
	if (xaddr_SCAPE[xaddr.key] == 0  ||
		xaddr_SCAPE[xaddr.key]->name != xaddr.noun->name) {
		return 0;
	}
	return &cache_DATA[xaddr.key];
}


void init() { 
	PatternSpec int_spec = { INT, 4, {{}}};
	spec_DATA[INT] = int_spec;
	
	PatternSpec point_spec = { POINT, 8, {{ getNoun(X), 0 }, { getNoun(Y), 4 }} };
	spec_DATA[POINT] = point_spec; 
	// 
	// 
	// PatternSpec point_spec = { LINE, 16, {{ getNoun(A), 0 }, { getNoun(B), 8 }} };
	// spec_DATA[POINT] = point_spec		
}

#endif


