
/*!
 * @file      allocator.cpp
 *
 * @brief     The main idea of allocator is to completely remove memory 
 *            fragmentation, providing safety mechanisms and some of the
 *            std containers.
 *            In order to completely remove the fragmentation, the CPU
 *            will reorganize all the affected elements (the elements beyond the
 *            resized one).
 *            However, fixed elements are not moved (considering fixed elements
 *            the ones with the same lifespan than BasicAllocation and
 *            with a fixed size).
 *            Basically, BasicAllocation allows to create multiple memory models
 *            within the desired area (and the area can be reserved through the 
 *            linker, stack, global, heap, etc).
 *
 *            Upper layers will allow to BasicAllocation to modify their
 *            pointers, so copy/move will have to be designed based on this
 *            idea.
 *
 * @note      It is developer design to work out the size of the BasicAllocation,
 *            if double copy is needed, how many elements are in, pure dynamic,
 *            pure fixed, mixed, etc...
 *
 * @note      Threading: same allocator doesn't support different threads
 *
 * @date      10 May 2020
 *
 * @version   Revision 1.0.0
 */


#include <cstdio>
#include <cstdint>
#include <iostream>
#include <cstring>
#include <mutex>
#include "mgmt.hpp"
#include "allocator.hpp"

#define ALIGN    1

namespace cus {

uint32_t MathArch::crc32(const void *start, const void *end)
{
    const uint32_t POLY = 0xedb88320;
    int k;

    const char *a = (char *)start;
    const char *b = (char *)end;
    uint32_t len = b - a;
    uint32_t crc = 0;
    while (len--) {
        crc ^= *a++;
        for (k = 0; k < 8; k++)
            crc = crc & 1 ? (crc >> 1) ^ POLY : crc >> 1;
    }
    return ~crc;
}

arch_t MathArch::roundUp(arch_t numToRound, uint32_t multiple)
{
    if (multiple == 0)
        return numToRound;

    arch_t remainder = numToRound % multiple;
    if (remainder == 0)
        return numToRound;

    return numToRound + multiple - remainder;
}

BasicAllocation::BasicAllocation() {}


BasicAllocation::BasicAllocation(const void *startSection, \
        const void *endSection) noexcept {

    sizeArena=(((((arch_t)endSection)-(arch_t)startSection)));
    start=((arch_t *)(startSection));
    end=((arch_t *)((arch_t)start+sizeArena));

    lastData=0;
    lastAddr=0;
}

bool BasicAllocation::allocate(const arch_t addrRequester, void*& requester, \
        std::size_t nBytes) noexcept {
    //std::lock_guard<std::mutex> guard(allocator_mutex);

    bool success=true;
    void * currentFreeAddr = (void *)((uint8_t *)start+lastData);

    for(uint32_t i=0;i<lastAddr;i=i+TOTAL_ELEMENTS) {
        if(addrRequester==end[((sarch_t)i*(-1))-POINTER_TO_REQUESTER]) {
            success=false;
            break;
        }
    }

    if(success==true) {
        uint32_t incrementSize = nBytes;
        uint32_t addrSectorSize = lastAddr*(sizeof(arch_t));
        uint32_t dataSectorSize = lastData;
        uint32_t used = addrSectorSize + dataSectorSize;

        //std::cout << sizeArena << " " << (incrementSize+used+TOTAL_ELEMENTS*sizeof(arch_t)) \
            << std::endl;

        if(sizeArena>=(incrementSize+used+TOTAL_ELEMENTS*sizeof(arch_t))) {
            // Update pointers
            arch_t *endV = (arch_t *)end;
            endV[((sarch_t)lastAddr*(-1))-POINTER_TO_DATA]=(arch_t)currentFreeAddr;
            requester=currentFreeAddr;
            endV[((sarch_t)lastAddr*(-1))-DATA_SIZE]=nBytes;
            endV[((sarch_t)lastAddr*(-1))-POINTER_TO_REQUESTER]=(arch_t)addrRequester;
            // Update Add
            lastAddr+=TOTAL_ELEMENTS;
            // Update data
            lastData += nBytes;

        } else {
            success=false;
        }
    }

    return success;
}

bool BasicAllocation::deallocate(const arch_t addrRequester) noexcept {
    bool valueFound=false;
    //std::lock_guard<std::mutex> guard(allocator_mutex);

    arch_t numberOfObjects=(lastAddr)/TOTAL_ELEMENTS;
    for(uint32_t idx=0;idx<numberOfObjects;idx++) {
        arch_t value = (arch_t) (end[(sarch_t)idx*TOTAL_ELEMENTS*(-1) - \
                POINTER_TO_REQUESTER]);
        if(addrRequester==value) {
            valueFound=true;
            uint32_t size = end[((TOTAL_ELEMENTS*(sarch_t)idx)*(-1))-DATA_SIZE];
            removeFromAddresses(idx,(void *)addrRequester,size);
            break;
        }
    }
#ifdef TODO
    if(valueFound==false) {
        // log error
    }
#endif
    return valueFound;
}

bool BasicAllocation::removeElement(const arch_t addrRequester, void * posElement, \
        const size_t size) noexcept {
    //std::lock_guard<std::mutex> guard(allocator_mutex);

    arch_t numberOfObjects=(lastAddr)/TOTAL_ELEMENTS;
    bool valueFound=false;
    for(uint32_t idx=0;idx<numberOfObjects;idx++) {
        arch_t value = (arch_t) (end[(sarch_t)idx*TOTAL_ELEMENTS*(-1) - \
                POINTER_TO_REQUESTER]);
        if(addrRequester==value) {
            valueFound=true;
            removeFromAddresses(idx,posElement,size);
            break;
        }
    }
    if(valueFound==false) {
        std::cout << "CRITICAL2" << std::endl;
    }

    return valueFound;
}



bool BasicAllocation::reallocate(void*& requester, const std::size_t pBytes, \
        const std::size_t nBytes) noexcept {
    //std::lock_guard<std::mutex> guard(allocator_mutex);

    bool success=false;
    // check if it fits
    bool itFits=false;
    void * currentFreeAddr = (void *)((uint8_t *)start+lastData);

    uint32_t incrementSize = nBytes - pBytes;
    uint32_t addrSectorSize = lastAddr*(sizeof(arch_t));
    uint32_t dataSectorSize = lastData;
    uint32_t used = addrSectorSize + dataSectorSize;

    if((sizeArena)>=incrementSize+used) {
        itFits=true;
    }
    //move the rest of the data
    if(itFits==true) {
        arch_t numberOfObjects=(lastAddr)/TOTAL_ELEMENTS;
        bool valueFound=false;
        sarch_t idx;
        for(idx=0;idx<numberOfObjects;idx++) {
            arch_t *value = (arch_t *) (end[((sarch_t)idx*TOTAL_ELEMENTS*(-1)) - \
                    POINTER_TO_DATA]);
            if(requester==(void *)value) {
                valueFound=true;
                arch_t *endV = (arch_t *)end;
                endV[((TOTAL_ELEMENTS*(sarch_t)idx)*(-1))-DATA_SIZE] = nBytes;
                // update its size
                lastData+=(nBytes-pBytes);
                success=true;
                break;
            }
        }

        if(valueFound==true) {
            for(uint32_t value=(numberOfObjects-1);value>idx;value--) {

                // Move data
                arch_t data_ = end[((TOTAL_ELEMENTS*(sarch_t)value)*(-1)) - \
                               POINTER_TO_DATA];

                void * moveTo = (void *)(data_+(nBytes-pBytes));
                uint32_t sizeToMove = (arch_t) end[((TOTAL_ELEMENTS*\
                            (sarch_t)value)*(-1))-DATA_SIZE];

                memcpy2(moveTo,(void *)data_,sizeToMove);

                // Update pointer to the data in the address region
                arch_t *endV = (arch_t *)end;
                endV[((TOTAL_ELEMENTS*(sarch_t)value)*(-1))-POINTER_TO_DATA] = (arch_t)moveTo;

                // Update the pointer of the caller object to the allocated region
                arch_t **object = (arch_t **)end[((TOTAL_ELEMENTS*
                            (sarch_t)value)*(-1))-POINTER_TO_REQUESTER];
                *object = (arch_t *)moveTo;
            }
        }
    }

    return success;
}

uint32_t BasicAllocation::elements() const noexcept {
    return (lastAddr)/TOTAL_ELEMENTS;
}

/*
uint32_t BasicAllocation::sizeElement(void*& requester) {
    arch_t numberOfObjects=(lastAddr)/TOTAL_ELEMENTS;
    for(uint32_t idx=0;idx<numberOfObjects;idx++) {
        arch_t *value = (arch_t *) (end[(sarch_t)idx*TOTAL_ELEMENTS*(-1)]);
        if(requester==(void*)value) {
            return end[((TOTAL_ELEMENTS*(sarch_t)idx)*(-1))-DATA_SIZE];
        }
    }
}
*/

void BasicAllocation::removeFromAddresses(uint32_t indexToDelete, \
        void * const element, size_t size) noexcept {
    //std::lock_guard<std::mutex> guard(allocator_mutex);

    arch_t numberOfObjects=(lastAddr)/TOTAL_ELEMENTS;
    uint32_t sizeObject = end[((TOTAL_ELEMENTS*(sarch_t)indexToDelete)*(-1))-DATA_SIZE];
    if(size > sizeObject) size = sizeObject;

    uint8_t skipElement = 0;
    if(size == sizeObject) {
        skipElement = 1;
    } else {
        arch_t *endV = (arch_t *)end;
        endV[((TOTAL_ELEMENTS*(sarch_t)indexToDelete)*(-1))-DATA_SIZE] = \
                sizeObject - size;
        memcpy2(element,(void *)((char *)element + size), \
            sizeObject-((arch_t)(((char *)element + size))- \
                endV[((TOTAL_ELEMENTS*(sarch_t)indexToDelete)*(-1))-POINTER_TO_DATA]));

        indexToDelete++;
    }

    for(uint32_t idx=indexToDelete;idx<(numberOfObjects-skipElement);idx++) {
        // Update pointers
        arch_t *endV = (arch_t *)end;
        sarch_t prevData = ((TOTAL_ELEMENTS*(sarch_t)idx)*(-1)) - \
                           (skipElement*TOTAL_ELEMENTS)-POINTER_TO_DATA;
        sarch_t prevSize=((TOTAL_ELEMENTS*(sarch_t)idx)*(-1)) - \
                         (skipElement*TOTAL_ELEMENTS)-DATA_SIZE;
        sarch_t prevAddrRequester = 
            ((TOTAL_ELEMENTS*(sarch_t)idx)*(-1))-(skipElement*TOTAL_ELEMENTS) - \
            POINTER_TO_REQUESTER;

        arch_t oldDataAddr = end[prevData];
        arch_t sizeElement = end[prevSize];
        arch_t addrRequester = end[prevAddrRequester];
        arch_t newDataAddr = oldDataAddr - size;

        endV[((TOTAL_ELEMENTS*(sarch_t)idx)*(-1))-POINTER_TO_DATA] = newDataAddr;
        endV[((TOTAL_ELEMENTS*(sarch_t)idx)*(-1))-DATA_SIZE] = sizeElement;
        endV[((TOTAL_ELEMENTS*(sarch_t)idx)*(-1))-POINTER_TO_REQUESTER] = addrRequester;

        arch_t **object = (arch_t **)addrRequester;
        *object = (arch_t *)newDataAddr;

        memcpy2((void *)newDataAddr, (void *)oldDataAddr,sizeElement);
    }

    if(size == sizeObject) {
        lastAddr-=TOTAL_ELEMENTS;
    }
    lastData-=size;
}

void BasicAllocation::shrinkData() noexcept {
    //std::lock_guard<std::mutex> guard(allocator_mutex);

    arch_t numberOfObjects=(lastAddr)/TOTAL_ELEMENTS;
    arch_t expectedNextAddr = (arch_t)start;
    lastData=0;
    for(uint32_t idx=0;idx<numberOfObjects;idx++) {
        arch_t value = end[((TOTAL_ELEMENTS*(sarch_t)idx)*(-1))-POINTER_TO_DATA];
        if(expectedNextAddr != value) {
            // Move data
            uint32_t sizeToMove = (arch_t) end[((TOTAL_ELEMENTS*(sarch_t)idx)*\
                    (-1))-DATA_SIZE];
            memcpy2((void *)expectedNextAddr,(void *)value,sizeToMove);

            // Update pointer to the data in the address region
            arch_t *endV = (arch_t *)end;
            endV[((TOTAL_ELEMENTS*(sarch_t)idx)*(-1))-POINTER_TO_DATA] = expectedNextAddr;

            // Update the pointer of the caller object to the allocated region
            arch_t *object = (arch_t *)end[((TOTAL_ELEMENTS*
                        (sarch_t)idx)*(-1))-POINTER_TO_REQUESTER];
            *object = expectedNextAddr;
        }
        uint32_t size = (arch_t) end[((TOTAL_ELEMENTS*(sarch_t)idx)*(-1))-DATA_SIZE];
        expectedNextAddr += size;
        lastData+=size;
    }
}




void BasicAllocation::showMap() {
    arch_t numberOfObjects=(lastAddr)/TOTAL_ELEMENTS;

    std::cout << "\nobjects : " << numberOfObjects << std::endl;
    std::cout << "start addr: " << start << std::endl;
    std::cout << "last data : " << lastData  << std::endl;
    std::cout << "first addr: " << lastAddr  << std::endl;
    std::cout << "end addr  : " << end << std::endl;

    for(uint32_t idx=0;idx<numberOfObjects;idx++) {
        arch_t value = end[((TOTAL_ELEMENTS*(sarch_t)idx)*(-1))-POINTER_TO_DATA];
        arch_t req = end[((TOTAL_ELEMENTS*(sarch_t)idx)*(-1))-POINTER_TO_REQUESTER];
        arch_t size = end[((TOTAL_ELEMENTS*(sarch_t)idx)*(-1))-DATA_SIZE];
        std::cout << "-Present: " << (arch_t)value << " size:" << size<<" req: "<<(arch_t)req<< "\n";
    }
}

CrcAllocation::CrcAllocation(const void *startSection,const void *endSection) noexcept {

    sizeArena=((((arch_t)endSection)-(arch_t)startSection)/2)-sizeof(arch_t);
    startCRC=((arch_t *)startSection);
    start=((arch_t *)(((arch_t)startCRC)+sizeof(arch_t)));
    end=((arch_t *)((arch_t)start+sizeArena));
    startMirrorCRC=((arch_t *)((arch_t)end)); // end goes backwards
    startMirror=((arch_t *)((arch_t)startMirrorCRC + sizeof(arch_t)));
    endMirror=((arch_t *)((arch_t)startMirror+sizeArena));

    if(endMirror != endSection) {
        std::cout << "REPORT ERROR" << std::endl;
    }

    lastData=0;
    lastAddr=0;

    updateMirror();
}

void CrcAllocation::updateMirror() noexcept {
    //std::lock_guard<std::mutex> guard(allocator_mutex);

    // This will have to copy only the modified
    memcpyMirror((void*)startMirror,(const void*)start, \
                 sizeArena);
    uint32_t crcOrig = crc32((const void *)(start),(const void *)((arch_t)end-1));
    uint32_t crcMirror=crc32((const void *)(startMirror),\
            (const void *)((arch_t)endMirror-1));

    *startCRC=(arch_t)crcOrig;
    *startMirrorCRC=(arch_t)crcMirror;
}


bool CrcAllocation::checkConsistency() noexcept {
    //std::lock_guard<std::mutex> guard(allocator_mutex);

    bool pass=true;

    // check CRC
    uint32_t crcOrig = crc32((const void *)(start),(const void *)((arch_t)end-1));
    uint32_t crcMirror = crc32((const void *)(startMirror),\
            (const void *)((arch_t)endMirror-1));

    if((*startCRC==(arch_t)crcOrig) && (*startMirrorCRC == (arch_t)crcMirror)) {
    } else if ((*startCRC != (arch_t)crcOrig) && \
            (*startMirrorCRC == (arch_t)crcMirror)) {
        memcpyMirror((void*)start,(const void*)startMirror, \
                     sizeArena);
    } else if((*startMirrorCRC != (arch_t)crcMirror) && \
            (*startCRC==(arch_t)crcOrig)) {
        memcpyMirror((void*)startMirror,(const void*)start, \
                     sizeArena);
    } else {
        pass=false;
    }

    return pass;
}



} // end namespace
