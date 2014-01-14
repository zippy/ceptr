#include "ceptr.h"


#define REPS_GET_NOUN(reps) (((RepsBody *)SKIP_ELEM_HEADER(reps))->noun)
#define REPS_SET_NOUN(reps,n) (((RepsBody *)SKIP_ELEM_HEADER(reps))->noun = n)


int util_xaddr_eq(Xaddr x1, Xaddr x2) {
    return x1.key == x2.key && x1.noun == x2.noun;
}

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

