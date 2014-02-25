#ifndef _CEPTR_H
#define _CEPTR_H

#define OPERANDS_SIZE (sizeof(Xaddr) * 2)

#define _GNU_SOURCE
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include <pthread.h>
#include <stdlib.h>

#include "ceptr_error.h"
#include "tree.h"
#include "conversation.h"

#define BUFFER_SIZE 10000

typedef enum { false, true } bool;

enum FunctionNames {
    INSTANCE_SIZE, INSTANCE_NEW, PRINT, INC, ADD, RUN
};

enum Symbols {
    CSPEC = -1, CSPEC_NOUN = -2, XADDR_NOUN = -3,
    CSTRING_NOUN = -4, PATTERN_SPEC_DATA_NOUN = -5, MEMBRANE = -6
};

typedef int FunctionName;

typedef void (* voidVoidFn)(void *);


typedef struct {
    int key;
    Symbol noun;
} Xaddr;

typedef struct {
    Xaddr noun;
    int offset;
} Offset;

typedef struct {
    Xaddr specXaddr;
    char label;
} NounSurface;

#define SEMSTACK_FRAME_NOUN -107
typedef struct {
    Symbol noun;
    size_t size;
    int surface;
} SemStackFrame;

typedef int ConversationID;

#define LOG_META_NOUN -103
typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t changed;
} LogMeta;

typedef struct {
    Tnode *root;
    Tnode *log;
    Tnode *scapes;
    Tnode *nouns;
} Data;

typedef struct {
    FunctionName name;
    void (*function)(void *);
} UntypedProcess;

typedef struct {
    Symbol name;
    int process_count;
    UntypedProcess processes;
} ElementSurface;

typedef bool (* ScapeMatchFn)(void *match_surface,size_t match_len, void *key_surface, size_t key_len);
typedef int ScapeID;

typedef struct {
    Xaddr xaddr;
    size_t surface_len;
    char surface;
} ScapeItem;

typedef struct {
    Tnode *items;
    char name[255];
    Symbol data_source;
    Symbol key_source;
    Symbol key_geometry;
    ScapeMatchFn matchfn;
} Scape;

typedef struct Receptor Receptor;

typedef void (* SignalProc)(Receptor *);

struct Receptor {
    Tnode *sem_stack;

    //built in xaddrs:
    Xaddr cspecXaddr;
    Xaddr nounSpecXaddr;
    Xaddr patternSpecXaddr;
    Xaddr arraySpecXaddr;
    Xaddr intPatternSpecXaddr;
    Xaddr linePatternSpecXaddr;
    Xaddr streamSpecXaddr;
    Xaddr receptorSpecXaddr;
    Xaddr membraneXaddr;
    Xaddr xaddrXaddr;
    Xaddr str255PatternSpecXaddr;
    Xaddr cfuncPatternSpecXaddr;

    struct Receptor *parent;

    SignalProc signalProc;
    Data data;

    int listenerCount;
    struct Receptor *listeners[10];

    Symbol charIntNoun;
    bool alive;
    pthread_t thread;
};


typedef struct {
    FunctionName name;
    int (*function)(Receptor *);
} YagnArgsProcess;

typedef int (*processFn)(Receptor *, Symbol, void *, void *);

typedef struct {
    FunctionName name;
    processFn function;
} Process;

typedef struct {
    FunctionName name;
    int (*function)(Receptor *, void *);
} LegacyProcess;

typedef struct {
    char *label;
    int child_count;
    Xaddr *children;
    int process_count;
    Process *processes;
} PatternSpecData;

typedef struct {
    Symbol noun;
} RepsBody;

void dump_xaddrs(Receptor *r);

//
#include "util.h"

//
#include "scape.h"

//
#include "data.h"

//
#include "semtree.h"

//
#include "stack.h"

//
#include "preops.h"

//
#include "element.h"

//
#include "ops.h"

//
#include "builtins/noun.h"

//
#include "builtins/pattern.h"

//
#include "builtins/array.h"

//
#include "builtins/stream.h"

//
#include "builtins/int.h"

//
#include "builtins/str255.h"

//
#include "builtins/cfunc.h"

//
#include "builtins/receptor.h"

//
#include "init.h"

//
#include "receptor_util.h"

#include "builtins/command.h"

//
#include "vm.h"
#include "builtins/vm_host.h"

//
#include "dump.h"

#endif
