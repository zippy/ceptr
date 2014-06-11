#ifndef _CEPTR_ERROR_H
#define _CEPTR_ERROR_H

#define raise_error0(error_msg)			\
    puts(error_msg);				\
    puts("\n");				\
    raise(SIGINT);
#define raise_error(error_msg, val)		\
    printf(error_msg, val);			\
    puts("\n");				\
    raise(SIGINT);
#define raise_error2(error_msg, val1,val2)	\
    printf(error_msg, val1,val2);		\
    puts("\n");				\
    raise(SIGINT);

#endif
