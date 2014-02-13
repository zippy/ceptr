#include "../src/ceptr.h"

void testReceptorUtil(){
    Receptor tr; Receptor *r = &tr; init(r);

    //****  signal_new
    time_t t = time(NULL);
    Address from = {VM,0};
    Address to = {VM,0};
    Signal *s = signal_new(r,from,to,CSTRING_NOUN,"fish");
    spec_is_str_equal(&s->surface,"fish");
    spec_is_equal((int)s->timestamp,(int)t);
    free(s);
}
