

/*!
 * @file      allocator.hpp
 *
 * @brief     This file provides the apis for the public allocator custom class.
 *            It is part of the cus namespace and it proposes a new way to
 *            manage the memory:
 *              - It allows to double copy the object and restore it if possible
 *              - It completely removes the memory fragmentation
 *              - It allows to define multiple BasicAllocation objects, so
 *                sections of memory can be independent
 *              - It allows to define the size and the specific addresses of the
 *                sections, so the user can create multiple sections
 *                through the linker
 *              - Every dynamic resize will reallocate all the objects in the
 *                BasicAllocation, so it will modify the pointer to the allocated
 *                section in upper layers
 *
 * @note      Every BasicAllocation object will get its own enviroment:
 *             - If BasicAllocation object was constructed with CRC support:
 *                  -----------------------------------------------------------
 *                  |    HIGHEST ADDR
 *                  -------------------
*                   |                         address to data area reserver for object 1
*                   |               object 1  size of object 1
*                   |                         pointer of object 1 to address to data
*                   | address area
*                   |                         address to data area reserver for object n
*                   |               object n  size of object n
*                   |                         pointer of object n to address to data
 *                  -----------------------------------------------------------
 *                  |                   fixed size elements
 *                  |               +++++++++++++++++++++++++++++++++++++++++++
 *                  |               arena for object n
 *                  |
 *                  | data area
 *                  |
 *                  |               arena for object 1
 *                  -----------------------------------------------------------
 *                  |    LOWEST ADDR
 *                  -----------------------------------------------------------
 *
 * @date      10 May 2020
 *
 * @author    jose.felipe.git@gmail.com
 *
 * @version   Revision 1.0.0
 *
 * @copyright GPL
 */

//Why reallocate has to receive the current size?
//Why deallocate receives addRequester and the other uses receiver?



#ifndef _ALLOCATOR_HPP_
#define _ALLOCATOR_HPP_


#include <cstdio>
#include <cstdint>
#include <iostream>
#include <mutex>
#include "mgmt.hpp"

typedef uint64_t arch_t;
typedef signed long long sarch_t;


namespace cus {

class MathArch {
    public:
        arch_t roundUp(arch_t numToRound, uint32_t multiple);
        uint32_t crc32(const void *start, const void *end);
};

class BasicAllocation: public MathArch, public MemoryMgmt {
    public:
        /*!
         * @brief   Constructor to cover a new area of memory
         * @param   startSection pointer to the starting address of the reserved
         *          area of memory
         * @param   endSection pointer to the ending address of the reserved
         *          area of memory
         */
        BasicAllocation(const void *startSection, const void *endSection);
        /*!
         * @brief   Copy constructor not allowed
         */
        BasicAllocation (const BasicAllocation&) = delete;
        /*!
         * @brief   Copy operator not allowed
         */
        BasicAllocation& operator= (const BasicAllocation&) = delete;
        /*!
         * @brief   Move constructor not allowed
         */
        BasicAllocation(BasicAllocation&&) = delete;
        /*!
         * @brief   Move operator not allowed
         */
        BasicAllocation& operator=(BasicAllocation&&) = delete;
        /*!
         * @brief   It is the way an object requests reserved space for itself.
         * @param   requester It is the address of the pointer which will
         *          point to the reserved area of memory
         * @param   addrRequester It is the pointer which will point to the
         *          reserved area of memory
         * @param   nBytes Number of bytes to be reserved for the requester
         * @return  True if the allocation was valid. Otherwise, False
         */
        bool allocate(arch_t addrRequester, void*& requester,std::size_t nBytes);
        /*!
         * @brief   It allows to resize the previously allocated memory for an
         *          object.
         * @param   requester It is the address of the pointer which will
         *          point to the reserved area of memory
         * @param   pBytes Size of the object before calling reallocate
         * @param   nBytes Desired size of the object
         * @note    This task rewrites all the addresses and data of the next
         *          objects, so avoid it if there are time constrictions.
         *          Check the objects which doesn't dinamically reallocate
         *          memory.
         * @return  True if the reallocation was valid, Otherwise, False.
         */
        bool reallocate(void*& requester,std::size_t pBytes, std::size_t nBytes);
        /*!
         * @brief   It wipes the reserved memory for an object and all its
         *          references.
         * @param   addRequester It is the pointer which will point to the
         *          reserved area of memory
         * @return  True if the reallocation was valid, Otherwise, False.
         */
        bool deallocate(arch_t addrRequester);
        /*!
         * @brief   It allows to remove part of the reserved memory of an
         *          object.
         * @param   addrRequester It is the pointer which will point to the
         *          reserved area of memory
         * @param   posElement address to indicate the first byte to remove
         * @param   size Number of bytes to remove
         * @note    This task rewrites all the addresses and data of the next
         *          objects, so avoid it if there are time constrictions.
         *          Check the objects which doesn't dinamically reallocate
         *          memory.
         * @return  True if the reallocation was valid, Otherwise, False.
         */
        bool removeElement(arch_t addrRequester, void * posElement, size_t size);
        /*!
         * @brief   It provides the number of allocated elements
         */
        uint32_t elements();
        /*!
         * @brief   It provides the amount of bytes reserved for an element
         * @note    This function might provide sensible information to
         *          another elementz
         */
        uint32_t sizeElement(void*& requester);
        /*!
         * @brief   If the object was created in double copy mode, 
         *          this member will update the mirroring and recalculate the
         *          CRCs.
         *          It means that this member has to be called when the memory is
         *          written in order to update the status
         */
        void updateMirror();
        /*!
         * @brief   If the object was created in double copu mode,
         *          this membre will check the arena, in order to work out if the
         *          current situation is the expected situation.
         *          If one of the copies does not match, it will overwrite it,
         *          using the information from the valid copy (in which the CRC
         *          matchs).
         *          If both copies does not match, it will consider the arena
         *          as corruptede and it will notify it to upper layers
         * @return  True if the consistency is valid or it was able to restore it.
         *          Otherwise, False.
         */
        bool checkConsistency();
        /*!
         * @brief   Debugging purposes
         */
        void showMap();
  protected:
        BasicAllocation();
        void removeFromAddresses(uint32_t indexToDelete, void * element, size_t size);
        void shrinkData();

        arch_t sizeArena;
        arch_t *start;
        arch_t *end;
        arch_t lastData;
        arch_t lastAddr;
        std::mutex allocator_mutex;
        enum mapPddress {
            POINTER_TO_DATA=1,
            DATA_SIZE=2,
            POINTER_TO_REQUESTER=3,
            TOTAL_ELEMENTS=3
        };
};



class CrcAllocation: public BasicAllocation {
    public:
        /*!
         * @brief   Constructor to cover a new area of memory
         * @param   startSection pointer to the starting address of the reserved
         *          area of memory
         * @param   endSection pointer to the ending address of the reserved
         *          area of memory
         */
        CrcAllocation(const void *startSection, const void *endSection);
        /*!
         * @brief   Copy constructor not allowed
         */
        CrcAllocation (const CrcAllocation&) = delete;
        /*!
         * @brief   Copy operator not allowed
         */
        CrcAllocation& operator= (const CrcAllocation&) = delete;
        /*!
         * @brief   Move constructor not allowed
         */
        CrcAllocation(CrcAllocation&&) = delete;
        /*!
         * @brief   Move operator not allowed
         */
        CrcAllocation& operator=(CrcAllocation&&) = delete;
        /*!
         * @brief   If the object was created in double copy mode, 
         *          this member will update the mirroring and recalculate the
         *          CRCs.
         *          It means that this member has to be called when the memory is
         *          written in order to update the status
         */
        void updateMirror();
        /*!
         * @brief   If the object was created in double copu mode,
         *          this membre will check the arena, in order to work out if the
         *          current situation is the expected situation.
         *          If one of the copies does not match, it will overwrite it,
         *          using the information from the valid copy (in which the CRC
         *          matchs).
         *          If both copies does not match, it will consider the arena
         *          as corruptede and it will notify it to upper layers
         * @return  True if the consistency is valid or it was able to restore it.
         *          Otherwise, False.
         */
        bool checkConsistency();
    private:
        arch_t *startMirror;
        arch_t *endMirror;
        arch_t * startCRC;
        arch_t * startMirrorCRC;
        uint32_t crcOrig;
        uint32_t crcMirror;
};

/*!
 * @brief   This class ensures that all the objects which are going to use the
 *          custom allocator has some needed members
 */
class Container {
    public:
        /*!
         * @brief   All the objets will have to provide a way to get its size
         */
        virtual std::size_t size()=0;
        /*!
         * @brief   All the objets will have to provide a way to know if the
         *          memory is corrupted
         */
        virtual bool isJeopardized()=0;
        /*!
         * @brief    Pointer to the reserved memory for this object
         */
        void* aMem;
};


}; // end namespace

#endif
