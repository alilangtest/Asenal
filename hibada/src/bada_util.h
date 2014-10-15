#ifndef __BADA_UTIL_H__
#define __BADA_UTIL_H__

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdint.h>

extern void qf_free(void *ptr);

template<typename T>
class FreePtr {
    public:
        explicit FreePtr(T *ptr) : ptr_(ptr) {
        }
        ~FreePtr() { free(ptr_); }
    private:
        T *ptr_;
        // No copying allowed
        FreePtr(const FreePtr&);
        void operator=(const FreePtr&);
};

template<typename T>
class DeletePtr {
    public:
        explicit DeletePtr(T *ptr) : ptr_(ptr) {
        }
        ~DeletePtr() { delete(ptr_); }
    private:
        T *ptr_;
        // No copying allowed
        DeletePtr(const DeletePtr&);
        void operator=(const DeletePtr&);
};
#endif
