#ifndef _CEPTR_H
#define _CEPTR_H

#define DEFAULT_ARRAY_SIZE 100
#define DEFAULT_CACHE_SIZE 10000
#include <signal.h>
#include <string.h>

enum Symbols {
	INT,POINT,X,Y,LINE,A,B,_LAST
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
	{ Y, INT },
	{ A, POINT },
	{ B, POINT }
};

PatternSpec spec_DATA[DEFAULT_ARRAY_SIZE];

Noun *xaddr_SCAPE[DEFAULT_CACHE_SIZE];

char surfaces_TMP[DEFAULT_CACHE_SIZE];

char cache_DATA[DEFAULT_CACHE_SIZE];

int last_noun = 1;
int last_spec = 0;

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

Noun *getNoun(Symbol name){
	int i;
	for (i=0; i<DEFAULT_ARRAY_SIZE; i++) {
		if (noun_DATA[i].name == name) {
			return &noun_DATA[i];
		}
	}
	return 0;
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

PatternSpec* makePatternSpec(Symbol name, int childCount, Symbol children[]) {
	int i;
	Noun *n;
	PatternSpec *ps = &spec_DATA[last_spec++];
	Offset *o;
	size_t current_size = 0;
	ps->name = name;
	for (i=0; i<childCount; i++) {
		n = getNoun(children[i]);
		o = &ps->children[i];
		o->noun = n;
		o->offset = current_size;
		current_size += getPatternSpec(n->patternName)->size;		
	}
	ps->size = current_size;
}

PatternSpec* makeBasePatternSpec(Symbol name, size_t size) {
	PatternSpec *ps = &spec_DATA[last_spec++];
	ps->name = name;
	ps->size = size;
	return ps;
}

void init() { 
	makeBasePatternSpec(INT, 4);
	
	Symbol pointChildren[2] = { X, Y };
	makePatternSpec(POINT, 2, pointChildren);
	
	Symbol lineChildren[2] = { A, B };
	makePatternSpec(LINE, 2, lineChildren);	
}

#endif


