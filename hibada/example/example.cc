#include "bada.h"
#include "status.h"
#include <sys/time.h>

inline int64_t NowMicros()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return static_cast<uint64_t>(tv.tv_sec) * 1000000 + tv.tv_usec;
}

int main()
{
    const char* hostname = "183.136.142.45";
    const char* table = "test_leveldb";
    Status s;
    BadaClient *b = new BadaClient(hostname, 10000, table);
    s = b->Connect();
    if (!s.ok()) {
        log_err("%s", s.ToString().c_str());
    } else {
        log_info("connect ok %s", s.ToString().c_str());
    }

    std::string key = "czz";
    std::string value = "tiancai";
    s = b->Set(key, value);
    if (!s.ok()) {
        log_info("set value error %s", s.ToString().c_str());
    } else if (!s.ok()) {
        log_info("set value success %s", s.ToString().c_str());
    }
    std::cout << value << std::endl;
    std::string key1 = "czz";
    std::string val1 = "baotiao11";
    int64_t st = NowMicros();
    int64_t ed = NowMicros();

    for (int i = 0; i < 10000; i++) {
        st = NowMicros();
        s = b->Set(key1, val1);
        if (!s.ok()) {
            log_err("set error");
        }
        ed = NowMicros();

        printf("total time %ld\n", ed - st);
        // qf_debug("mmap cost time %lld", ed - st);
        // if (!s.ok()) {
        //     log_err("%s", s.ToString().c_str());
        // } else {
        //     log_info("%s", s.ToString().c_str());
        // }
    }
    delete(b);
    return 0;
}
