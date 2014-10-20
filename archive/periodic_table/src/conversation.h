#ifndef _CONVERSATION_H
#define _CONVERSATION_H

#include <time.h>
#include "tree.h"

typedef int ReceptorAddress;
typedef int Aspect;
typedef struct {
    ReceptorAddress addr;
    Aspect aspect;
} Address;

typedef struct {
    Address from;
    Address to;
    time_t timestamp;
} SignalHeader;

typedef struct {
    SignalHeader header;
    Symbol noun;
    char surface;
} Signal;

typedef int SignalKey;

typedef struct {
    SignalKey k;
    Signal *s;
} SignalEntry;

typedef int ConversationStatus;

enum {CSTAT_NEW=-99, CSTAT_PENDING};

typedef struct {
    ConversationStatus status;
} ConversationMeta;


typedef T Conversation;

Signal *_signal_new(Address from, Address to,time_t time,Symbol noun,void *surface, size_t size){
    size_t total_size = size + sizeof(Signal);
    Signal *s = malloc(total_size);
    if (s != NULL) {
	s->header.from = from;
	s->header.to = to;
	s->header.timestamp = time;
	s->noun = noun;
	memcpy(&s->surface,surface,size);
    }
    return s;
}

SignalHeader _s_header(Signal *s) {
    return s->header;
}

int conversation_append(Conversation *c,SignalKey k,Signal *s) {
    SignalEntry se = {k,s};
    T *t = _t_new(c,SIGNAL_ENTRY_NOUN,&se,sizeof(SignalEntry));
    return _t_children(c);
}

Conversation *conversation_new(T *log) {
    ConversationMeta cm = {CSTAT_NEW};
    Conversation *c = _t_new(log,CONVERSATION_META_NOUN,&cm,sizeof(ConversationMeta));
    return c;
}

int conversation_delete(Conversation *c){
    _t_free(c);
    return 0;
}

SignalEntry *conversation_get_signalentry(Conversation *c,int signal_id) {
    return _t_get_child_surface(c,signal_id);
}

Signal *conversation_get_signal(Conversation *c,int signal_id) {
    SignalEntry *se = conversation_get_signalentry(c,signal_id);
    return se->s;
}

int conversation_signals(Conversation *c) {
    return _t_children(c);
}

ConversationStatus conversation_status(Conversation *c) {
    return ((ConversationMeta *)_t_surface(c))->status;
}
#endif
