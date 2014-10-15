#include "bada.h"
#include "bada_context.h"

BadaClient::BadaClient(const char *hostname, const int port, const char *table)
        : hostname_(hostname), port_(port), table_(table)
{
    timeout_.tv_sec = 1;
    timeout_.tv_usec = 500000;

    badaContext_ = new BadaContext();
}

BadaClient::~BadaClient()
{
    delete(badaContext_);
}

Status BadaClient::Connect()
{
    Status s = Status::OK();
    int sockfd, rv;

    char _port[6];
    struct addrinfo hints, *servinfo, *p;
    snprintf(_port, 6, "%d", port_);
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(hostname_, _port, &hints, &servinfo)) != 0) {
        hints.ai_family = AF_INET6;
        if ((rv = getaddrinfo(hostname_, _port, &hints, &servinfo)) != 0) {
            s = Status::IOError("tcp_connect error for ", hostname_);
            return s;
        }
    }
    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) ==
                -1) {
            continue;
        }
        badaContext_->set_fd(sockfd);
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            if (errno == EHOSTUNREACH) {
                close(sockfd);
                return Status::IOError("EHOSTUNREACH", "The target host cannot
                        be reached");
            } else if (errno == EINPROGRESS && (badaContext_->flags() &
                        O_NONBLOCK)) {
                /**
                 * This branch mean the fd is NONBLOCK, so EINPROGRESS is true
                 */
                continue;
            }
            s = Status::IOError(strerror(errno), "Connect server error");
            freeaddrinfo(servinfo);
            return s;
        }
        s = Status::OK();
        badaContext_->SetBlockType(kBlock);
        badaContext_->SetTcpNoDelay();
        freeaddrinfo(servinfo);
        return s;
    }
    if (p == NULL) {
        s = Status::IOError(strerror(errno), "Can't create socket ");
        return s;
    }
    freeaddrinfo(servinfo);
    freeaddrinfo(p);
    return s;
}

Status BadaClient::Get(const std::string &key, std::string *value)
{
    Status s;

    SdkGet *sdkGet = SdkGetBuilder(this->table_, key);
    DeletePtr<SdkGet> fp(sdkGet);
    std::string buf;
    sdkGet->SerializeToString(&buf);
    badaContext_->BuildObuf(kSdkGet, buf.c_str(), buf.size());

    s = badaContext_->BadaBufferWrite();
    if (!s.ok()) {
        return s;
    }
    s = badaContext_->BadaBufferRead();
    if (!s.ok()) {
        return s;
    }

    s = GetBufferParse(badaContext_->r_opcode(), badaContext_->rbuf(),
            badaContext_->rbuf_len(), value);
    return s;
}

Status BadaClient::Set(const Slice &key, const Slice &value)
{
    Status s;

    SdkSet *sdkSet = SdkSetBuilder(this->table_, key, value);
    DeletePtr<SdkSet> fp(sdkSet);
    std::string buf;
    sdkSet->SerializeToString(&buf);
    badaContext_->BuildObuf(kSdkSet, buf.c_str(), buf.size());

    s = badaContext_->BadaBufferWrite();

    if (!s.ok()) {
        return s;
    }
    s = badaContext_->BadaBufferRead();
    if (!s.ok()) {
        return s;
    }

    s = SetBufferParse(badaContext_->r_opcode(), badaContext_->rbuf(),
            badaContext_->rbuf_len());
    return s;
}
