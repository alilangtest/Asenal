#include "mario.h"
#include "producer.h"
#include "consumer.h"
#include "version.h"
#include "mutexlock.h"
#include "port.h"
#include <iostream>
#include <string>

namespace mario {

struct Mario::Writer {
    Status status;
    bool done;
    port::CondVar cv;

    explicit Writer(port::Mutex* mu) : cv(mu) { }
};


Mario::Mario(uint32_t consumer_num, Consumer::Handler *h)
    : consumer_num_(consumer_num),
    item_num_(0),
    env_(Env::Default()),
    readfile_(NULL),
    writefile_(NULL),
    versionfile_(NULL),
    version_(NULL),
    info_log_(NULL),
    bg_cv_(&mutex_),
    file_num_(0),
    pool_(NULL),
    exit_all_consume_(false),
    mario_path_("./log")
{
    const std::string filename = mario_path_ + "/write2file";
    const std::string manifest = mario_path_ + "/manifest";

    env_->CreateDir(mario_path_);
    env_->set_thread_num(consumer_num_);
    // env_->NewLogger("./mario.log", &info_log_);

#if defined(MARIO_MEMORY)
    pool_ = (char *)malloc(sizeof(char) * kPoolSize);
    if (pool_ == NULL) {
        log_err("malloc error");
        exit(-1);
    }
    producer_ = new Producer(pool_, kPoolSize);
    consumer_ = new Consumer(0, h, pool_, kPoolSize);
#endif

#if defined(MARIO_MMAP)

    Status s;
    if (!env_->FileExists(filename)) {
        log_info("Not exist file ");
        env_->NewWritableFile(filename, &writefile_);
        env_->NewSequentialFile(filename, &readfile_);
        env_->NewRWFile(manifest, &versionfile_);
        if (versionfile_ == NULL) {
            log_err("versionfile_ new error");
        }
        version_ = new Version(versionfile_);
        version_->set_item_num(0);
        version_->set_offset(0);
        version_->StableSave();
        log_info("offset %llu itemnum %d", version_->offset(), version_->item_num());

    } else {
        log_info("exist file ");
        env_->AppendWritableFile(filename, &writefile_);
        env_->AppendSequentialFile(filename, &readfile_);
        s = env_->NewRWFile(manifest, &versionfile_);
        if (s.ok()) {
            version_ = new Version(versionfile_);
            version_->InitSelf();
            log_info("offset %lld itemnum %d", version_->offset(), version_->item_num());
        } else {
            log_info("new REFile error");
        }
    }

    uint64_t filesize;
    env_->GetFileSize(filename, &filesize);
    log_info("file size %d", filesize);

    producer_ = new Producer(writefile_, filesize);
    consumer_ = new Consumer(readfile_, version_->offset(), h, version_);

#endif
    for (uint32_t i = 0; i < consumer_num_; i++) {
        env_->StartThread(&Mario::BGWork, this);
    }
    // env_->Schedule(&Mario::BGWork, this);
}

Mario::~Mario()
{
    mutex_.Lock();
    exit_all_consume_ = true;
    bg_cv_.SignalAll();
    while (consumer_num_ > 0) {
        bg_cv_.Wait();
    }
    mutex_.Unlock();

    delete consumer_;
    delete producer_;
    // delete info_log_;

#if defined(MARIO_MMAP)
    delete version_;
    delete versionfile_;
#endif

    delete writefile_;
    delete readfile_;
#if defined(MARIO_MEMORY)
    free(pool_);
#endif
    // delete env_;
}


void Mario::BGWork(void *m)
{
    reinterpret_cast<Mario*>(m)->BackgroundCall();
}

void Mario::BackgroundCall()
{
    while (1) {
        {
        mutex_.Lock();
#if defined(MARIO_MMAP)
        while (version_->item_num() <= 0) {
#endif

#if defined(MARIO_MEMORY)
        while (item_num_ <= 0) {
#endif
            if (exit_all_consume_) {
                consumer_num_--;
                mutex_.Unlock();
                bg_cv_.Signal();
                pthread_exit(NULL);
            }
            bg_cv_.Wait();
        }
        consumer_->Consume();
#if defined(MARIO_MMAP)
        version_->minus_item_num();
        version_->StableSave();
#endif

#if defined(MARIO_MEMORY)
        item_num_--;
#endif
        mutex_.Unlock();
        }
    }
    return ;
}

Status Mario::Put(const std::string &item)
{
    Status s;

    {
    MutexLock l(&mutex_);
    s = producer_->Produce(Slice(item.data(), item.size()));
    if (s.ok()) {
#if defined(MARIO_MMAP)
        version_->plus_item_num();
        version_->StableSave();
#endif

#if defined(MARIO_MEMORY)
        item_num_++;
#endif
    }

    }
    bg_cv_.Signal();
    return s;
}

Status Mario::Get()
{
    Status s;

    std::string scratch;
    MutexLock l(&mutex_);
    s = consumer_->Consume();

    return s;
}


} // namespace mario

std::string itos(unsigned long num)
{
    std::string buf = "";
    unsigned long mod;
    while (num > 0) {
        mod = num % 10;
        buf = char(mod + '0') + buf;
        num /= 10;
    }
    return buf;
}
using namespace mario;


void *func(void *arg)
{
    mario::Status s;
    mario::Mario* m = reinterpret_cast<Mario*>(arg);
    for (int i = 0; i < 100; i++) {
        s = m->Put(itos(pthread_self()) + "" + itos(i) + "chenzongzhi\n");
    }
    return NULL;
}

