

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

template <typename T>
Vector<T>::Vector(){

}

template <typename T>
Vector<T>::Vector(BasicAllocation& section) {
    internalFailure=false;
    arena = &section;
    aMem=nullptr;
    elements=0;
}

template <typename T>
Vector<T>::Vector(BasicAllocation& section,std::initializer_list<T> cList) {
    internalFailure=false;
    arena = &section;
    aMem=nullptr;
    elements=0;
    for (T x : cList) {
        push_back(x);
    }
}

template <typename T>
Vector<T>::~Vector() {
    elements=0;
    arena->deallocate((arch_t)&aMem);
}

template <typename T>
bool Vector<T>::push_back(T value) {
    bool validAlloc = false;

    if(elements==0) {
        validAlloc = arena->allocate((arch_t)&aMem, aMem,sizeof(T));
    } else {
        std::size_t sizeBytes = elements * sizeof(T);
        validAlloc = arena->reallocate(aMem,sizeBytes,sizeBytes + sizeof(T));
    }

    if(aMem != nullptr && validAlloc==true) {
        *(elements + (T *)aMem) = value;
        elements++;
    } else {
        internalFailure=true;
    }

    return internalFailure;
}

template <typename T>
bool Vector<T>::push_back(Vector<T>& toAppend) {
    for(uint32_t idx=0;idx<toAppend.size();idx++) {
        push_back(toAppend[idx]);
    }

    return internalFailure;
}

template <typename T>
bool Vector<T>::resize(uint32_t newElements) {
    bool validAlloc = false;


    if(elements==0) {
        validAlloc = arena->allocate((arch_t)&aMem, aMem,(newElements * sizeof(T)));
    } else {
        std::size_t sizeBytes = elements * sizeof(T);
        validAlloc = arena->reallocate(aMem,sizeBytes,sizeBytes + (newElements*sizeof(T)));
    }

    if(aMem != nullptr && validAlloc==true) {
        elements++;
    } else {
        internalFailure=true;
    }

    return internalFailure;
}

template <typename T>
void Vector<T>::erase(uint32_t index) {
    if(index < elements) {
        bool removed = arena->removeElement((arch_t)&aMem, \
                                (void *)((T *)aMem + index), sizeof(T));
        if(removed==true) {
            elements--;
        } else {
            internalFailure=true;
        }
    }
}

template <typename T>
void Vector<T>::erase(uint32_t index, bool& erased) {
    erased=false;
    if(index < elements) {
        bool removed = arena->removeElement((arch_t)&aMem, \
                                (void *)((T *)aMem + index), sizeof(T));
        if(removed==true) {
            elements--;
            erased=true;
        } else {
            internalFailure=true;
        }
    }
}


template <typename T>
std::size_t Vector<T>::size() {
    return elements;
}

template <typename T>
bool Vector<T>::isJeopardized() {
    return internalFailure;
}

template <typename T>
const T& Vector<T>::operator[](uint32_t index) const {
    return *((T *)aMem + index);
}

template <typename T>
T Vector<T>::at(uint32_t index,bool& outOfBoundaries) const {
    if(index<elements) {
        outOfBoundaries=false;
        return *((T *)aMem + index);
    } else {
        outOfBoundaries=true;
    }
}



template <typename T>
CrcVector<T>::CrcVector(CrcAllocation& section) {
    internalFailure=false;
    arena = &section;
    aMem=nullptr;
    elements=0;
}

template <typename T>
CrcVector<T>::CrcVector(CrcAllocation& section,std::initializer_list<T> cList) {
    internalFailure=false;
    arena = &section;
    aMem=nullptr;
    elements=0;
    for (T x : cList) {
        push_back(x);
    }
}

template <typename T>
CrcVector<T>::~CrcVector() {
    elements=0;
    arena->deallocate((arch_t)&aMem);
}

template <typename T>
bool CrcVector<T>::push_back(T value) {
    bool validAlloc = false;

    bool crcOk = arena->checkConsistency();
    if(crcOk==true) {

        if(elements==0) {
            validAlloc = arena->allocate((arch_t)&aMem, aMem,sizeof(T));
        } else {
            std::size_t sizeBytes = elements * sizeof(T);
            validAlloc = arena->reallocate(aMem,sizeBytes,sizeBytes + sizeof(T));
        }

        if(aMem != nullptr && validAlloc==true) {
            *(elements + (T *)aMem) = value;
            elements++;
            arena->updateMirror();
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
bool CrcVector<T>::push_back(Vector<T>& toAppend) {
    resize(toAppend.size());
    for(uint32_t idx=0;idx<toAppend.size();idx++) {
        push_back(toAppend[idx]);
    }

    return internalFailure;
}

template <typename T>
bool CrcVector<T>::resize(uint32_t newElements) {
    bool validAlloc = false;

    bool crcOk = arena->checkConsistency();
    if(crcOk==true) {

        if(elements==0) {
            validAlloc = arena->allocate((arch_t)&aMem, aMem,sizeof(T));
        } else {
            std::size_t sizeBytes = elements * sizeof(T);
            validAlloc = arena->reallocate(aMem,sizeBytes,sizeBytes + sizeof(T));
        }

        if(aMem != nullptr && validAlloc==true) {
            elements++;
            arena->updateMirror();
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
void CrcVector<T>::erase(uint32_t index) {
    if(index < elements) {
        bool crcOk = arena->checkConsistency();
        if(crcOk==true) {
            bool removed = arena->removeElement((arch_t)&aMem, \
                                    (void *)((T *)aMem + index), sizeof(T));
            if(removed==true) {
                elements--;
                arena->updateMirror();
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
void CrcVector<T>::erase(uint32_t index, bool& erased) {
    erased=false;
    if(index < elements) {
        bool crcOk = arena->checkConsistency();
        if(crcOk==true) {
            bool removed = arena->removeElement((arch_t)&aMem, \
                                    (void *)((T *)aMem + index), sizeof(T));
            if(removed==true) {
                elements--;
                erased=true;
                arena->updateMirror();
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

}; // end namespace
