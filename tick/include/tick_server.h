#ifndef __TICK_H__
#define __TICK_H__

#include <stdio.h>
#include <sys/epoll.h>
#include <stdlib.h>
#include <fcntl.h>
#include <event.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <sys/epoll.h>

#include "csapp.h"
#include "xdebug.h"
#include "tick_define.h"


class TickThread;
class TickEpoll;

class TickServer
{

public:
    TickServer();
    ~TickServer();

    void RunProcess();

    static void* StartThread(void* arg);

private:
    int sockfd_;
    int port_;
    struct sockaddr_in servaddr_;

    TickEpoll *tickEpoll_;

    int last_thread_;
    TickThread *tickThread_[TICK_THREAD_NUM];

    // No copying allowed
    TickServer(const TickServer&);
    void operator=(const TickServer&);

};

class tickConf
{
    int thread_num;
};

#endif
