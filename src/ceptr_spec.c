#include "ceptr.h"
#include <stdio.h>
#include <signal.h>

int main(int argc, const char** argv)
{
	init();
	Noun *here = newNoun("here", POINT);
	
	Xaddr hereHere = { 10, here };
	Xaddr thereHere = { 12, here };
	
	// Point p = { x: 22, y: 43 };

	int myPoint[2] = { 2,10 };
	set(hereHere, &myPoint);
	void *surface = get(hereHere);
	
	void *failSurface = get(thereHere);
	
	printf("failSurface %ld\n", (long)failSurface);
	printf("point-X %d", *(int*)(surface));
	printf("point-Y %d\n", *(int*)(surface + 4));
	
	int myLine[4] = {1,2,3,4};
	Noun *inTheSand = newNoun("in the sand", LINE);
	Xaddr itsLine = { 20, inTheSand };
	set(itsLine, myLine);
	surface = get(itsLine);
	
	printf("line-A (%d,%d) B (%d, %d) \n", 
		*(int*)(surface),
		*(int*)(surface + 4),
		*(int*)(surface + 8),
		*(int*)(surface + 12)
		);
}