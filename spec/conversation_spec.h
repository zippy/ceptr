#include "../src/ceptr.h"



void testNewConversation() {
    Signal *s = _signal_new(1,2,10234,CSTRING_NOUN,"hello",6);
    Conversation *c = conversation_new(s);
    spec_is_equal(conversation_signals(c),1);
    spec_is_equal(c->signals[0]->from,1);
    spec_is_equal(c->signals[0]->to,2);
    spec_is_equal(c->signals[0]->timestamp,10234);
    spec_is_long_equal(c->signals[0]->noun,CSTRING_NOUN);
    spec_is_str_equal(&c->signals[0]->surface,"hello");
    conversation_delete(c);
}

void testAppendConversation() {
    Signal *s = _signal_new(1,2,10234,CSTRING_NOUN,"hello",6);
    Conversation *c = conversation_new(s);
    Signal *s1 = _signal_new(2,1,10235,CSTRING_NOUN,"bye",4);
    int id = conversation_append(c,s1);
    spec_is_equal(id,1);
    spec_is_equal(conversation_signals(c),2);
    spec_is_equal(c->signals[id]->from,2);
    spec_is_equal(c->signals[id]->to,1);
    spec_is_equal(c->signals[id]->timestamp,10235);
    spec_is_long_equal(c->signals[id]->noun,CSTRING_NOUN);
    spec_is_str_equal(&c->signals[id]->surface,"bye");
    conversation_delete(c);
}

void testConversation() {
    testNewConversation();
    testAppendConversation();
}
