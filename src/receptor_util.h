#include "ceptr.h"


bool logChange(Receptor *r) {
    return r->data.log_head != r->data.log_tail;
}

Signal *signal_new(Receptor *r,Address from, Address to,Symbol noun,void *surface){
    return _signal_new(from,to,time(NULL),noun,surface,size_of_named_surface(r,noun,surface));
}
