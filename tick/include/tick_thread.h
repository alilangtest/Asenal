#ifndef __TICK_THREAD_H__
#define __TICK_THREAD_H__

#include "xdebug.h"

#include <pthread.h>
#include <sys/epoll.h>
#include <queue>
#include "port.h"
#include "tick_item.h"


class TickItem;
class TickEpoll;

class TickThread
{
public:
    TickThread();

    void RunProcess();

    std::queue<TickItem> conn_queue() { return conn_queue_; }
    std::queue<TickItem> conn_queue_;

    int notify_receive_fd() { return notify_receive_fd_; }
    int notify_send_fd() { return notify_send_fd_; }

    pthread_t thread_id_;

private:

    int notify_receive_fd_;
    int notify_send_fd_;


    TickEpoll *tickEpoll_;
    port::Mutex mutex_;
};

#endif
