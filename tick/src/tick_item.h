#ifndef __TICK_ITEM_H__
#define __TICK_ITEM_H__

class TickItem
{
public:
    TickItem() {};
    TickItem(int fd) : fd_(fd) {};

    int fd() { return fd_; }

private:
    int fd_;

};


#endif
