#ifndef __TICK_DEFINE_H__
#define __TICK_DEFINE_H__

#define TICK_MAX_CLIENTS 10240
#define TICK_MAX_MESSAGE 10240
#define TICK_THREAD_NUM 15

enum EventStatus{
    kNone = 0,
    kReadable = 1,
    kWriteable = 2,
};

#endif
