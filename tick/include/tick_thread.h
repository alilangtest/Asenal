#ifndef __TICK_THREAD_H__
#define __TICK_THREAD_H__

#include "xdebug.h"

#include <pthread.h>
#include <sys/epoll.h>
#include <queue>
#include "port.h"
#include "tick_item.h"
#include "Qbus.h"


class TickItem;
class TickEpoll;

class TickThread
{
public:
    TickThread();
    ~TickThread();

    void RunProcess();

    std::queue<TickItem> conn_queue_;

    int notify_receive_fd() { return notify_receive_fd_; }
    int notify_send_fd() { return notify_send_fd_; }

    pthread_t thread_id_;

    // port::Mutex mutex() { return mutex_; }

private:

    friend class TickServer;
    int notify_receive_fd_;
    int notify_send_fd_;

    TickEpoll *tickEpoll_;

    const char *qbus_cluster_;
    const char *qbus_conf_path_;
    std::string qbus_topic_;
    KafkaProducer *producer_;
    port::Mutex mutex_;

    // No copy || assigned operator allowed
    TickThread(const TickThread&);
    void operator=(const TickThread &);
};

#endif
