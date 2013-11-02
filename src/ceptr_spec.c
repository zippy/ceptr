#include "ceptr.h"
#include <stdio.h>
#include <signal.h>
#include <assert.h>


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
    while (process[i].name != 0 || process[i].function != 0) {
        if (i != 0) {
            printf(",");
        }
        printf("{ %d, %d }", process[i].name, process[i].function);
        i++;
    }
}

Xaddr noun_to_xaddr(Symbol noun) {
    Xaddr nounXaddr = {noun, NOUN_SPEC};
    return nounXaddr;
}

char *noun_label(Receptor *r, Symbol noun) {
    NounSurface *ns = (NounSurface *) op_get(r, noun_to_xaddr(noun));
    return ns->label;
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
            printf("    size: %d\n", ps->size);
            printf("    children: ");
            dump_children_array(ps->children);
            printf("\n    processes: ");
            dump_process_array(ps->processes);
            printf("\n");
            break;
        case NOUN_SPEC:
            ns = (NounSurface *) &r->data.cache[key];
            printf("Noun \    { %d, %5d } %s", ns->namedElement.key, ns->namedElement.noun, ns->label);
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

void test_op_new_noun() {
    Receptor tr;
    init(&tr);
    Receptor *r = &tr;
    dump_xaddrs(r);
}

int main(int argc, const char **argv) {
    test_op_new_noun();
    // testPoint();
    //     testSemFault();
    //     testLine();
    //     testInc();
    //     testAdd();
    //     testSymbolPath();
    //     testRun();
}
