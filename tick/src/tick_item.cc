#include "tick_item.h"
#include "tick_define.h"
#include <sys/stat.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include "csapp.h"

TickItem::TickItem(const char* str, int len)
{
    msg_ = (char *)malloc(sizeof(char) * TICK_MAX_MESSAGE);
    memcpy(msg_, str, len);
}

TickItem::~TickItem()
{
    free(msg_);
}
