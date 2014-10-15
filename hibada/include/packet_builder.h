#ifndef __PACKET_BUILDER__
#define __PACKET_BUILDER__

#include "bada_sdk.pb.h"
#include "slice.h"
#include "string.h"



extern SdkGet *SdkGetBuilder(const char *table, const Slice &key);
extern SdkSet *SdkSetBuilder(const char *table, const Slice &key, const Slice
        &value);

extern Status GetBufferParse(const int32_t opcode, const char *rbuf, const
        int32_t rbuf_len, std::string *value);
extern Status SetBufferParse(const int32_t opcode, const char *rbuf, const
        int32_t rbuf_len);

#endif
