#include "ceptr.h"
#include <stdio.h>
#include <signal.h>
#include <assert.h>

#define MAX_FAILURES 1000
int spec_failures = 0;
int spec_total = 0;
char failures[MAX_FAILURES][255];

void hexDump(char *desc, void *addr, int len) {
    int i;
    unsigned char buff[17];
    unsigned char *pc = addr;

    // Output description if given.
    if (desc != NULL)
        printf("%s:\n", desc);

    // Process every byte in the data.
    for (i = 0; i < len; i++) {
        // Multiple of 16 means new line (with line offset).

        if ((i % 16) == 0) {
            // Just don't print ASCII for the zeroth line.
            if (i != 0)
                printf("  %s\n", buff);

            // Output the offset.
            printf("  %04x ", i);
        }

        // Now the hex code for the specific character.
        printf(" %02x", pc[i]);

        // And store a printable ASCII character for later.
        if ((pc[i] < 0x20) || (pc[i] > 0x7e))
            buff[i % 16] = '.';
        else
            buff[i % 16] = pc[i];
        buff[(i % 16) + 1] = '\0';
    }
}

void dump_children_array(Offset *children) {
    int i = 0;
    while (children[i].noun.key != 0 || children[i].noun.noun != 0) {
        if (i != 0) {
            printf(",");
        }
        printf("{ %d, %d }(%d)", children[i].noun.key, children[i].noun.noun, children[i].offset);
        i++;
    }
}


void dump_process_array(Process *process) {
    int i = 0;
    while(process[i].name != 0 || process[i].function != 0) {
	if (i!=0){
	    printf(",");
	}
        printf("{ %d, %zu }", process[i].name, (size_t)process[i].function);
	i++;
    }
}

void dump_xaddr(Receptor *r, Xaddr xaddr, int indent_level) {
    int i;
    PatternSpec *ps;
    NounSurface *ns;
    Process *print_proc;
    void *surface;
    int key = xaddr.key;
    int noun = xaddr.noun;
    switch (noun) {
        case PATTERN_SPEC:
            ps = (PatternSpec *) &r->data.cache[key];
            printf("Pattern Spec\n");
            printf("    name: %s(%d)\n", noun_label(r, ps->name), ps->name);
            printf("    size: %d\n", (int)ps->size);
            printf("    children: ");
            dump_children_array(ps->children);
            printf("\n    processes: ");
            dump_process_array(ps->processes);
            printf("\n");
            break;
        case NOUN_SPEC:
            ns = (NounSurface *) &r->data.cache[key];
            printf("Noun      { %d, %5d } %s", ns->namedElement.key, ns->namedElement.noun, ns->label);
            break;
        default:
            surface = op_get(r, noun_to_xaddr(noun));
            ns = (NounSurface *) surface;
            printf("%s : ", ns->label);

            //FIXME: this breaks when named elements can be other than patterns
            ps = (PatternSpec *) op_get(r, ns->namedElement);
            print_proc = getProcess(ps, PRINT);
            if (print_proc) {
                (*print_proc->function)(r, xaddr);
            } else {
                hexDump("hexDump of surface", &r->data.cache[key], ps->size);
            }
    }
}

void dump_xaddrs(Receptor *r) {
    int i;
    PatternSpec *ps;
    NounSurface *ns;
    void *surface;
    for (i = 0; i <= r->data.current_xaddr; i++) {
        printf("Xaddr { %5d, %5d } - ", r->data.xaddrs[i].key, r->data.xaddrs[i].noun);
        dump_xaddr(r, r->data.xaddrs[i], 0);
        printf("\n");
    }
}

#define spec_is_true(x) spec_total++;if (x){putchar('.');} else {putchar('F');sprintf(failures[spec_failures++],"%s:%d expected %s to be true",__FUNCTION__,__LINE__,#x);}

void testInt() {
    Receptor tr;init(&tr);Receptor *r = &tr;
    Symbol MY_INT = op_new_noun(r, r->intPatternSpecXaddr, "MY_INT");
    int val = 7;
    Xaddr my_int_xaddr = op_new(r, MY_INT, &val);
    int *v = op_get(r,my_int_xaddr);
    spec_is_true(*v == 7);
    val = 8;
    op_set(r,my_int_xaddr,&val);
    v = op_get(r,my_int_xaddr);
    spec_is_true(*v == 8);
}

 void testPoint(){
     Receptor tr;init(&tr);Receptor *r = &tr;

     Symbol HERE = op_new_noun(r, r->pointPatternSpecXaddr, "HERE");
     int value[2] = {777, 422};
     Xaddr here_xaddr = op_new(r, HERE, &value);
     int *v = op_get(r,here_xaddr);
     spec_is_true(*v==777 && *(v+1) == 422);
 }

void testInc(){
    Receptor tr;init(&tr);Receptor *r = &tr;
    Symbol AGE = op_new_noun(r, r->intPatternSpecXaddr, "Age");
    int val = 7;
    Xaddr age_xaddr = op_new(r, AGE, &val);
    op_exec(r,age_xaddr,INC);
    int *v = op_get(r,age_xaddr);
    spec_is_true(*v == 8);
}

void testAdd(){
    Receptor tr;init(&tr);Receptor *r = &tr;
    Symbol AGE = op_new_noun(r, r->intPatternSpecXaddr, "Age");
    int val = 7;
    Xaddr age_xaddr = op_new(r, AGE, &val);
    val = 3;
    op_push_pattern(r,r->intPatternSpecXaddr.key,&val);
    op_exec(r,age_xaddr, ADD);
    spec_is_true(*(int*)(&r->valStack[0]) == 10);
}

// void testSemFault(){
//     Receptor tr;init(&tr);Receptor *r = &tr;
//     Noun *here = newNoun(r,"here", POINT);
//     Xaddr thereHere = { 12, here };
//     void *failSurface = op_get(r,thereHere);
//     printf("failSurface %ld\n", (long)failSurface);
// }
//

void testLine(){
    Receptor tr;init(&tr);Receptor *r = &tr;
    int myLine[4] = {1,2,3,4};
    Symbol inTheSand = op_new_noun(r,r->linePatternSpecXaddr,"in the sand");
    Xaddr itsLine = op_new(r,inTheSand,&myLine);

    void *surface = op_get(r,itsLine);
    spec_is_true(
	   *(int*)(surface) == 1 &&
	   *(int*)(surface + 4) == 2 &&
	   *(int*)(surface + 8) == 3 &&
	   *(int*)(surface + 12) == 4
	   );
}

void testSymbolPath(){
    Receptor tr; init(&tr); Receptor *r = &tr;
    int myLine[4] = {1,2,3,4};
    Symbol inTheSand = op_new_noun(r,r->linePatternSpecXaddr,"in the sand");
    Xaddr itsLine = op_new(r,inTheSand,&myLine);
    Symbol B = getSymbol(r,"B");
    Symbol Y = getSymbol(r,"Y");
    Symbol path[3] = {B,Y,SYMBOL_PATH_TERMINATOR};
    int *val;
    int seven = 7;

    val = op_getpath(r,itsLine, path);
    spec_is_true(*val == 4);

    op_setpath(r,itsLine, path, &seven);
    val = op_getpath(r,itsLine, path);
    spec_is_true(*val == 7);

    void *surface = op_get(r,itsLine);
    spec_is_true(*(((int*)surface)+3) == 7);
}



void testRun(){
    Receptor tr;init(&tr);Receptor *r = &tr;
    int values[1] = { 31415 };
    ImmediatePatternOperand i = { r->intPatternSpecXaddr.key, 0 };
    Instruction pushProgram[2];
    pushProgram[0].opcode = PUSH_IMMEDIATE;
    memcpy(&pushProgram[0].operands, &i, OPERANDS_SIZE);
    pushProgram[1].opcode = RETURN;
    int topOfStack = r->valStackPointer;
    spec_is_true(r->valStackPointer == 0);
    spec_is_true(r->semStackPointer == -1);
    run(r,pushProgram, values);
    spec_is_true(r->valStackPointer == 4);
    spec_is_true(r->semStackPointer == 0);
    spec_is_true(*(int*)(&r->valStack[topOfStack])== 31415 );
}

void test_op_new_noun(){
    Receptor tr;init(&tr);Receptor *r = &tr;
    dump_xaddrs(r);
}

int main(int argc, const char** argv)
{
    printf("Running all tests...\n\n");
    test_op_new_noun();
    testInt();
    testPoint();
    testInc();
    testAdd();
    //     testSemFault();
    testLine();
    testSymbolPath();
    testRun();
    int i;
    if (spec_failures > 0) {
	printf("\n%d out of %d specs failed:\n",spec_total,spec_failures);
	for(i = 0; i < spec_failures; i++) {
	    printf("%s\n",failures[i]);
	}
    }
    else {
	printf("\nAll %d specs pass\n",spec_total);
    }

    return 0;
}
