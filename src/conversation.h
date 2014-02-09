#ifndef _CONVERSATION_H
#define _CONVERSATION_H

#define SIGNAL_BUFFER_SIZE 500
#define CONVERSATION_MAX_SIGNALS 5

typedef long TimeStamp;

typedef int Address;
typedef int Symbol;

typedef struct {
    Address from;
    Address to;
    TimeStamp timestamp;
    Symbol noun;
    char surface[SIGNAL_BUFFER_SIZE];
} Signal;

typedef struct {
    unsigned int signal_count;
    Signal signals[CONVERSATION_MAX_SIGNALS];
} Conversation;

Conversation *conversation_new(Signal *s) {
    Conversation *c = malloc(sizeof(Conversation));
    c->signal_count = 1;
    memcpy(&c->signals[0],s,sizeof(Signal));
    return c;
}

int conversation_append(Conversation *c,Signal *s) {
    memcpy(&c->signals[c->signal_count],s,sizeof(Signal));
    return c->signal_count++;
}

int conversation_delete(Conversation *c){
    free(c);
    return 0;
}

int conversation_signals(Conversation *c) {
    return c->signal_count;
}
#endif
