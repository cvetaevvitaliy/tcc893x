
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/poll.h>
#include <unistd.h>

#define LOG_TAG "MessageQueue"
#include <utils/Log.h>

#include "MessageQueue.h"

MessageQueue::MessageQueue()
{
    int fds[2] = {-1,-1};

    pipe(fds);

    this->fd_read = fds[0];
    this->fd_write = fds[1];
}

MessageQueue::~MessageQueue()
{
    close(this->fd_read);
    close(this->fd_write);
}

int MessageQueue::get(Message* msg)
{
    char* p = (char*) msg;
    size_t read_bytes = 0;

    while( read_bytes  < sizeof(*msg) )
    {
        int bytes = read(this->fd_read, p+read_bytes, sizeof(*msg) - read_bytes);
        
        if( bytes < 0 ) {
            ALOGE("read() error: %s, (fd(%d), read_size(%d - %d)", strerror(errno), this->fd_read, sizeof(*msg), read_bytes);
            return -1;
        }
        else
            read_bytes += bytes;
    }

    return 0;
}

int MessageQueue::put(Message* msg)
{
    char* p = (char*) msg;
    size_t write_bytes = 0;

    while( write_bytes  < sizeof(*msg) )
    {
        int bytes = write(this->fd_write, p+write_bytes, sizeof(*msg) - write_bytes);
    	 
        if( bytes < 0 ) {
            ALOGE("write() error: %s, (fd(%d), write_size(%d - %d)", strerror(errno), this->fd_write, sizeof(*msg), write_bytes);
            return -1;
        }
        else
            write_bytes += bytes;
    }

    return 0;    
}


bool MessageQueue::isEmpty()
{
    struct pollfd pfd;

    pfd.fd = this->fd_read;
    pfd.events = POLLIN;
    pfd.revents = 0;

    if( -1 == poll(&pfd,1,0) )
    {
        ALOGE("poll() error: %s", strerror(errno));
    }

    return (pfd.revents & POLLIN) == 0;
}
