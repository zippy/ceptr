#include "../src/ceptr.h"



void testNewSignal() {
    Address from = {VM,0};
    Address to = {VM,1};
    Signal *s = _signal_new(from,to,10234,CSTRING_NOUN,"hello",6);
    SignalHeader h = _s_header(s);
    spec_is_equal(h.from.aspect,0);
    spec_is_equal(h.to.aspect,1);
    spec_is_equal((int)h.timestamp,10234);
    spec_is_equal(s->noun,CSTRING_NOUN);
    spec_is_str_equal(&s->surface,"hello");
    free(s);
}

void testNewConversation() {
    Conversation *c = conversation_new(0);
    spec_is_equal(conversation_signals(c),0);
    conversation_delete(c);
}

void testAppendConversation() {
    Address from = {VM,0};
    Address to = {VM,1};
    Signal *s = _signal_new(from,to,10234,CSTRING_NOUN,"hello",6);
    Conversation *c = conversation_new(0);
    int id1 = conversation_append(c,99,s);
    spec_is_equal(id1,1);
    spec_is_equal(conversation_signals(c),1);
    Signal *s1 = _signal_new(to,from,10235,CSTRING_NOUN,"bye",4);
    int id = conversation_append(c,99,s1);
    spec_is_equal(id,2);
    spec_is_equal(conversation_signals(c),2);
    SignalEntry *se = conversation_get_signalentry(c,id);
    spec_is_equal(se->k,99);

    Signal *sr = conversation_get_signal(c,id);
    SignalHeader h = _s_header(sr);
    spec_is_equal(h.from.addr,VM);
    spec_is_equal(h.to.aspect,0);
    spec_is_equal((int)h.timestamp,10235);
    spec_is_equal(sr->noun,CSTRING_NOUN);
    spec_is_str_equal(&sr->surface,"bye");
    conversation_delete(c);
}

void testConversation() {
    testNewConversation();
    testAppendConversation();
}
