#include "../src/ceptr.h"

void testReceptorUtil(){
    Receptor tr; Receptor *r = &tr; init(r);

    //**** logChange
 /*   spec_is_true(!logChange(r));
    data_write_log(NULL, r, CSPEC_NOUN, r, 3);  // what's being written doesn't work
    spec_is_true(logChange(r));
 */

    //****  signal_new
    time_t t = time(NULL);
    Signal *s = signal_new(r,1,2,CSTRING_NOUN,"fish");
    spec_is_str_equal(&s->surface,"fish");
    spec_is_equal(s->timestamp,t);
    free(s);
}