/**
 * @ingroup stream
 *
 * @{
 *
 * @file stream.c
 * @brief stream abstraction implementation
 *
 * @copyright Copyright (C) 2013-2016, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 */

#include "stream.h"
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#include "process.h"
#include "debug.h"
#include "util.h"


void __st_realloc_reader(Stream *st) {
    st->buf_size *= 2;
    debug(D_STREAM,"realloc buffer to %ld\n",st->buf_size);
    st->buf = realloc(st->buf,st->buf_size+1);  //add 1 for a null
}

/**
 * load file data into the stream buffer
 *
 * returns the number of bytes loaded
 * if 0 bytes loaded, st->err will hold the reason, which is either because
 * and EndOfFile condition, or a some other unix error (from errno)
 *
 * if the buffer is already full when this function is called, it will
 * attempt to increase the buffer size with a realloc before reading.
 */
size_t __st_unix_stream_load(Stream *st) {
    FILE *stream = st->data.unix_stream;
    if (feof(stream)) {
        debug(D_STREAM,"eof before load\n");
        st->err = EOF;
        return 0;
    }
    if (__st_buf_full(st)) {
        __st_realloc_reader(st);
    }
    size_t max = st->buf_size-st->bytes_used;
    char *buf = st->buf+st->bytes_used;
    st->err = 0;

    size_t l = fread(buf,1,max,stream);
    debug(D_STREAM,"loaded %ld bytes from file: %.*s\n",l,(int)l,buf);
    if (l == 0) {
        st->err = errno;
    }
    else {
        st->bytes_used += l;
    }
    return l;
}


/**
 * load socket data into the stream buffer
 *
 * returns the number of bytes loaded
 * if 0 bytes loaded, st->err will hold the reason, which is either because
 * socket error condition, or a some other unix error (from errno)
 *
 * if the buffer is already full when this function is called, it will
 * attempt to increase the buffer size with a realloc before reading.
 */
size_t __st_socket_stream_load(Stream *st) {

    int sockfd = st->data.socket_stream;

    if (__st_buf_full(st)) {
        __st_realloc_reader(st);
    }
    size_t max = st->buf_size-st->bytes_used;
    char *buf = st->buf+st->bytes_used;
    st->err = 0;

    size_t l = recv(sockfd, buf, max, 0);
    debug(D_STREAM,"loaded %ld bytes from socket: %.*s\n",l,(int)l,buf);
    if (l == -1) {
        l = 0;
        st->err = errno;
    }
    else {
        st->bytes_used += l;
    }
    return l;
}

/**
 * scan a stream's buffer for a unit
 *
 * currently a unit simply is defined by a new-line char
 * @todo allow other delimiters to mark off a unit.
 */
void __st_scan(Stream *st) {
    if (st->scan_state == StreamScanInitial)
        st->unit_start = 0;
    else if (st->scan_state == StreamScanSuccess)
        st->unit_start +=  st->unit_size + 1;
    if (st->unit_start >= st->bytes_used) {
        st->scan_state = StreamScanComplete;
        return;
    }

    char c;
    size_t i=  (st->scan_state == StreamScanPartial) ? st->partial : st->unit_start;

    while (i<st->bytes_used) {
        if (st->buf[i] == '\n') {
            st->scan_state = StreamScanSuccess;
            st->unit_size = i - st->unit_start;
            return;
        }
        i++;
    }
    st->partial = i;
    st->scan_state = StreamScanPartial;
}


char *ss2str(s) {
    switch(s) {
    case StreamScanSuccess:return "StreamScanSuccess";
    case StreamScanPartial:return "StreamScanPartial";
    case StreamScanComplete:return "StreamScanComplete";
    case StreamScanInitial:return "StreamScanInitial";
    }
}


/**
 * reads a unit from a stream
 *
 * calls the stream loading function according to stream type when needed to
 * get stream data into the read buffer, and then scans along that buffer
 * setting the StreamHasData flag for each unit found.
 * If the scan returns Partial (i.e. no terminator found yet) and then stream
 * is terminated (indicate by 0 value from the load function) then the partial
 * will be treated as a successfully read unit.
 */
void __st_stream_read(Stream *st) {
    size_t l;

    if (st->bytes_used > 0) goto scan;
    // woot!  a good use case for gotos!
 init:
    __st_init_scan(st);
 load:
    if (st->type == UnixStream) {
        l = __st_unix_stream_load(st);
    }
    else if (st->type == SocketStream) {
        l = __st_socket_stream_load(st);
    }
    else raise_error("unknown stream type");

    if (l==0) {
        debug(D_STREAM,"load return zero, clearing alive bit. scan state: %s\n",ss2str(st->scan_state));
        if (st->scan_state == StreamScanPartial) {
            // treat as successful line
            st->scan_state = StreamScanComplete;
            st->flags |= StreamHasData;
        }
        else if (st->scan_state == StreamScanInitial)
            st->scan_state = StreamScanComplete;

        st->flags &= ~StreamAlive;
        return;
    }
 scan:
    __st_scan(st);
    debug(D_STREAM,"scanned with state: %s\n",ss2str(st->scan_state));
    if (st->scan_state == StreamScanSuccess) {
        debug(D_STREAM,"scan value: %.*s\n",(int)_st_data_size(st),_st_data(st));
        st->flags |= StreamHasData;
        return;
    }
    else if (st->scan_state == StreamScanPartial) {
        goto load;
    }
    else if (st->scan_state == StreamScanComplete) {
        goto init;
    }
    raise_error("unknown scan state!");
}


// stream reading thread function
void *_st_stream_read(void *arg) {
    Stream *st = (Stream *) arg;
    do {
        debug(D_STREAM,"wating for read.\n");
        pthread_mutex_lock(&st->mutex);
        st->flags |= StreamAlive; // don't change the state until the mutex is locked
        st->flags |= StreamWaiting;
        pthread_cond_wait(&st->cv, &st->mutex);
        st->flags &= ~StreamWaiting;

        if (!(st->flags & StreamHasData) && _st_is_alive(st)) {
            debug(D_STREAM,"starting read.\n");
            // this call is expected to block until a unit can be read and will
            // result in StreamHasData being set when it returns
            __st_stream_read(st);
        }
        else {
            if (st->flags & StreamHasData) debug(D_STREAM,"hmmm, stream already has data on read wakeup!\n");
            if (!(st->flags & StreamAlive)) debug(D_STREAM,"hmmm, stream dead on read wakeup!\n");
        }
        pthread_mutex_unlock(&st->mutex);
        if (st->callback) {
            (st->callback)(st);
        }
    } while _st_is_alive(st);
    pthread_exit(NULL);
}

// lo level stream allocator function
Stream *__st_alloc_stream() {
    Stream *s = malloc(sizeof(Stream));
    memset(s,0,sizeof(Stream));
    s->flags = StreamCloseOnFree;
    return s;
}

/**
 * set up a stream as a reader by allocating a read buffer
 * creating the reader thread, and setting up the conditional
 * mutex that gets called to activate reading in that thread
 */
void __st_start_reader(Stream *s,size_t reader_buffer_size) {
    s->flags |= StreamReader;

    s->buf = malloc(reader_buffer_size+1);
    s->buf_size = reader_buffer_size;
    pthread_mutex_init(&(s->mutex), NULL);
    pthread_cond_init(&(s->cv), NULL);

    int rc = pthread_create(&s->pthread,0,_st_stream_read,s);
    if (rc){
        raise_error("Error starting reader thread; return code from pthread_create() is %d\n", rc);
    }

    // wait for the the reader thread to come alive and block on mutex wait
    while(!(s->flags & StreamWaiting)) {
        sleepns(1);
    };
}

/**
 * create a new stream object of the unix file flavor
 *
 * if the the stream is a read stream you must pass in a starting buffer size
 */
Stream *__st_new_unix_stream(FILE *stream,size_t reader_buffer_size) {
    Stream *s =  __st_alloc_stream();
    s->type = UnixStream;
    s->data.unix_stream = stream;

    if (reader_buffer_size) {
        __st_start_reader(s,reader_buffer_size);
    }
    return s;
}

/**
 * create a new stream object of the unix file flavor
 */
Stream *_st_new_socket_stream(int sockfd) {
    Stream *s =  __st_alloc_stream();
    s->type = SocketStream;
    s->data.socket_stream = sockfd;

    __st_start_reader(s, DEFAULT_READER_BUFFER_SIZE);

    return s;
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void *__st_socket_stream_accept(void *arg) {
    SocketListener *l = (SocketListener *) arg;
    socklen_t sin_size;
    struct sockaddr_storage their_addr; // connector's address information
    int new_fd;
    char s[INET6_ADDRSTRLEN];

    do {
        sin_size = sizeof their_addr;
        debug(D_SOCKET,"listener on %d: starting accept\n",l->port);
        new_fd = accept(l->sockfd, (struct sockaddr *)&their_addr, &sin_size);
        debug(D_SOCKET,"listener on %d: got accept with fd %d\n",l->port,new_fd);
        if (new_fd == -1) {
            debug(D_SOCKET,"accept err %d\n",errno);
            break;
        }

        if (debugging(D_SOCKET)) {
            inet_ntop(their_addr.ss_family,
                      get_in_addr((struct sockaddr *)&their_addr),
                      s, sizeof s);
            debug(D_SOCKET,"listener on %d: got connection from %s\n",l->port, s);
        }

        Stream *st = _st_new_socket_stream(new_fd);
        (l->callback)(st,l->callback_arg);

    } while(l->alive);
    pthread_exit(NULL);
}

#define BACKLOG 10
/**
 * create a socket listener on a port which will generate socket streams when connections arrive
 */
SocketListener *_st_new_socket_listener(int port,lisenterConnectionCallbackFn fn,void *callback_arg) {
    char portstr[255];
    sprintf(portstr,"%d",port);
    SocketListener *l = malloc(sizeof(SocketListener));
    l->port = port;
    l->callback = fn;
    l->callback_arg = callback_arg;
    l->alive = true;

    int sockfd;  // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    int yes=1;
    char s[INET6_ADDRSTRLEN];
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, portstr, &hints, &servinfo)) != 0) {
        raise_error("getaddrinfo error: %s\n", gai_strerror(rv));
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            debug(D_SOCKET,"server: socket err %d",errno);
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int)) == -1) {
            raise_error("setsockopt %d",errno);
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            debug(D_SOCKET,"server: bind err %d",errno);
            continue;
        }

        break;
    }
    l->sockfd = sockfd;

    freeaddrinfo(servinfo); // all done with this structure

    if (p == NULL)  {
        raise_error("server: failed to bind %d\n",errno);
    }

    if (listen(sockfd, BACKLOG) == -1) {
        raise_error("listen error %d\n",errno);
    }
    debug(D_SOCKET,"starting listener thread on %d\n",port);
    int rc = pthread_create(&l->pthread,0,__st_socket_stream_accept,l);
    if (rc){
        raise_error("Error starting listener thread; return code from pthread_create() is %d\n", rc);
    }
    pthread_detach(l->pthread);
    if (rc){
        raise_error("Error detaching listener thread; return code from pthread_detach() is %d\n", rc);
    }
    return l;
}

/**
 *close a socket listener
 */
void _st_close_listener(SocketListener *l) {
    l->alive = false;
    shutdown(l->sockfd,SHUT_RDWR);
    //@todo this is a cheap way to signal the listener accept thread to die, we need to do something better
    sleepms(1);
    free(l);
}

/**
 * wake the stream reader thread
*/
void _st_start_read(Stream *st) {

    // don't throw the error if the stream is being killed
    // @todo figure out why this didn't work by testing the value of StreamAlive which is
    // also cleared in _st_kill at the same time StreamDying is set.
    if ((st->flags & StreamHasData) && !(st->flags & StreamDying)) {raise_error("stream data hasn't been consumed!");}
    debug(D_STREAM,"waking stream reader\n");
    pthread_mutex_lock(&st->mutex);
    pthread_cond_signal(&st->cv);
    pthread_mutex_unlock(&st->mutex);
}

/**
 * mark that data in a stream has been consumed
 */
void _st_data_consumed(Stream *st) {
    debug(D_STREAM,"marking data as read\n");
    st->flags &= ~StreamHasData;
}

/**
 * kill a stream
 *
 * if a reader stream is blocked and waiting, calls  _st_start_read so that
 * the reader thread can complete.
 */
void _st_kill(Stream *st) {
    if (st->flags & StreamDying) {
        debug(D_STREAM,"already dying\n");
        return;
    }

    st->flags &= ~StreamAlive;
    st->flags |= StreamDying;
    if (st->type == SocketStream) {
        debug(D_SOCKET,"shutting down socket in st_kill\n");
        shutdown(st->data.socket_stream,SHUT_RDWR);
    }
    if (st->flags & StreamReader) {
        debug(D_STREAM,"shutting down reader in st_kill\n");
        st->scan_state = StreamScanComplete;
        if (st->flags & StreamWaiting) {
            _st_start_read(st);
            while(st->flags & StreamWaiting) {sleepms(1);};
        }
    }
}

/**
 * release the resources of a stream object
 *
 */
void _st_free(Stream *st) {

    if (st->flags & StreamCloseOnFree) {
        debug(D_STREAM,"cleaning up stream\n");

        if (st->type == UnixStream)
            fclose(st->data.unix_stream);
        else if (st->type == SocketStream) {
            close(st->data.socket_stream);
        }
        //else raise_error("unknown stream type:%d\n",st->type);
    }
    _st_kill(st);
    //@todo who should clean up the mutexes??
    if (st->flags & StreamReader) {
        debug(D_STREAM,"cleaning up reader\n");
        free(st->buf);
        pthread_mutex_destroy(&st->mutex);
        pthread_cond_destroy(&st->cv);
    }
    free(st);
}

/**
 * write to a stream
 *
 */
int _st_write(Stream *st,char *buf,size_t len) {
    int bytes_written;
    if (st->type == UnixStream) {
        FILE *stream = st->data.unix_stream;
        bytes_written = fwrite(buf,sizeof(char),len,stream);
        if (bytes_written > 0) {
            fflush(stream);
        }
    }
    else if (st->type == SocketStream) {
        bytes_written = send(st->data.socket_stream,buf,len,MSG_NOSIGNAL);
    }
    else raise_error("unknown stream type:%d\n",st->type);
    debug(D_STREAM,"write of '%s' results in %d\n",buf,bytes_written);
    if (st->flags & StreamCloseAfterOneWrite) {
        _st_kill(st);
    }
    return bytes_written;
}
