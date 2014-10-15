#ifndef __BADA_DEFINE_H
#define __BADA_DEFINE_H

#define COMMAND_HEADER_LENGTH 4 // Part1 length
#define COMMAND_CODE_LENGTH   4 // Part2 length

#define MAX_PACKAGE_LEN (1024*256)

enum CommandCode {
    kSdkInvalidOperation = 512,
    kSdkSet = 513,
    kSdkSetRet = 514,
    kSdkDelete = 515,
    kSdkDeleteRet = 516,
    kSdkGet = 518,
    kSdkGetRet = 519,
};

enum BlockType {
    kBlock = 0,
    kNonBlock = 1,
};

#endif
