/**
 * @file util.c
 * @copyright Copyright (C) 2013-2016, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 */
#include "util.h"
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <stdlib.h>
#include "ceptr_error.h"

void hexDump(char *desc, void *addr, int len) {
    int i;
    unsigned char buff[17];
    unsigned char *pc = addr;

    // Output description if given.
    if (desc != NULL)
        printf("%s:\n", desc);
    // Process every byte in the data.
    for (i = 0; i < len; i++) {
        // Multiple of 16 means new line (with line offset).

        if ((i % 16) == 0) {
            // Just don't print ASCII for the zeroth line.
            if (i != 0)
                printf("  %s\n", buff);

            // Output the offset.
            printf("  %04x ", i);
        }

        // Now the hex code for the specific character.
        printf(" %02x", pc[i]);

        // And store a printable ASCII character for later.
        if ((pc[i] < 0x20) || (pc[i] > 0x7e))
            buff[i % 16] = '.';
        else
            buff[i % 16] = pc[i];
        buff[(i % 16) + 1] = '\0';
    }
}

int strcicmp(char const *a, char const *b)
{
    for (;; a++, b++) {
        int d = tolower(*a) - tolower(*b);
        if (d != 0 || !*a)
            return d;
    }
}

void writeFile(char *fn,void *data,size_t size) {
    FILE *ofp;

    ofp = fopen(fn, "w");
    if (ofp == NULL) {
        fprintf(stderr, "Can't open output file %s!\n",fn);
    }
    else {
        fwrite(data, 1,size, ofp);
        fclose(ofp);
    }
}

void *readFile(char *fn,size_t *size) {
    off_t file_size;
    char *buffer;
    struct stat stbuf;
    int fd;

    fd = open(fn, O_RDONLY);
    if (fd == -1) {
        raise_error("unable to open: %s",fn);
    }

    if ((fstat(fd, &stbuf) != 0) || (!S_ISREG(stbuf.st_mode))) {
        close(fd);
        raise_error("not a regular file: %s",fn);
    }

    file_size = stbuf.st_size;
    if (size) *size = file_size;

    buffer = malloc(file_size);
    if (buffer == NULL) {
        close(fd);
        raise_error("unable to allocate enough memory for contents of: %s",fn);
    }
    ssize_t bytes_read = read(fd,buffer,file_size);
    if (bytes_read == -1) {
        close(fd);
        free(buffer);
        raise_error("error reading %s: %d",fn,errno);
    }
    close(fd);
    return buffer;
}

uint64_t diff_micro(struct timespec *start, struct timespec *end)
{
    /* us */
    return ((end->tv_sec * (1000000)) + (end->tv_nsec / 1000)) -
        ((start->tv_sec * 1000000) + (start->tv_nsec / 1000));
}

#define MS_PER_NANO_SECOND  1000000L  // 1 millisecond = 1,000,000 Nanoseconds

void sleepms(long milliseconds) {
    struct timespec ts;
    ts.tv_sec = milliseconds/1000;
    ts.tv_nsec = (milliseconds-(ts.tv_sec*1000))*MS_PER_NANO_SECOND;
    nanosleep(&ts,NULL);
}
