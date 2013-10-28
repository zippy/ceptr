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
	void *localHereSurface = get(hereHere);
	
	void *failSurface = get(thereHere);
	
	printf("failSurface %ld", (long)failSurface);
	printf("surface-X %d", *(int*)(localHereSurface));
	printf("surface-Y %d", *(int*)(localHereSurface + 4));
}