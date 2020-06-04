


#include <cstdio>
#include <cstdint>
#include <iostream>
#include "mgmt.hpp"

void * MemoryMgmt::memcpy2(void *dest, const void *src, size_t len) {
    if(dest >= src) {
        char *d = (char *)dest+len;
        const char *s = (char *)src+len;
        while (len--) {
            *--d = *--s;
        }
        return (void *)dest;
    } else {
        char *d = (char *)dest;
        const char *s = (char *)src;
        while (len--) {
            *d++ = *s++;
        }
        return (void *)dest;
    }
}

void * MemoryMgmt::memcpyMirror(void *dest, const void *src, size_t len) {
    if(dest >= src) {
        char *d = (char *)dest;
        const char *s = (char *)src;
        while (len--)
            *d++ = ~(*s++);
        return dest;
    } else {
        char *d = (char *)dest+len;
        const char *s = (char *)src+len;
        while (len--)
            *d-- = ~(*s--);
        return (void *)dest;
    }
}

uint32_t MemoryMgmt::checkMirror(const void *startA, const void *startB, size_t len) {
    uint32_t mismatch=0;
    const char *a = (char *)startA;
    const char *b = (char *)startB;
    while (len--) {
        if(*a++ != ~(*b++)) {
            mismatch++;
        }
    }
    return mismatch;
}


