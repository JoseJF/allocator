

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
Vector<T,A>::Vector(A& section) {
    internalFailure=false;
    arena = (void *)&section;
    aMem=nullptr;
    elements=0;
}

template <typename T, typename A>
Vector<T,A>::Vector(A& section,std::initializer_list<T> cList) {
    internalFailure=false;
    arena = (void *)&section;
    aMem=nullptr;
    elements=0;
    for (T x : cList) {
        push_back(x);
    }
}

template <typename T, typename A>
Vector<T,A>::Vector(Vector& vector) {
    internalFailure=false;
    arena = (void *)vector.arena;
    aMem=nullptr;
    elements=0;

    push_back(vector);
}

template <typename T, typename A>
Vector<T,A>& Vector<T,A>::operator=(Vector<T,A>& vector) {
    internalFailure=false;
    arena = (void *)vector.arena;
    aMem=nullptr;
    elements=0;

    push_back(vector);
}

template <typename T, typename A>
Vector<T,A>::Vector(Vector&& vector) {
    internalFailure=false;
    arena = (void *)vector.arena;
    aMem=nullptr;
    elements=0;

    push_back(vector);
    vector.~Vector();
}

template <typename T, typename A>
Vector<T,A>& Vector<T,A>::operator=(Vector<T,A>&& vector) {
    internalFailure=false;
    arena = (void *)vector.arena;
    aMem=nullptr;
    elements=0;

    push_back(vector);
    vector.~Vector();
}

template <typename T, typename A>
Vector<T,A>::~Vector() {
    elements=0;
    ((A *)arena)->deallocate((arch_t)&aMem);
}

template <typename T, typename A>
bool Vector<T,A>::push_back(T value) {
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
bool Vector<T,A>::push_back(Vector<T,A>& toAppend) {
    for(uint32_t idx=0;idx<toAppend.size();idx++) {
        push_back(toAppend[idx]);
    }

    return internalFailure;
}

template <typename T, typename A>
bool Vector<T,A>::resize(uint32_t newElements) {
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
void Vector<T,A>::erase(uint32_t index) {
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
void Vector<T,A>::erase(uint32_t index, bool& erased) {
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
std::size_t Vector<T,A>::size() {
    return elements;
}

template <typename T, typename A>
bool Vector<T,A>::isJeopardized() {
    return internalFailure;
}

template <typename T, typename A>
T& Vector<T,A>::operator[](uint32_t index) {
    return *((T *)aMem + index);
}

template <typename T, typename A>
T Vector<T,A>::at(uint32_t index,bool& outOfBoundaries) {
    if(index<elements) {
        outOfBoundaries=false;
        return *((T *)aMem + index);
    } else {
        outOfBoundaries=true;
    }
}


template <typename T>
CrcVector<T>::CrcVector(CrcAllocation& section):Vector<T,CrcAllocation>(section) {

}

template <typename T>
CrcVector<T>::CrcVector(CrcAllocation& section,std::initializer_list<T> cList): \
        Vector<T,CrcAllocation>(section,cList) {

    ((CrcAllocation *)arena)->updateMirror();
}



template <typename T>
CrcVector<T>::CrcVector(CrcVector<T>& vector):\
        Vector<T,CrcAllocation>(*((CrcAllocation *)vector.arena)) {

    push_back(vector);
    ((CrcAllocation *)arena)->updateMirror();
}


template <typename T>
CrcVector<T>& CrcVector<T>::operator=(CrcVector<T>& vector) {

    push_back(vector);
    ((CrcAllocation *)arena)->updateMirror();
}

template <typename T>
CrcVector<T>::CrcVector(CrcVector<T>&& vector):\
        Vector<T,CrcAllocation>(*((CrcAllocation *)vector.arena)) {

    push_back(vector);
    ((CrcAllocation *)arena)->updateMirror();
    vector.~Vector<T,CrcAllocation>();
}

template <typename T>
CrcVector<T>& CrcVector<T>::operator=(CrcVector<T>&& vector) {

    push_back(vector);
    ((CrcAllocation *)arena)->updateMirror();
    vector.~Vector<T,CrcAllocation>();
}


template <typename T>
bool CrcVector<T>::push_back(T value) {
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
bool CrcVector<T>::push_back(CrcVector<T>& toAppend) {
    for(uint32_t idx=0;idx<toAppend.size();idx++) {
        push_back(toAppend[idx]);
    }

    return internalFailure;
}

template <typename T>
bool CrcVector<T>::resize(uint32_t newElements) {
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
void CrcVector<T>::erase(uint32_t index) {
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
void CrcVector<T>::erase(uint32_t index, bool& erased) {
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
T CrcVector<T>::at(uint32_t index,bool& outOfBoundaries) {
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
void CrcVector<T>::set(uint32_t index, bool& outOfBoundaries, T value) {
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
