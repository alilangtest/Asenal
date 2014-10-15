#ifndef __BADA_CONTEXT_H_
#define __BADA_CONTEXT_H_
#include "bada_define.h"
#include "fcntl.h"
#include "bada_util.h"
#include "status.h"

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

class BadaContext {
public:
    BadaContext();
    ~BadaContext();
    // int err; // Error flags, 0 when there is no error
    // char errstr[128]; // String representation of error when applicable

    Status BuildObuf(int32_t opcode, const char *packet, const int packet_len);

    Status BadaBufferWrite();
    Status BadaBufferRead();

    void BadaCloseFd();

    void set_fd(const int fd) { fd_ = fd; }
    void set_flags(const int flags) { flags_ = flags; }

    Status SetBlockType(BlockType type);
    Status SetTcpNoDelay();

    int fd() { return fd_; }
    int flags() { return flags_; }
    const char *rbuf() { return rbuf_; }
    int32_t rbuf_len() { return rbuf_len_; }
    int32_t r_opcode() { return r_opcode_; }
    // redisReader *reader; // Protocol reader
private:

    int fd_;
    int flags_;
    Status BadaBufferReadHeader();
    Status BadaBufferReadCode();
    Status BadaBufferReadPacket();
    char *obuf_; // Write buffer
    int32_t obuf_len_;

    int header_len_;
    char *rbuf_;
    int32_t rbuf_len_;
    int32_t r_opcode_;

    BadaContext(const BadaContext&);
    void operator = (const BadaContext&);
};

#endif
