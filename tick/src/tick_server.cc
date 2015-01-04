#include "tick_server.h"
#include "tick_define.h"
#include "tick_util.h"
#include "tick_epoll.h"
#include "tick_item.h"
#include "tick_thread.h"
#include "tick_conf.h"
#include "mutexlock.h"

extern TickConf *g_tickConf;

TickServer::TickServer()
{
    // init sock
    sockfd_ = socket(AF_INET, SOCK_DGRAM, 0);

    memset(&servaddr_, 0, sizeof(servaddr_));

    port_ = g_tickConf->port();
    servaddr_.sin_family = AF_INET;
    servaddr_.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr_.sin_port = htons(port_);

    bind(sockfd_, (struct sockaddr *) &servaddr_, sizeof(servaddr_));
    setnonblocking(sockfd_);

    // init tick epoll
    tickEpoll_ = new TickEpoll();
    tickEpoll_->TickAddEvent(sockfd_, EPOLLIN | EPOLLERR | EPOLLHUP);

    last_thread_ = 0;
    for (int i = 0; i < TICK_THREAD_NUM; i++) {
        tickThread_[i] = new TickThread();
    }
}

TickServer::~TickServer()
{
    for (int i = 0; i < TICK_THREAD_NUM; i++) {
        delete(tickThread_[i]);
    }
    delete(tickEpoll_);
    close(sockfd_);
}

void* TickServer::StartThread(void* arg)
{
    reinterpret_cast<TickThread*>(arg)->RunProcess();
    return NULL;
}

void TickServer::RunProcess()
{
    // start the tickThread_ thread
    for (int i = 0; i < TICK_THREAD_NUM; i++) {
        pthread_create(&(tickThread_[i]->thread_id_), NULL, &(TickServer::StartThread), tickThread_[i]);
    }
    int nfds;
    TickFiredEvent *tfe;
    for (;;) {
        nfds = tickEpoll_->TickPoll();
        tfe = tickEpoll_->firedevent();
        // log_info("TickServer get nfds %d\n", nfds);
        for (int i = 0; i < nfds; i++) {
             int fd = tfe->fd_;
             if (tfe->mask_ & EPOLLIN) {
                 std::queue<TickItem> *q = &(tickThread_[last_thread_]->conn_queue_);
                 TickItem ti(fd);
                 log_info("TickItem %d\n", last_thread_);
                 {
                 MutexLock l(&tickThread_[last_thread_]->mutex_);
                 q->push(ti);
                 }
                 write(tickThread_[last_thread_]->notify_send_fd(), "", 1);
                 last_thread_++;
                 last_thread_ %= TICK_THREAD_NUM;
             }
        }
    }
}
