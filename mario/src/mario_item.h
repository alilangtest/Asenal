#ifndef __MARIO_ITEM_H__
#define __MARIO_ITEM_H__

namespace mario {
    enum RecordType {
        kZeroType = 0,
        kFullType = 1,
        kFirstType = 2,
        kMiddleType = 3,
        kLastType = 4,
        kEof = 5,
        kBadRecord = 6,
        kOldRecord = 7
    };
    static const size_t kBlockSize = 1048576;

    // Header is Type(1 byte), length (2 bytes)
    static const size_t kHeaderSize = 1 + 2;
    static const uint64_t kPoolSize = 1024 * 1024 * 256;

}

#endif
