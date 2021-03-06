#include "../ceptr.h"
#include <unistd.h>

// receptor aspects
enum {NULL_ASPECT=-1,STDIN,STDOUT};

// receptor addresses
enum {_HOST=-2,VM = -1};

typedef struct {
    Receptor receptor;
    T *receptors;
    Xaddr cmdPatternSpecXaddr;
    Xaddr cmdDump;
    Xaddr cmdStop;
    Symbol host_command;
    Scape *command_scape;
    pthread_t stdin_thread;
    ConversationID std_out;
} HostReceptor;

void null_proc(Receptor *r) {
    printf("Got a log message.  I do nothing\n");
}

int conversations_active(Receptor *r) {
    return _t_children(r->data.log);
}

Conversation *get_conversation(Receptor *r,ConversationID id) {
    return _t_get_child(r->data.log,id);
}

ConversationID start_conversation(Receptor *r,SignalKey key, Signal *s)
{
    ConversationID i = 0;
    LogMeta *lm = _t_surface(r->data.log);
    assert( pthread_mutex_lock(&lm->mutex) == 0);
    Conversation *c = conversation_new(r->data.log);
    if (c) {
	conversation_append(c,key,s);
	assert( pthread_cond_broadcast( &lm->changed) == 0);
	i =  _t_children(r->data.log);
    }
    assert( pthread_mutex_unlock( &lm->mutex) == 0);
    return i;
}

void add_to_conversation(Receptor *r,ConversationID id,SignalKey key, Signal *s)
{
    Conversation *c = get_conversation(r,id);
    conversation_append(c,key,s);
}

Address host_a =  {_HOST,NULL_ASPECT};
Address vm_a = {VM,NULL_ASPECT};
Address stdout_a = {VM,STDOUT};
Address stdin_a = {VM,STDIN};

enum {RAW_COMMAND,STDOUT_LINE};

void cs_out(HostReceptor *h,char *s) {
    add_to_conversation((Receptor *)h,h->std_out,STDOUT_LINE,signal_new((Receptor *)h,vm_a,stdout_a,CSTRING_NOUN,s));
}

void * stdin_run_proc(void *v){
    HostReceptor *h = (HostReceptor *)v;
    ssize_t read = 0;
    char *line = NULL;
    char *welcome = "Welcome to the ceptr command line\n";
    char prompt[] = {'>',' ',0};
    size_t len = 0;

    h->std_out = start_conversation((Receptor *)h,STDOUT_LINE,signal_new((Receptor *)h,vm_a,stdout_a,CSTRING_NOUN,welcome));

    while(read != -1 && ((Receptor *)h)->alive) {
	cs_out((HostReceptor *)h,prompt);
	read = getline(&line,&len,stdin);
	if (read != -1) {
            start_conversation((Receptor *)h,RAW_COMMAND,signal_new((Receptor *)h,host_a,stdin_a,CSTRING_NOUN,line));
	}
    }
    printf("Stdin closing down\n");
    free(line);
}

void *receptor_task(void *arg) {
    Receptor *r = (Receptor *)arg;
    // r->initProc;
    r->alive = true;
    LogMeta *lm = _t_surface(r->data.log);
    while(r->alive) {
	assert( pthread_mutex_lock( &lm->mutex) == 0);
	assert( pthread_cond_wait(&lm->changed, &lm->mutex) == 0);
        if (r->alive && r->signalProc) {
            (r->signalProc)(r);
        }
	assert( pthread_mutex_unlock( &lm->mutex) == 0);
    }
    printf("receptor task complete \n");
    return NULL;
}
#define RECEPTOR_NOUN -998
Receptor *get_receptor(HostReceptor *h,int id) {
    return _t_get_child_surface(h->receptors,id);
}

int receptor_count(HostReceptor *h) {
    return _t_children(h->receptors);
}

void iter_receptors(HostReceptor *h,tIterSurfaceFn fn,void *param) {
    _t_iter_children_surface(h->receptors,fn,param);
}

void vmh_receptor_new(HostReceptor *h, SignalProc sp) {
    Receptor rb,*r;
    T *t = _t_new(h->receptors,RECEPTOR_NOUN,&rb,sizeof(Receptor));
    r = _t_surface(t);
    r->signalProc = sp;
    r->parent = (Receptor *)h;
    init(r);
    int rc;
    printf("creating thread\n");
    rc = pthread_create(&r->thread, NULL, receptor_task, (void *)r);
    assert(0 == rc);
}

void stopfunc(Receptor *r,int i,void *param) {
    r->alive = false;
}

int vm_host_cmd_stop(Receptor *r) {
    HostReceptor *h = (HostReceptor *)r;
    printf("Stopping all receptors...\n");
    iter_receptors(h,(tIterSurfaceFn)stopfunc,0);
    h->receptor.alive = false;
}

void dumpfunc(HostReceptor *r,int i,void *param) {
    char b[200];
    sprintf(b,"\n\n Receptor %d:\n",i);
    cs_out(r,b);
    dump_xaddrs((Receptor *)r);
}

int vm_host_cmd_dump(Receptor *r) {
    HostReceptor *h = (HostReceptor *)r;
    cs_out(h,"\n\n HOST Receptor:\n");
    dump_xaddrs(&h->receptor);

    char b[200];
    sprintf(b,"Receptor count: %d\n",receptor_count(h));
    cs_out(h,b);
    iter_receptors(h,(tIterSurfaceFn)dumpfunc,0);
    //    _d_dump_surface(r->data.root);
}

//TODO: clear out old conv signals to to be reprocessing them
void conv_changed_func(Conversation *c,int i, HostReceptor *r) {
    ConversationMeta *cm = _t_surface(c);
    if (cm->status == CSTAT_NEW) {
	int t = _t_children(c);
	for (i=1;i<=t;i++) {
	    SignalEntry *se = conversation_get_signalentry(c,i);
	    Signal *s = se->s;
	    SignalHeader h = _s_header(s);
	    //	    printf("got a signal: key:%d; from %d.%d; to %d.%d; noun %d; surface: %s \n",se->k,h.from.addr,h.from.aspect,h.to.addr,h.to.aspect,s->noun,&s->surface);
	    if (h.to.addr == VM) {
		if (h.to.aspect == STDOUT && se->k == STDOUT_LINE) {
		    printf("%s",(char *)&conversation_get_signal(c,i)->surface);
		    se->k = -1;
		}

		if (h.to.aspect == STDIN) {
		    cm->status = CSTAT_PENDING;
		    Xaddr c = _scape_lookup(r->command_scape,&s->surface,0);
		    //  Xaddr c = ((HostReceptor *)r)->cmdDump;
		    if ((c.noun == -1) && (c.key == -1)) {
			printf("Unable to make sense of: %s !\n",&s->surface);
		    }
		    else {
			printf("running cmd\n");
			op_invoke(&r->receptor,c,RUN);
		    }
		}
	    }
	}
    }
}

void vm_host_log_proc(Receptor *r) {
    _t_iter_children(r->data.log,(tIterFn)conv_changed_func,r);
}

bool first_word_match(void *match_surface,size_t match_len, void *key_surface, size_t key_len) {
    char *m = (char *)match_surface;
    char *k = (char *)key_surface;
    while(*k != 0) {
	if (*m == *k) {
	    m++;k++;
	}
	else return false;
    }
    return (*m==0 || *m == ' ' || *m==0xA);
}

#define RECEPTORS_ARRAY_NOUN -987
void vm_host_init(HostReceptor *r){
    r->receptors = _t_new_root(RECEPTORS_ARRAY_NOUN);
    init(&r->receptor);
    r->receptor.alive = true;
    r->receptor.signalProc = vm_host_log_proc;

    r->cmdPatternSpecXaddr = command_init((Receptor *)r);
    r->host_command = preop_new_noun((Receptor *)r, r->cmdPatternSpecXaddr, "Host Command");

    Data *d = &((Receptor *)r)->data;
    r->command_scape = get_scape(d,new_scape(d,"command_scape_name",r->host_command,getSymbol((Receptor *)r,"CMD_STR"),CSTRING_NOUN,first_word_match));

    r->cmdStop = make_command((Receptor *)r,r->host_command,"stop","s",vm_host_cmd_stop);
    r->cmdDump = make_command((Receptor *)r,r->host_command,"dump","d",vm_host_cmd_dump);

    int rc;
    printf("creating stdin thread \n");
    rc = pthread_create(&r->stdin_thread, NULL, stdin_run_proc, (void *) r);
    assert(0 == rc);
}

void vm_host_run(HostReceptor *h) {

    receptor_task(h);
   int rc;
   for (int i = 1; i <= receptor_count(h); i++) {
	Receptor *r = get_receptor(h,i);
        rc = pthread_join(r->thread, NULL);
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



typedef struct {
    Address destination;
    Xaddr value;
} Packet;


Receptor *resolve(HostReceptor *r, ReceptorAddress addr) {
    Receptor *dr = get_receptor(r,addr);
    if (dr == 0) {
        raise_error("whatchu talking about addred %d\n", addr );
        return 0;
    }
    return dr;
}

void _send_message(HostReceptor *r, Address destination, Symbol noun, void *surface, size_t size) {
    Receptor *dest_receptor = resolve(r, destination.addr);
    raise_error0("UNIMPLEMENTED\n");
    //    data_write_log(r, dest_receptor, noun, surface, size);
}

void send_message(Receptor *r, Packet *p) {
    void *surface = surface_for_xaddr(r, p->value);
    size_t size = size_of_named_surface(r, p->value.noun, surface);
    _send_message((HostReceptor *)r->parent, p->destination, p->value.noun, surface, size);
}

void listen(Receptor *r, ReceptorAddress addr) {
    HostReceptor *h = (HostReceptor *)r->parent;
    Receptor *l = resolve(h, addr);
    l->listeners[l->listenerCount++] = r;
}
