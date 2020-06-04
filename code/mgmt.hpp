

#ifndef _MEMORY_MGMT_HPP_
#define _MEMORY_MGMT_HPP_



class MemoryMgmt {
    public:
        void * memcpy2(void *dest, const void *src, size_t len);
        void * memcpyMirror(void *dest, const void *src, size_t len);
        uint32_t checkMirror(const void *startA, const void *startB, size_t len);
};
#endif
