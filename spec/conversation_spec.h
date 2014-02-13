#include "../src/ceptr.h"



void testNewConversation() {
    Address from = {VM,0};
    Address to = {VM,1};
    Signal *s = _signal_new(from,to,10234,CSTRING_NOUN,"hello",6);
    Conversation *c = conversation_new(77,s);
    spec_is_equal(conversation_signals(c),1);
    spec_is_equal(c->signals[0]->from.aspect,0);
    spec_is_equal(c->signals[0]->to.aspect,1);
    spec_is_equal((int)c->signals[0]->timestamp,10234);
    spec_is_equal(c->signals[0]->noun,CSTRING_NOUN);
    spec_is_str_equal(&c->signals[0]->surface,"hello");
    conversation_delete(c);
}

void testAppendConversation() {
    Address from = {VM,0};
    Address to = {VM,1};
    Signal *s = _signal_new(from,to,10234,CSTRING_NOUN,"hello",6);
    Conversation *c = conversation_new(77,s);
    Signal *s1 = _signal_new(to,from,10235,CSTRING_NOUN,"bye",4);
    int id = conversation_append(c,99,s1);
    spec_is_equal(id,1);
    spec_is_equal(conversation_signals(c),2);
    spec_is_equal(c->keys[id],99);
    spec_is_equal(c->signals[id]->from.addr,VM);
    spec_is_equal(c->signals[id]->to.aspect,0);
    spec_is_equal((int)c->signals[id]->timestamp,10235);
    spec_is_equal(c->signals[id]->noun,CSTRING_NOUN);
    spec_is_str_equal(&c->signals[id]->surface,"bye");
    conversation_delete(c);
}

void testConversation() {
    testNewConversation();
    testAppendConversation();
}
