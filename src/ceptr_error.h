#ifndef _CEPTR_ERROR_H
#define _CEPTR_ERROR_H
#include <stdio.h>
#include <signal.h>
#include <assert.h>
#include <setjmp.h>
jmp_buf G_err;

#define die(x) raise(x)
//#define die(x) longjmp(G_err,x)

#define raise_error(...) \
    {fprintf(stderr,__VA_ARGS__);        \
     fprintf(stderr,"\n");                \
     die(SIGINT);}

#endif
