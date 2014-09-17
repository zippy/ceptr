#include "ceptr.h"


Signal *signal_new(Receptor *r,Address from, Address to,Symbol noun,void *surface){
    return _signal_new(from,to,time(NULL),noun,surface,size_of_named_surface(r,noun,surface));
}

void receptor_free(Receptor *r) {
    _data_free(&r->data);
    stack_free(r);
}
