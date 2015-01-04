#include "tick_thread.h"
#include "mutexlock.h"
#include "port.h"
#include "tick_epoll.h"
#include "tick_define.h"
#include "csapp.h"
#include "tick_item.h"
#include "tick_conf.h"
#include <glog/logging.h>

extern TickConf* g_tickConf;

TickThread::TickThread()
{
    thread_id_ = pthread_self();

    /*
     * inital the tickepoll object, add the notify_receive_fd to epoll
     */
    tickEpoll_ = new TickEpoll();
    int fds[2];
    if (pipe(fds)) {
        LOG(FATAL) << "Can't create notify pipe";
    }
    notify_receive_fd_ = fds[0];
    notify_send_fd_ = fds[1];
    tickEpoll_->TickAddEvent(notify_receive_fd_, EPOLLIN | EPOLLERR | EPOLLHUP);

    /*
     * initial the qbus client
     */
    qbus_cluster_ = g_tickConf->qbus_cluster();
    qbus_conf_path_ = g_tickConf->qbus_conf_path();
    qbus_topic_ = g_tickConf->qbus_topic();
    producer_ = KafkaProducer::getInstance(qbus_cluster_, qbus_conf_path_, false);
    if (producer_ == NULL) {
        LOG(FATAL) << "KafkaProducer getInstance error";
    }

    producer_->setSendTimeout(5);
    producer_->setRecvTimeout(2);
    producer_->setConnMax(4);

    std::string errstr("");
    std::vector<std::string> msg(1, "czz_heiheiho");
    bool ret = producer_->send(msg, qbus_topic_, errstr, KafkaConstDef::MESSAGE_RANDOM_SEND);
    log_info("%d", ret);
}

TickThread::~TickThread()
{
    producer_ = NULL;
    delete(tickEpoll_);
    mutex_.Unlock();
    close(notify_send_fd_);
    close(notify_receive_fd_);
}

void TickThread::RunProcess()
{
    thread_id_ = pthread_self();
    int nfds;
    TickFiredEvent *tfe;
    char buf[1024];
    int tot = 0;
    char bb[1];
    TickItem ti;
    for (;;) {
        nfds = tickEpoll_->TickPoll();
        tfe = tickEpoll_->firedevent();
        for (int i = 0; i < nfds; i++) {
            read(notify_receive_fd_, bb, 1);
            {
            MutexLock l(&mutex_);
            ti = conn_queue_.front();
            conn_queue_.pop();
            }

            int fd = ti.fd();
            // log_info("TickThread get fd %d", fd);
            if (tfe->mask_ & EPOLLIN) {
                ssize_t nread = 1;
                while (1) {
                    nread = read(fd, buf, MAXLINE);

                    if (nread == -1) {
                        if ((errno == EINTR)) {
                            continue;
                        } else {
                            break;
                        }
                    } else if (nread == 0){
                        close(fd);
                        break;
                    } else {
                        tot += nread;
                        // log_info("tot %lld %d", thread_id_, tot);
                        break;
                    }
                }
            } else {
                log_info("error come");
            }
        }
    }
}
