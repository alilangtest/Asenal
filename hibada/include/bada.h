#ifndef __BADA_H_
#define __BADA_H_

#include "xdebug.h"
#include "slice.h"
#include "status.h"
#include "packet_builder.h"

#include <errno.h>
#include <stdio.h>
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <sys/time.h>



class BadaContext;
/* Client for a connection to Bada */
class BadaClient {
public:
    BadaClient(const char *hostname, const int port, const char *table);
    ~BadaClient();

    BadaContext *badaContext() { return badaContext_; };
    Status Connect();
    // Implement of the BadaClient interface
    Status Get(const std::string &key, std::string* value);
    Status Set(const Slice &key, const Slice &value);

private:
    const char *hostname_;
    int port_;
    struct timeval timeout_;
    const char *table_;
    // Status SdkGetBuilder();

    BadaContext *badaContext_;
    BadaClient(const BadaClient&);
    void operator = (const BadaClient&);
};

#endif
