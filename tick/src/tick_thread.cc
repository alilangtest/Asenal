#include "tick_thread.h"
#include "mutexlock.h"
#include "port.h"
#include "tick_epoll.h"
#include "tick_define.h"
#include "csapp.h"
#include "tick_item.h"

TickThread::TickThread()
{
    log_info("TickThread init");
    thread_id_ = pthread_self();

    tickEpoll_ = new TickEpoll();

    int fds[2];
    if (pipe(fds)) {
        log_err("Can't create notify pipe");
    }
    notify_receive_fd_ = fds[0];
    notify_send_fd_ = fds[1];
    tickEpoll_->TickAddEvent(notify_receive_fd_, EPOLLIN | EPOLLERR | EPOLLHUP);
}

void TickThread::RunProcess()
{
    thread_id_ = pthread_self();
    struct timeval t = {1, 0};
    int nfds;
    TickFiredEvent *tfe;
    char buf[1024];
    int tot = 0;
    char bb[1];
    TickItem ti;
    for (;;) {
        // log_info("TickThread %lld in the poll", thread_id_);
        nfds = tickEpoll_->TickPoll(&t);
        tfe = tickEpoll_->firedevent();
        // log_info("TickThread get nfds %d", nfds);
        for (int i = 0; i < nfds; i++) {
            read(notify_receive_fd_, bb, 1);
            ti = conn_queue_.front();
            conn_queue_.pop();

            int fd = ti.fd();
            // log_info("TickThread get fd %d", fd);
            if (tfe->mask_ & EPOLLIN) {
                ssize_t nread = 1;
                while (1) {
                    nread = read(fd, buf, MAXLINE);
                    // log_info("%d", nread);
                    // log_info("%s\n", buf);

                    if (nread == -1) {
                        if ((errno == EINTR)) {
                            // log_info("errno EINTR");
                            continue;
                        } else {
                            // log_info("errno other");
                            break;
                        }
                    } else if (nread == 0){
                        // log_info("nread equal 0");
                        close(fd);
                        break;
                    } else {
                        tot += nread;
                        log_info("tot %lld %d", thread_id_, tot);
                        break;
                    }
                }
            } else {
                log_info("error come");
            }
        }
    }
}
