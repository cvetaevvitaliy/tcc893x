#ifndef __MESSAGEQUEUE_H__
#define __MESSAGEQUEUE_H__

struct Message
{
    uint32_t command;
    uint32_t arg1;
    uint32_t arg2;
    uint32_t arg3;
    uint32_t arg4;
};

class MessageQueue
{
public:
    enum UsageType { REQ, RESP };

    MessageQueue();
    ~MessageQueue();
    bool isEmpty();

    int32_t Send(uint32_t cmd, uint32_t param1 = 0, uint32_t param2 = 0, uint32_t param3 = 0, uint32_t param4 = 0);
    int32_t Send(Message* msg);
    int32_t Receive(Message* msg);

    inline int32_t Size() const { 
	return (m_send_count - m_recv_count);
};

private:
    int32_t put(Message*);
    int32_t get(Message*);

    int32_t fd_read;
    int32_t fd_write;

    uint32_t 	m_send_count;
    uint32_t    m_recv_count;
};
#endif // __MESSAGEQUEUE_H__
