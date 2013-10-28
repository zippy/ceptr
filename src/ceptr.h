#define starting_size 100

enum patternName {
	BIT,
	INT,
	FLOAT	
};

enum nounName {
	BIT0,BIT1,BIT2,BIT3,BIT4,BIT5,BIT6,BIT7,BIT8,BIT9,BIT10,BIT11,BIT12,BIT13,BIT14,BIT15,BIT16,BIT17,BIT18,BIT19,BIT20,BIT21,BIT22,BIT23,BIT24,BIT25,BIT26,BIT27,BIT28,BIT29,BIT30,BIT31
};	

typedef struct {
	int nounName;
	int patternName;
	int offset;
} ElementOffset;


typedef struct {
	int patternName;
	ElementOffset children[starting_size];
} PatternSpec;


PatternSpec specs[starting_size] = {
	{
		BIT, {{ 0, 0, 0}}
	},
	{
		INT, 
		{
		  {BIT0, BIT, 0},{BIT1 , BIT,  1  },{BIT2 , BIT,  2  },{BIT3 , BIT,  3  },{BIT4 , BIT,  4  },{BIT5 , BIT,  5  },{BIT6 , BIT,  6  },{BIT7 , BIT,  7  },{BIT8 , BIT,  8  },{BIT9 , BIT,  9  },{BIT10, BIT,  10 },{BIT11, BIT,  11 },{BIT12, BIT,  12 },{BIT13, BIT,  13 },{BIT14, BIT,  14 },{BIT15, BIT,  15 },{BIT16, BIT,  16 },{BIT17, BIT,  17 },{BIT18, BIT,  18 },{BIT19, BIT,  19 },{BIT20, BIT,  20 },{BIT21, BIT,  21 },{BIT22, BIT,  22 },{BIT23, BIT,  23 },{BIT24, BIT,  24 },{BIT25, BIT,  25 },{BIT26, BIT,  26 },{BIT27, BIT,  27 },{BIT28, BIT,  28 },{BIT29, BIT,  29 },{BIT30, BIT,  30 },{BIT31, BIT,  31 }
	    }
	}
};

ElementOffset e = {BIT0, BIT, 0};




/*
	[{}]
	

*/
