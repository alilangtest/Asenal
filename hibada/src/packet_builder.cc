#include "packet_builder.h"

extern SdkGet *SdkGetBuilder(const char *table, const Slice &key)
{
    SdkGet *sdkGet = new SdkGet();
    sdkGet->set_opcode(kSdkGet);
    sdkGet->set_table(table);
    sdkGet->set_key(key.data());
    return sdkGet;
}

extern SdkSet *SdkSetBuilder(const char *table, const Slice &key, const Slice &value)
{
    SdkSet *sdkSet = new SdkSet();
    sdkSet->set_opcode(kSdkSet);
    sdkSet->set_table(table);
    sdkSet->set_key(key.data());
    sdkSet->set_value(value.data());
    return sdkSet;
}

extern Status GetBufferParse(const int32_t opcode, const char *rbuf, const
        int32_t rbuf_len, std::string *value)
        {
    Status s;
    std::string buf;
    buf = std::string(rbuf, rbuf_len);
    if (opcode == kSdkInvalidOperation) {
        SdkInvalidOperation sdkInvalidOperation;
        if (!sdkInvalidOperation.ParseFromString(buf)) {
            s = Status::Corruption("Parse invalid operation error");
            return s;
        }
        if (sdkInvalidOperation.what() == 1003) {
            return Status::NotFound("Can't not found the key");
        }
        return Status::InvalidArgument("Invalid operation");
    } else {
        SdkGetRet *sdkGetRet = new SdkGetRet();
        if (!sdkGetRet->ParseFromString(buf)) {
            s = Status::Corruption("Parse Get Ret error");
            return s;
        }
        if (sdkGetRet->opcode() == kSdkGetRet) {
            s = Status::OK();
            value->assign(sdkGetRet->value().data(), sdkGetRet->value().size());
        } else {
            s = Status::IOError("Get error");
        }
        delete(sdkGetRet);
    }
    return s;
}

Status SetBufferParse(const int32_t opcode, const char *rbuf, const int32_t rbuf_len)
{
    Status s;
    std::string buf = std::string(rbuf, rbuf_len);

    if (opcode == kSdkInvalidOperation) {
        SdkInvalidOperation sdkInvalidOperation;
        if (sdkInvalidOperation.ParseFromString(buf)) {
            s = Status::Corruption("Parse invalid operation error");
            return s;
        }
        if (sdkInvalidOperation.what() == 1003) {
            return Status::NotFound("Can't not found the key");
        }
        return Status::InvalidArgument("Invalid operation");
    } else {
        SdkSetRet *sdkSetRet = new SdkSetRet();
        if (!sdkSetRet->ParseFromString(buf)) {
            s = Status::Corruption("Parse error");
            return s;
        }
        if (sdkSetRet->opcode() == kSdkSetRet) {
            s = Status::OK();
        } else {
            s = Status::IOError("Set error");
        }
        delete(sdkSetRet);
    }
    return s;
}
