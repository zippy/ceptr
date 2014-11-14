#ifndef _CEPTR_ERROR_H
#define _CEPTR_ERROR_H
#include <stdio.h>
#include <signal.h>
#include <assert.h>
#include <setjmp.h>
jmp_buf G_err;

#define die(x) raise(x)
//#define die(x) longjmp(G_err,x)

#define raise_error0(error_msg)			\
    {puts(error_msg);				\
    puts("\n");				\
    die(SIGINT);}

#define raise_error(error_msg, val)		\
    {printf(error_msg, val);			\
    puts("\n");				\
    die(SIGINT);}
#define raise_error2(error_msg, val1,val2)	\
    {printf(error_msg, val1,val2);		\
    puts("\n");				\
    die(SIGINT);}

#endif
