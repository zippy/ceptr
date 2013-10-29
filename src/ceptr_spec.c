#include "ceptr.h"
#include <stdio.h>
#include <signal.h>

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

void testExec(){
	Noun *x = getNoun(X);
	Xaddr theX =  { 12, x };
	int val = 3;
	op_set(theX, &val);
	op_exec(theX, INC);
	void *surface = op_get(theX);
	printf("after Exec: %d\n", *(int*)(surface));
}

int main(int argc, const char** argv)
{
	init();
	testPoint();
	testSemFault();
	testLine();	
	testExec();
}