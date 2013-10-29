#include "ceptr.h"
#include <stdio.h>
#include <signal.h>
#include <assert.h>

void testPoint(){
	Noun *here = newNoun("here", POINT);
	Xaddr hereHere = { 10, here };
	int myPoint[2] = { 2,10 };
	op_set(hereHere, &myPoint);
	void *surface = op_get(hereHere);

	printf("point-X %d", *(int*)(surface));
	printf("point-Y %d\n", *(int*)(surface + 4));
}


void testSemFault(){
	Noun *here = newNoun("here", POINT);
	Xaddr thereHere = { 12, here };
	void *failSurface = op_get(thereHere);
	printf("failSurface %ld\n", (long)failSurface);
}

void testLine(){
	int myLine[4] = {1,2,3,4};
	Noun *inTheSand = newNoun("in the sand", LINE);
	Xaddr itsLine = { 20, inTheSand };
	op_set(itsLine, myLine);

	// op_exec()

	void *surface = op_get(itsLine);

	printf("line-A (%d,%d) B (%d, %d) \n",
		*(int*)(surface),
		*(int*)(surface + 4),
		*(int*)(surface + 8),
		*(int*)(surface + 12)
		);
}

void testInc(){
	Noun *x = getNoun(X);
	Xaddr theX =  { 12, x };
	int val = 3;
	op_set(theX, &val);
	op_exec(theX, INC);
	void *surface = op_get(theX);
	printf("after Inc: %d\n", *(int*)(surface));
	assert(*(int*)(surface) == 4);
}

void testAdd(){
	Noun *x = getNoun(X);
	Xaddr theX =  { 12, x };
	int val = 3;
	op_set(theX, &val);
	op_push_pattern(INT, &val);
	op_exec(theX, ADD);

	printf("stack: %d\n", *(int*)(&valStack[0]));
	assert(*(int*)(&valStack[0]) == 6);
}


void testSymbolPath(){
	int myLine[4] = {1,2,3,4};
	Noun *inTheSand = newNoun("in the sand", LINE);
	Xaddr itsLine = { 4, inTheSand };
	op_set(itsLine, myLine);
	Symbol path[3] = {B,Y,TERMINATOR};
	int *val;
	int seven = 7;
	val = op_getpath(itsLine, path);
	assert(*val == 4);

	op_setpath(itsLine, path, &seven);
	val = op_getpath(itsLine, path);
	assert(*val == 7);

	void *surface = op_get(itsLine);
	printf("after Pathset: %d\n", *(((int*)surface)+3) );
	assert(*(((int*)surface)+3) == 7);
}

int main(int argc, const char** argv)
{
	init();
	testPoint();
	testSemFault();
	testLine();
	testInc();
	testAdd();
	testSymbolPath();
}
