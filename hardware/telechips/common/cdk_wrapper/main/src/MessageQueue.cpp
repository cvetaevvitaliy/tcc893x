
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/poll.h>
#include <unistd.h>

#define LOG_TAG "MessageQueue"
#include <utils/Log.h>

#include "MessageQueue.h"

MessageQueue::MessageQueue()
	: m_send_count(0), m_recv_count(0)
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

int MessageQueue::put(Message* msg)
{
    char* p = (char*) msg;
    size_t bytes = 0;

    while( bytes  < sizeof(msg) )
    {
        int err = write(this->fd_write, p, sizeof(*msg) - bytes);
        
        if( err < 0 ) {
            ALOGE("write() error: %s", strerror(errno));
            return -1;
        }
        else
            bytes += err;
    }

    return 0;
}

int MessageQueue::get(Message* msg)
{
    char* p = (char*) msg;
    size_t read_bytes = 0;

    while( read_bytes  < sizeof(msg) )
    {
        int err = read(this->fd_read, p, sizeof(*msg) - read_bytes);
    	 
        if( err < 0 ) {
            ALOGE("read() error: %s", strerror(errno));
            return -1;
        }
        else
            read_bytes += err;
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

int MessageQueue::Send(uint32_t cmd, uint32_t param1, uint32_t param2, uint32_t param3, uint32_t param4)
{
	Message ReqMsg;

	ReqMsg.command = cmd;
	ReqMsg.arg1 = param1;
	ReqMsg.arg2 = param2;
	ReqMsg.arg3 = param3;
	ReqMsg.arg4 = param4;

	return this->put(&ReqMsg);
}

int MessageQueue::Send(Message* msg)
{
	m_send_count++;
	return this->put(msg);
}

int MessageQueue::Receive(Message* msg)
{
	int ret = this->get(msg);
	m_recv_count++;
	return ret;
}
