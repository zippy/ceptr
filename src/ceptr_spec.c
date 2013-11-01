#include "ceptr.h"
#include <stdio.h>
#include <signal.h>
#include <assert.h>

 
void dump_children_array(Offset* children){
	int i = 0;
	while(children[i].noun.key != 0 || children[i].noun.noun != 0) {
		if (i!=0){
			printf(",");
		}
		printf("{ %d, %d }(%d)", children[i].noun.key, children[i].noun.noun, children[i].offset);
		i++;
	}
}


void dump_process_array(Process* process){
	int i = 0;
	while(process[i].name != 0 || process[i].function != 0) {
		if (i!=0){
			printf(",");
		}
		printf("{ %d, %d }", process[i].name, process[i].function);
		i++;
	}
}

void xaddr_dump(Receptor* r) {
	int i;
	PatternSpec* ps;
	NounSurface* ns;
	for (i=0; i<=r->data.current_xaddr; i++){
		int key = r->data.xaddrs[i].key;
		int noun = r->data.xaddrs[i].noun;
		printf("Xaddr { %d, %d } : ", key, noun);
		switch(noun){
			case PATTERN_SPEC:
				ps = (PatternSpec*)&r->data.cache[key];
				printf("Pattern Spec \n");
				printf("    name: %d\n", ps->name);
				printf("    size: %d\n", ps->size);
				printf("    children: ");
				dump_children_array(ps->children);
				printf("\n    processes: ");
				dump_process_array(ps->processes);
				printf("\n");
				break;
			case NOUN_SPEC:
				ns = (NounSurface*)&r->data.cache[key];
				printf("Noun \n    { %d, %d } %s\n" , ns->namedElement.key, ns->namedElement.noun, ns->label);
		}
		printf("\n");
	}
}

// 
// void testPoint(){
//     Receptor tr;init(&tr);Receptor *r = &tr;
//     Noun *here = newNoun(r,"here", POINT);
//     Xaddr hereHere = { 10, here };
//     int myPoint[2] = { 2,10 };
//     op_set(r,hereHere, &myPoint);
//     void *surface = op_get(r,hereHere);
// 
//     printf("point-X %d", *(int*)(surface));
//     printf("point-Y %d\n", *(int*)(surface + 4));
// }
// 
// void testSemFault(){
//     Receptor tr;init(&tr);Receptor *r = &tr;
//     Noun *here = newNoun(r,"here", POINT);
//     Xaddr thereHere = { 12, here };
//     void *failSurface = op_get(r,thereHere);
//     printf("failSurface %ld\n", (long)failSurface);
// }
// 
// void testLine(){
//     Receptor tr;init(&tr);Receptor *r = &tr;
//     int myLine[4] = {1,2,3,4};
//     Noun *inTheSand = newNoun(r,"in the sand", LINE);
//     Xaddr itsLine = { 20, inTheSand };
//     op_set(r,itsLine, myLine);
// 
//     // op_exec()
// 
//     void *surface = op_get(r,itsLine);
// 
//     printf("line-A (%d,%d) B (%d, %d) \n",
// 	   *(int*)(surface),
// 	   *(int*)(surface + 4),
// 	   *(int*)(surface + 8),
// 	   *(int*)(surface + 12)
// 	   );
// }
// 
// void testInc(){
//     Receptor tr;init(&tr);Receptor *r = &tr;
//     Noun *x = getNoun(r,X);
//     Xaddr theX =  { 12, x };
//     int val = 3;
//     op_set(r,theX, &val);
//     op_exec(r,theX, INC);
//     void *surface = op_get(r,theX);
//     printf("after Inc: %d\n", *(int*)(surface));
//     assert(*(int*)(surface) == 4);
// }
// 
// void testAdd(){
//     Receptor tr;init(&tr);Receptor *r = &tr;
//     Noun *x = getNoun(r,X);
//     Xaddr theX =  { 12, x };
//     int val = 3;
//     op_set(r,theX, &val);
//     op_push_pattern(r,INT, &val);
//     op_exec(r,theX, ADD);
// 
//     printf("stack: %d\n", *(int*)(&r->valStack[0]));
//     assert(*(int*)(&r->valStack[0]) == 6);
// }
// 
// void testSymbolPath(){
//     Receptor tr; init(&tr); Receptor *r = &tr;
//     int myLine[4] = {1,2,3,4};
//     Noun *inTheSand = newNoun(r,"in the sand", LINE);
//     Xaddr itsLine = { 4, inTheSand };
//     op_set(r,itsLine, myLine);
//     Symbol path[3] = {B,Y,TERMINATOR};
//     int *val;
//     int seven = 7;
//     val = op_getpath(r,itsLine, path);
//     assert(*val == 4);
// 
//     op_setpath(r,itsLine, path, &seven);
//     val = op_getpath(r,itsLine, path);
//     assert(*val == 7);
// 
//     void *surface = op_get(r,itsLine);
//     printf("after Pathset: %d\n", *(((int*)surface)+3) );
//     assert(*(((int*)surface)+3) == 7);
// }
// 
// void testRun(){
//     Receptor tr;init(&tr);Receptor *r = &tr;
//     int values[1] = { 22 };
//     ImmediatePatternOperand i = { INT, 0 };
//     Instruction pushProgram[2];
//     pushProgram[0].opcode = PUSH_IMMEDIATE;
//     memcpy(&pushProgram[0].operands, &i, OPERANDS_SIZE);
//     pushProgram[1].opcode = RETURN;
//     int topOfStack = r->valStackPointer;
//     printf("stack before run: %d\n", *(int*)(&r->valStack[r->valStackPointer]));
//     run(r,pushProgram, values);
//     printf("stack after run: %d\n", *(int*)(&r->valStack[topOfStack]));
// }

void test_op_new_noun(){
    Receptor tr;init(&tr);Receptor *r = &tr;
	xaddr_dump(r);
}

int main(int argc, const char** argv)
{
	test_op_new_noun();
    // testPoint();
    //     testSemFault();
    //     testLine();
    //     testInc();
    //     testAdd();
    //     testSymbolPath();
    //     testRun();
}
