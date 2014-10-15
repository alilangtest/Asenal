#include "bada_context.h"
#include "bada_util.h"

BadaContext::BadaContext()
{
    fd_ = -1;
    obuf_ = NULL;
    obuf_len_ = 0;
    header_len_ = 0;
    r_opcode_ = 0;
    rbuf_len_ = 0;
    rbuf_ = NULL;
}

BadaContext::~BadaContext()
{
    qf_free(obuf_);
    qf_free(rbuf_);
    BadaCloseFd();
}

void BadaContext::BadaCloseFd()
{
    if (fd_ >= 0) {
        close(fd_);
        fd_ = -1;
    }
}

Status BadaContext::SetBlockType(BlockType type)
{
    Status s;
    if ((flags_ = fcntl(fd_, F_GETFL, 0)) < 0) {
        s = Status::Corruption("F_GETFEL error");
        BadaCloseFd();
        return s;
    }
    if (type == kBlock) {
        flags_ &= (~O_NONBLOCK);
    } else if (type == kNonBlock) {
        flags_ |= O_NONBLOCK;
    }
    if (fcntl(fd_, F_SETFL, flags_) < 0) {
        s = Status::Corruption("F_SETFL error");
        BadaCloseFd();
        return s;
    }
    return Status::OK();
}

Status BadaContext::SetTcpNoDelay()
{
    Status s;
    int yes = 1;
    if (setsockopt(fd_, IPPROTO_TCP, TCP_NODELAY, &yes, sizeof(yes)) == -1) {
        s = Status::Corruption("setsockopt(TCO_NODELAY) error");
        BadaCloseFd();
        return s;
    }
    return s;
}

Status BadaContext::BuildObuf(int32_t opcode, const char *packet, const int
        packet_len)
{
    Status s;
    obuf_ = (char *)realloc(obuf_, sizeof(char) * (packet_len +
                COMMAND_HEADER_LENGTH + COMMAND_CODE_LENGTH));

    uint32_t code_len = COMMAND_CODE_LENGTH + packet_len;
    uint32_t u;

    u = htonl(code_len);
    memcpy(obuf_, &u, sizeof(uint32_t));
    u = htonl(opcode);
    memcpy(obuf_ + COMMAND_CODE_LENGTH, &u, sizeof(uint32_t));
    memcpy(obuf_ + COMMAND_HEADER_LENGTH + COMMAND_CODE_LENGTH, packet,
            packet_len);

    obuf_len_ = COMMAND_HEADER_LENGTH + COMMAND_CODE_LENGTH + packet_len;
    return s;
}

Status BadaContext::BadaBufferWrite()
{
    Status s;
    int32_t nwritten = 0;
    nwritten = write(fd_, obuf_, obuf_len_);
    if (nwritten == -1) {
        if ((errno == EAGAIN && !(flags_ & O_NONBLOCK)) || (errno == EINTR)) {
        } else {
            s = Status::IOError(obuf_, "write bada context error");
            return s;
        }
    } else if (nwritten > 0) {
        if (nwritten == obuf_len_) {
            return Status::OK();
        }
    }

    return s;
}

Status BadaContext::BadaBufferRead()
{
    Status s;
    s = BadaBufferReadHeader();
    if (!s.ok()) {
        return s;
    }
    // s = BadaBufferReadCode();
    // if (!s.ok()) {
    //     return s;
    // }
    // s = BadaBufferReadPacket();
    return s;
}

Status BadaContext::BadaBufferReadHeader()
{
    Status s;
    char buf[MAX_PACKAGE_LEN];
    int32_t nread = 0, integer = 0;
    while (1) {
        nread = read(fd_, buf, MAX_PACKAGE_LEN);
        if (nread == -1) {
            if ((errno == EAGAIN && !(flags_ & O_NONBLOCK)) ||
                    (errno == EINTR)) {
                continue;
            } else {
                s = Status::IOError("read command header error");
                return s;
            }
        } else {
            break;
        }
    }
    memcpy((char *)(&integer), buf, sizeof(int32_t));
    header_len_ = ntohl(integer);
    memcpy((char *)(&integer), buf + COMMAND_HEADER_LENGTH, sizeof(int32_t));
    r_opcode_ = ntohl(integer);
    rbuf_ = (char *)realloc(rbuf_, sizeof(char) * nread);
    memcpy(rbuf_, buf + COMMAND_HEADER_LENGTH + COMMAND_CODE_LENGTH, nread -
            COMMAND_HEADER_LENGTH - COMMAND_CODE_LENGTH);
    rbuf_len_ = nread - COMMAND_HEADER_LENGTH - COMMAND_CODE_LENGTH;
    return Status::OK();
}

Status BadaContext::BadaBufferReadCode()
{
    Status s;
    char buf[1024];
    int32_t nread = 0, integer = 0;
    while (1) {
        nread = recv(fd_, buf, COMMAND_CODE_LENGTH, 0);
        if (nread == -1) {
            if ((errno == EAGAIN && !(flags_ & O_NONBLOCK)) ||
                    (errno == EINTR)) {
                continue;
            } else {
                s = Status::IOError("read command code error");
                return s;
            }
        } else {
            break;
        }
    }
    memcpy((char *)(&integer), buf, sizeof(int32_t));
    r_opcode_ = ntohl(integer);
    return Status::OK();
}

Status BadaContext::BadaBufferReadPacket()
{
    Status s;
    char buf[MAX_PACKAGE_LEN];
    int nread = 0;
    while (1) {
        nread = read(fd_, buf, header_len_ - 4);
        if (nread == -1) {
            if ((errno == EAGAIN && !(flags_ & O_NONBLOCK)) ||
                    (errno == EINTR)) {
                continue;
            } else {
                s = Status::IOError("read data error");
                return s;
            }
        } else {
            break;
        }
    }
    rbuf_ = (char *)realloc(rbuf_, sizeof(char) * nread);
    memcpy(rbuf_, buf, nread);
    rbuf_len_ = nread;
    return s;
}

