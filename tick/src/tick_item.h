#ifndef __TICK_ITEM_H__
#define __TICK_ITEM_H__

#include "status.h"
#include "tick_define.h"

class TickItem
{
public:
    TickItem(const char* str, int len);
    ~TickItem();
    char* msg() { return msg_; }

private:

    char *msg_;

    /*
     * No copy && assigned allowed
     */
    TickItem(const TickItem&);
    void operator=(const TickItem&);

};

#endif
