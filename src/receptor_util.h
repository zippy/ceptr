#include "ceptr.h"

bool logChange(Receptor *r) {
    return r->data.log_head != r->data.log_tail;
}

