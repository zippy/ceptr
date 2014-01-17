#ifndef _CEPTR_H
#define _CEPTR_H

#define DEFAULT_CACHE_SIZE 100000
#define STACK_SIZE 10000
#define OPERANDS_SIZE (sizeof(Xaddr) * 2)

#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include <pthread.h>
#include <stdlib.h>

#define raise_error0(error_msg)        \
    printf(error_msg);            \
    printf("\n"); \
    raise(SIGINT);
#define raise_error(error_msg, val)        \
    printf(error_msg, val);            \
    printf("\n"); \
    raise(SIGINT);
#define raise_error2(error_msg, val1,val2)        \
    printf(error_msg, val1,val2);                \
    printf("\n"); \
    raise(SIGINT);

typedef int Symbol;

#define BUFFER_SIZE 10000
#define SMALL_BUFFER_SIZE 500

typedef enum { false, true } bool;

enum FunctionNames {
    INSTANCE_SIZE, INSTANCE_NEW, PRINT, INC, ADD
};

enum Symbols {
    CSPEC = -1, CSPEC_NOUN = -2, XADDR_NOUN = -3,
    CSTRING_NOUN = -4, PATTERN_SPEC_DATA_NOUN = -5, MEMBRANE = -6,  ROOT = -100
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

typedef struct {
    Symbol noun;
    size_t size;
} SemStackFrame;

typedef struct {
    Symbol noun;
    char content[SMALL_BUFFER_SIZE];
} LogEntry;

typedef struct {
    Xaddr xaddrs[DEFAULT_CACHE_SIZE];
    Symbol xaddr_scape[DEFAULT_CACHE_SIZE];
    char cache[DEFAULT_CACHE_SIZE];
    int current_xaddr;
    LogEntry lastLogEntry;
    size_t cache_index;
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

typedef struct Receptor;

typedef void (* LogProc)(struct Receptor *);

typedef struct {
    SemStackFrame semStack[STACK_SIZE];
    Symbol nounStack[STACK_SIZE];
    int semStackPointer;
    char valStack[DEFAULT_CACHE_SIZE];
    int valStackPointer;
    ElementSurface rootSurface;

    //built in xaddrs:
    Xaddr rootXaddr;
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

    struct Receptor *parent;

    LogProc logProc;
    LogProc pollProc;
    Data data;

    int listenerCount;
    struct Receptor *listeners[10];

    Symbol charIntNoun;
    bool alive;
} Receptor;


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
#include "builtins/receptor.h"

//
#include "init.h"

//
#include "builtins/vm_host.h"

//
#include "dump.h"

#endif
