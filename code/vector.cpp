

/*!
 * @file      vector.cpp 
 *
 * @brief     The source code of this file is part of the custom library. It defines
 *            the body of the class Vector.
 *
 * @date      10 May 2020
 *
 * @version   Revision 1.0.0
 */


#include <initializer_list>
#include <cstdint>
#include "allocator.hpp"
#include "vector.hpp"

namespace cus {

template <typename T, typename A>
Vector<T,A>::Vector(const A& section) noexcept {
    internalFailure=false;
    arena = (void *)&section;
    aMem=nullptr;
    elements=0;
}

template <typename T, typename A>
Vector<T,A>::Vector(const A& section,const std::initializer_list<T> cList) noexcept {
    internalFailure=false;
    arena = (void *)&section;
    aMem=nullptr;
    elements=0;
    for (T x : cList) {
        push_back(x);
    }
}

template <typename T, typename A>
Vector<T,A>::~Vector() noexcept {
    elements=0;
    ((A *)arena)->deallocate((arch_t)&aMem);
}


template <typename T, typename A>
Vector<T,A>::Vector(const Vector& vector) noexcept {
    internalFailure=false;
    arena = (void *)vector.arena;
    aMem=nullptr;
    elements=0;

    push_back(vector);


}

template <typename T, typename A>
Vector<T,A>& Vector<T,A>::operator=(const Vector<T,A>& vector) noexcept {
    internalFailure=false;
    arena = (void *)vector.arena;
    aMem=nullptr;
    elements=0;

    push_back(vector);
}

template <typename T, typename A>
Vector<T,A>::Vector(const Vector&& vector) noexcept {
    internalFailure=false;
    arena = (void *)vector.arena;
    aMem=nullptr;
    elements=0;

    push_back(vector);
    vector.~Vector();
}

template <typename T, typename A>
Vector<T,A>& Vector<T,A>::operator=(const Vector<T,A>&& vector) noexcept {
    internalFailure=false;
    arena = (void *)vector.arena;
    aMem=nullptr;
    elements=0;

    push_back(vector);
    vector.~Vector();
}


template <typename T, typename A>
bool Vector<T,A>::push_back(const T value) noexcept {
    bool validAlloc = false;

    if(elements==0) {
        validAlloc = ((A *)arena)->allocate((arch_t)&aMem, aMem,sizeof(T));
    } else {
        std::size_t sizeBytes = elements * sizeof(T);
        validAlloc = ((A *)arena)->reallocate(aMem,sizeBytes,sizeBytes + sizeof(T));
    }

    if(aMem != nullptr && validAlloc==true) {
        *(elements + (T *)aMem) = value;
        elements++;
    } else {
        internalFailure=true;
    }

    return internalFailure;
}

template <typename T, typename A>
bool Vector<T,A>::push_back(const Vector<T,A>& toAppend) noexcept {
    for(uint32_t idx=0;idx<toAppend.size();idx++) {
        push_back(toAppend[idx]);
    }

    return internalFailure;
}

template <typename T, typename A>
bool Vector<T,A>::resize(const uint32_t newElements) noexcept {
    bool validAlloc = false;

    if(elements==0) {
        validAlloc = ((A *)arena)->allocate((arch_t)&aMem, aMem,\
                (newElements * sizeof(T)));
    } else {
        std::size_t sizeBytes = elements * sizeof(T);
        validAlloc = ((A *)arena)->reallocate(aMem,sizeBytes,\
                sizeBytes + (newElements*sizeof(T)));
    }

    if(aMem != nullptr && validAlloc==true) {
        elements=elements+newElements;
    } else {
        internalFailure=true;
    }

    return internalFailure;
}

template <typename T, typename A>
void Vector<T,A>::erase(const uint32_t index) noexcept {
    if(index < elements) {
        bool removed = ((A *)arena)->removeElement((arch_t)&aMem, \
                                (void *)((T *)aMem + index), sizeof(T));
        if(removed==true) {
            elements--;
        } else {
            internalFailure=true;
        }
    }
}

template <typename T, typename A>
void Vector<T,A>::erase(const uint32_t index, bool& erased) noexcept {
    erased=false;
    if(index < elements) {
        bool removed = ((A *)arena)->removeElement((arch_t)&aMem, \
                                (void *)((T *)aMem + index), sizeof(T));
        if(removed==true) {
            elements--;
            erased=true;
        } else {
            internalFailure=true;
        }
    }
}

template <typename T, typename A>
std::size_t Vector<T,A>::size() const noexcept {
    return elements;
}

template <typename T, typename A>
bool Vector<T,A>::isJeopardized() const noexcept {
    return internalFailure;
}

template <typename T, typename A>
T& Vector<T,A>::operator[](const uint32_t index) noexcept {
    return *((T *)aMem + index);
}

template <typename T, typename A>
const T& Vector<T,A>::operator[](const uint32_t index) const noexcept {
    return *((T *)aMem + index);
}

template <typename T, typename A>
T Vector<T,A>::at(const uint32_t index,bool& outOfBoundaries) noexcept {
    if(index<elements) {
        outOfBoundaries=false;
        return *((T *)aMem + index);
    } else {
        outOfBoundaries=true;
    }
}


template <typename T>
CrcVector<T>::CrcVector(const CrcAllocation& section) noexcept : \
        Vector<T,CrcAllocation>(section) {

}

template <typename T>
CrcVector<T>::CrcVector(const CrcAllocation& section, \
        const std::initializer_list<T> cList) noexcept: \
        Vector<T,CrcAllocation>(section,cList) {

    ((CrcAllocation *)arena)->updateMirror();
}



template <typename T>
CrcVector<T>::CrcVector(const CrcVector<T>& vector) noexcept:\
        Vector<T,CrcAllocation>(*((CrcAllocation *)vector.arena)) {

    push_back(vector);
    ((CrcAllocation *)arena)->updateMirror();
}


template <typename T>
CrcVector<T>& CrcVector<T>::operator=(const CrcVector<T>& vector) noexcept {

    push_back(vector);
    ((CrcAllocation *)arena)->updateMirror();
}

template <typename T>
CrcVector<T>::CrcVector(const CrcVector<T>&& vector) noexcept :\
        Vector<T,CrcAllocation>(*((CrcAllocation *)vector.arena)) {

    push_back(vector);
    ((CrcAllocation *)arena)->updateMirror();
    vector.~Vector<T,CrcAllocation>();
}

template <typename T>
CrcVector<T>& CrcVector<T>::operator=(const CrcVector<T>&& vector) noexcept {

    push_back(vector);
    ((CrcAllocation *)arena)->updateMirror();
    vector.~Vector<T,CrcAllocation>();
}


template <typename T>
bool CrcVector<T>::push_back(const T value) noexcept {
    bool validAlloc = false;

    bool crcOk = ((CrcAllocation *)arena)->checkConsistency();
    if(crcOk==true) {
        if(elements==0) {
            validAlloc = ((CrcAllocation *)arena)->allocate((arch_t)&aMem, \
                    aMem,sizeof(T));
        } else {
            std::size_t sizeBytes = elements * sizeof(T);
            validAlloc = ((CrcAllocation *)arena)->reallocate(aMem,sizeBytes, \
                    sizeBytes + sizeof(T));
        }

        if(aMem != nullptr && validAlloc==true) {
            *(elements + (T *)aMem) = value;
            elements++;
            ((CrcAllocation *)arena)->updateMirror();
        } else {
            internalFailure=true;
        }
    } else {
        // This does not have to be an internal failure. The mirror will be used
        // to workout the jeopardised areas of memory
        internalFailure=true;
    }

    return internalFailure;
}

template <typename T>
bool CrcVector<T>::push_back(const CrcVector<T>& toAppend) noexcept {
    for(uint32_t idx=0;idx<toAppend.size();idx++) {
        push_back(toAppend[idx]);
    }

    return internalFailure;
}

template <typename T>
bool CrcVector<T>::resize(const uint32_t newElements) noexcept {
    bool validAlloc = false;

    bool crcOk = ((CrcAllocation *)arena)->checkConsistency();
    if(crcOk==true) {

        if(elements==0) {
            validAlloc = ((CrcAllocation *)arena)->allocate((arch_t)&aMem, \
                    aMem,(newElements*sizeof(T)));
        } else {
            std::size_t sizeBytes = elements * sizeof(T);
            validAlloc = ((CrcAllocation *)arena)->reallocate(aMem, \
                    sizeBytes,sizeBytes + (newElements*sizeof(T)));
        }

        if(aMem != nullptr && validAlloc==true) {
            elements=newElements+elements;
            ((CrcAllocation *)arena)->updateMirror();
        } else {
            internalFailure=true;
        }
    } else {
        // This does not have to be an internal failure. The mirror will be used
        // to workout the jeopardised areas of memory
        internalFailure=true;
    }

    return internalFailure;
}

template <typename T>
void CrcVector<T>::erase(const uint32_t index) noexcept {
    if(index < elements) {
        bool crcOk = ((CrcAllocation *)arena)->checkConsistency();
        if(crcOk==true) {
            bool removed = ((CrcAllocation *)arena)->removeElement((arch_t)&aMem, \
                                    (void *)((T *)aMem + index), sizeof(T));
            if(removed==true) {
                elements--;
                ((CrcAllocation *)arena)->updateMirror();
            } else {
                internalFailure=true;
            }
        } else {
            // This does not have to be an internal failure. The mirror will be used
            // to workout the jeopardised areas of memory
            internalFailure=true;
        }
    }
}

template <typename T>
void CrcVector<T>::erase(const uint32_t index, bool& erased) noexcept {
    erased=false;
    if(index < elements) {
        bool crcOk = ((CrcAllocation *)arena)->checkConsistency();
        if(crcOk==true) {
            bool removed = ((CrcAllocation *)arena)->removeElement((arch_t)&aMem, \
                                    (void *)((T *)aMem + index), sizeof(T));
            if(removed==true) {
                elements--;
                erased=true;
                ((CrcAllocation *)arena)->updateMirror();
            } else {
                internalFailure=true;
            }
        } else {
            // This does not have to be an internal failure. The mirror will be used
            // to workout the jeopardised areas of memory
            internalFailure=true;
        }
    }
}

template <typename T>
T CrcVector<T>::at(const uint32_t index,bool& outOfBoundaries) noexcept {
    if(index<elements) {
        outOfBoundaries=false;
        bool crcOk = ((CrcAllocation *)arena)->checkConsistency();
        if(crcOk==true) {
            return *(index + (T *)aMem);
        } else {
            // This does not have to be an internal failure. The mirror will be used
            // to workout the jeopardised areas of memory
            internalFailure=true;
        }
    } else {
        outOfBoundaries=true;
    }
}

template <typename T>
void CrcVector<T>::set(const uint32_t index, bool& outOfBoundaries, \
        const T value) noexcept {
    if(index<elements) {
        outOfBoundaries=false;
        bool crcOk = ((CrcAllocation *)arena)->checkConsistency();
        if(crcOk==true) {
            *(index + (T *)aMem) = value;
            ((CrcAllocation *)arena)->updateMirror();
        } else {
            // This does not have to be an internal failure. The mirror will be used
            // to workout the jeopardised areas of memory
            internalFailure=true;
        }
    } else {
        outOfBoundaries=true;
    }
}

}; // end namespace
