#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <event.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include "csapp.h"
#include "xdebug.h"


int setnonblocking(int sockfd)
{
    if (fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFD, 0)|O_NONBLOCK) == -1) {
        return -1;
    }
    return 0;
}


void str_echo(int fd)
{
    char buf[10240];
    rio_t rio;
    rio_readinitb(&rio, fd);

    ssize_t nread;
    while (1) {
        nread = read(fd, buf, MAXLINE);
        printf("read size %d\n", nread);
        // printf("read buf %s\n", buf);
        if (nread == -1) {
            if ((errno == EINTR)) {
                printf("errno EINTR\n");
                continue;
            } else {
                printf("errno other\n");
                return ;
            }
        } else if (nread == 0){
            return ;
        } else {
            break;
        }
    }
    return ;
}


int main()
{
    int listenfd, connfd;
    struct sockaddr_in cliaddr, servaddr;
    socklen_t clilen;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(9877);
    bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));
    listen(listenfd, 10);
    setnonblocking(listenfd);

    struct epoll_event ev, events[100];
    int kdfpd = epoll_create(100);
    ev.events = EPOLLIN;
    ev.data.fd = listenfd;
    int s = epoll_ctl(kdfpd, EPOLL_CTL_ADD, listenfd, &ev);
    int nfds;
    
    for (;;) {
        nfds = epoll_wait(kdfpd, events, 100, 1000);
        for (int i = 0; i < nfds; i++) {
            if (events[i].data.fd == listenfd) {
                connfd = accept(listenfd, (struct sockaddr *) &cliaddr, &clilen);
                log_info("the accept fd is %d\n", connfd);
                setnonblocking(connfd);
                ev.events = EPOLLIN;
                ev.data.fd = connfd;
                epoll_ctl(kdfpd, EPOLL_CTL_ADD, connfd, &ev);
            } else {
                str_echo(events[i].data.fd);
            }
        }
    }

    return 0;
}
