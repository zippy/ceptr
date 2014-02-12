#include "../ceptr.h"
#include <unistd.h>

#define MAX_RECEPTORS 100

// addresses
enum { STDOUT };

// receptor addresses
enum {VM};

typedef struct {
    Receptor receptor;
    Receptor *receptors[MAX_RECEPTORS];
    Xaddr cmdPatternSpecXaddr;
    Xaddr cmdDump;
    Xaddr cmdStop;
    Symbol host_command;
    int receptor_count;
    pthread_t stdin_thread;
    pthread_t threads[MAX_RECEPTORS];
} HostReceptor;

void null_proc(Receptor *r) {
    printf("Got a log message.  I do nothing\n");
}

Receptor *resolve(HostReceptor *r, Address addr) {
    Receptor *dr = r->receptors[addr];
    if (dr == 0) {
        raise_error("whatchu talking about addred %d\n", addr );
        return 0;
    }
    return dr;
}

void data_write_log(HostReceptor *h, Receptor *r, Symbol noun, void *surface, size_t length) {
    // HostReceptor h is here just to make it clear this isn't internal operation on r

    assert( pthread_mutex_lock( &r->data.log_mutex) == 0);

    r->data.log[r->data.log_head].noun = noun;
    memcpy( r->data.log[r->data.log_head].surface, surface, length);

    int new_log_head = r->data.log_head + 1;
    if (new_log_head >= MAX_LOG_ENTRIES) {
        new_log_head = 0;
    }
    if (new_log_head == r->data.log_tail) {
        raise_error0("buffer overflow");
    }
    r->data.log_head = new_log_head;

    assert( pthread_mutex_unlock( &r->data.log_mutex) == 0);

//    wakeup(r);
}

void write_out(HostReceptor *h, Receptor *r, Symbol noun, void *surface, size_t length) {
    for (int i=0; i < r->listenerCount; i++){
        data_write_log(h, r->listeners[i], noun, surface, length);
    }
}

ConversationID data_add_conversation(Receptor *r,Conversation *c) {
    ConversationID i;
    ConversationEntry *ce = malloc(sizeof(ConversationEntry));
    if (ce) {
	assert( pthread_mutex_lock( &r->data.log_mutex) == 0);
	ce->id =  r->data.conversations_active++;
	ce->c = c;
	ce->prev = r->data.conversations_last;
	r->data.conversations_last = ce;
	if (r->data.conversations_first ==NULL) {
	    r->data.conversations_first = ce;
	}

	assert( pthread_cond_broadcast( &r->data.log_changed_cv) == 0);

	assert( pthread_mutex_unlock( &r->data.log_mutex) == 0);
	return ce->id;
    }
    return -1;
}

int conversations_active(Receptor *r) {
    return r->data.conversations_active;
}

Conversation *get_conversation(Receptor *r,ConversationID id) {
    ConversationEntry *ce = r->data.conversations_last;
    while (ce != NULL && ce->id != id) {
	ce = ce->prev;
    }
    if (ce != NULL) return ce->c;
    return NULL;
}

ConversationID start_conversation(Receptor *r,SignalKey key, Signal *s)
{
    Conversation *c = conversation_new(s);
    ConversationID i = data_add_conversation(r,c);
    return i;
}

typedef struct {
    Address destination;
    Xaddr value;
} Packet;

void _send_message(HostReceptor *r, Address destination, Symbol noun, void *surface, size_t size) {
    Receptor *dest_receptor = resolve(r, destination);
    data_write_log(r, dest_receptor, noun, surface, size);
}

void send_message(Receptor *r, Packet *p) {
    void *surface = surface_for_xaddr(r, p->value);
    size_t size = size_of_named_surface(r, p->value.noun, surface);
    _send_message(r->parent, p->destination, p->value.noun, surface, size);
}

void listen(Receptor *r, Address addr) {
    HostReceptor *h = (HostReceptor *)r->parent;
    Receptor *l = resolve(h, addr);
    l->listeners[l->listenerCount++] = r;
}

void dump_named_surface(Receptor *r, Symbol noun, void *surface) {
    if (spec_noun_for_noun(r, noun) == r->patternSpecXaddr.noun) {
        dump_pattern_spec(r, surface);
    } else if (spec_noun_for_noun(r, noun) == r->intPatternSpecXaddr.noun) {
        proc_int_print(r, noun, surface);
    } else {
        dump_xaddrs(r);
        raise_error("dunno how to dump named surface %d\n", noun);
    }
}

enum {RAW_COMMAND};

void stdin_run_proc(HostReceptor *h){
    ssize_t read = 0;
    char *line = NULL;
    size_t len = 0;
    printf("Welcome to the ceptr command line\n");
    while(read != -1 && ((Receptor *)h)->alive) {
	printf("> ");
	read = getline(&line,&len,stdin);
	if (read != -1) {
            start_conversation(h,RAW_COMMAND,signal_new(h,VM,VM,CSTRING_NOUN,line));
	}
    }
    printf("Stdin closing down\n");
    free(line);
/*    //    printf("stdin_run_proc\n");
    int c;
    c = getchar();
    while(c != EOF && r->alive) {
	c = getchar();
	write_out((HostReceptor *)r->parent, r, r->charIntNoun, &c, 4);
    }*/
}

void stdout_log_proc(Receptor *r, Signal *s) {
    //    printf("stdout_log_proc in\n");
    dump_named_surface(r, s->noun, &s->surface );
    //    printf("stdout_log_proc out\n");
}


void *receptor_task(void *arg) {
    Receptor *r = (Receptor *)arg;
    // r->initProc;
    r->alive = true;
    while(r->alive) {
	assert( pthread_mutex_lock( &r->data.log_mutex) == 0);
	assert( pthread_cond_wait(&r->data.log_changed_cv, &r->data.log_mutex) == 0);
        if (r->signalProc) {
	    ConversationEntry *ce = r->data.conversations_last;
            Signal *s = ce->c->signals[0];
            (r->signalProc)(r, s);
        }
	assert( pthread_mutex_unlock( &r->data.log_mutex) == 0);
    }
    return NULL;
}
void _vmh_receptor_new(HostReceptor *h, Address addr, SignalProc sp) {
    Receptor *r = malloc(sizeof(Receptor));
    if (r == NULL) {raise_error0("couldn't allocate receptor\n");}
    h->receptors[addr] = r;
    init(r);
    r->signalProc = sp;
    r->parent = h;

    int rc;
    //printf("creating thread for addr %d\n", addr);

    rc = pthread_create(&h->threads[addr], NULL, receptor_task, (void *) h->receptors[addr]);
    assert(0 == rc);
}

Receptor *vmh_receptor_new(HostReceptor *r, SignalProc sp) {
    _vmh_receptor_new(r, ++r->receptor_count, sp);
    return r->receptors[r->receptor_count];
}

int vm_host_cmd_stop(HostReceptor *h) {
    printf("Stopping all receptors...\n");
    for (int i = 0; i <= h->receptor_count; i++) {
	h->receptors[i]->alive = false;
    }
}
int vm_host_cmd_dump(HostReceptor *h) {
    for (int i = 0; i <= h->receptor_count; i++) {
	printf("\n\n Receptor %d:\n",i);
	dump_xaddrs(h->receptors[i]);
    }
}

void vm_host_log_proc(Receptor *r, Signal *s) {
    if (strcmp(&s->surface,"quit")) {
	r->alive = false;
    }
    printf("got a signal: from %d; to %d; noun %d; surface: %s \n",s->from,s->to,s->noun,&s->surface);
}

void vm_host_init(HostReceptor *r){
    r->receptor_count = 0;
    init(&r->receptor);
    r->receptor.alive = true;
    r->receptor.signalProc = vm_host_log_proc;

    r->cmdPatternSpecXaddr = command_init((Receptor *)r);
    r->host_command = preop_new_noun(r, r->cmdPatternSpecXaddr, "Host Command");

    r->cmdStop = make_command(r,r->host_command,"stop","s",vm_host_cmd_stop);
    r->cmdDump = make_command(r,r->host_command,"dump","d",vm_host_cmd_dump);

    int rc;
    printf("creating stdin thread \n");
    rc = pthread_create(&r->stdin_thread, NULL, stdin_run_proc, (void *) r);
    assert(0 == rc);

    _vmh_receptor_new(r, STDOUT, stdout_log_proc);
    r->receptor_count = STDOUT;

}


void vm_host_run(HostReceptor *h) {

    receptor_task(h);
    int rc;
    for (int i = 1; i < h->receptor_count; i++) {
        rc = pthread_join(h->threads[i], NULL);
        assert(0 == rc);
    }
    rc = pthread_join(h->stdin_thread,NULL);
    assert(0 == rc);
    //
//    repl {
//        Code c;
//        Instruction *i = &c;
//        while(true) {
//            switch(i.op) {
//                case LISTEN:
//                case INVOKE:
//                case PUSH:
//                case POP:
//            }
//        }
//    }

}
