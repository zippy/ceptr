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
        printf("{ %d, %zu }", process[i].name, (size_t)process[i].function);
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

void test_op_new_noun() {
    Receptor tr;
    init(&tr);
    Receptor *r = &tr;
    dump_xaddrs(r);
}

int main(int argc, const char **argv) {
    test_op_new_noun();
}
