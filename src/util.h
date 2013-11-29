#include "ceptr.h"


#define REPS_GET_NOUN(reps) (((RepsBody *)SKIP_ELEM_HEADER(reps))->noun)
#define REPS_SET_NOUN(reps,n) (((RepsBody *)SKIP_ELEM_HEADER(reps))->noun = n)


int util_xaddr_eq(Xaddr x1, Xaddr x2) {
    return x1.key == x2.key && x1.noun == x2.noun;
}
