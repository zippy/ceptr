#ifndef _CEPTR_SYMBOL_H
#define _CEPTR_SYMBOL_H

#define TEST_SYMBOL 99
#define TEST_SYMBOL2 100

// System defined symbols.
// In a regex, there are two things match -- actual literal, and "." (any)
enum 
{
	SEMTREX_SYMBOL_LITERAL,				// This system symbol matches on the semantic type.
	SEMTREX_SEQUENCE,
	SEMTREX_OR,
	SEMTREX_SYMBOL_ANY,
	SEMTREX_STAR,
	SEMTREX_PLUS,
	SEMTREX_QUESTION,
	SEMTREX_GROUP,
	SEMTREX_MATCH_RESULTS,
	SEMTREX_MATCH,
	SEMTREX_MATCH_SIBLINGS_COUNT,
	SEMTREX_VALUE_LITERAL,				// Matches on the semantic type and the data value.
	TREE_PATH
};

typedef int Symbol;

#endif


//	Semantic Type:
//		Structures, like "float", are abstract labels.
//		A semantic type is a label with a particular semantic application or implementation of that labelled structure.
//		"int" is a labelled structure, "age" is a semantic implementation.
//	
//	Ex:
//		/TEST_SYMBOL="t"		: the root's semantic type is "semtrex_symbol_literal" and it's literal value = 't'







