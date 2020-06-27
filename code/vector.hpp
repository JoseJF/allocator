

/*!
 * @file      vector.hpp
 *
 * @brief     This file provides the apis for the public vector custom class.
 *            It is part of the cus namespace and it replicates the
 *            std::vector functionality but using a custom allocator.
 *
 * @note      In order to create an object of type cus::Vector, a valid allocator
 *            has to be declared and defined, in order to pass it to the constructor
 *            of a cus::Vector object.
 *
 * @date      10 May 2020
 *
 * @author    jose.felipe.git@gmail.com
 *
 * @version   Revision 1.0.0
 *
 * @copyright GPL
 */


#ifndef _CUS_VECTOR_HPP_
#define _CUS_VECTOR_HPP_

#include <initializer_list>
#include <cstdint>
#include "allocator.hpp"

namespace cus {

template <typename T, typename A=BasicAllocation>
class Vector: public Container {
    public:
        using Container::aMem;
        using Container::arena;
        /*!
         * @brief   Constructor to receive just an allocator, without initialisers
         * @param   section Reference of an object of typed BasicAllocation
         *          to be used a lower layer to manage the memory
         */
        explicit Vector(A& section);
        /*!
         * @brief   Constructor when receiving an allocator and a list to
         *          initialise the cus::Vector object
         * @param   section Reference of an object of typed BasicAllocation
         *          to be used a lower layer to manage the memory
         * @param   cList object used to initialise the object
         */
        explicit Vector(A& section,std::initializer_list<T> cList);
        /*!
         * @brief   Destructor to notify the lower layers that the memory used
         *          by the self is not needed anymore and it has to be released
         */
        virtual ~Vector();
        /*!
         * @brief   Copy constructir
         * @note    Temporally deleted (!0-3-5)
         */
        Vector(Vector& vector);
        /*!
         * @brief   Copy constructir
         * @note    Temporally deleted (!0-3-5)
         */
        Vector<T,A>& operator=(Vector<T,A>&);

        /*!
         * @brief   Move constructir
         * @note    Temporally deleted (!0-3-5)
         */
        Vector(Vector&& vector);
        /*!
         * @brief   Copy constructir
         * @note    Temporally deleted (!0-3-5)
         */
        Vector<T,A>& operator=(Vector<T,A>&&);

        /*!
         * @brief   It allows to append an element of type T at the end of
         *          object. It copies the element, so there will be two positions
         *          of memory with the same values.
         * @param   value it's the value to be added at the end of the vector
         * @note    add values dinamically is quite expensive due to the way the
         *          allocator works.
         *          See BasicAllocation in order to know what happens when
         *          an object allows dynamic size.
         *          In order to avoid this, when the size is well-known at
         *          compilation time and fixed, cus::array should be used.
         * @return  True if the object is not corrupted. Otherwise, False.
         */
        virtual bool push_back(T value);
        /*!
         * @brief   It allows to append elements of type T at the end of the
         *          object. It copies, so there will be different areas of
         *          memory with the same values
         * @param   toAppend cus::Vector<T> which contains the elements to append
         * @return  True if the object is not corrupted. Otherwise, False.
         */
        virtual bool push_back(Vector<T,A>& toAppend);
        /*!
         * @brief   It assignes more space for the object. The space is based on
         *          the type T, so different bytes might be allocated for the same
         *          amount of elements fwhen different Ts.
         * @param   newElements amount of elements to increment the Vector
         * @return  True if the resize was valid. Otherwise, False.
         */
        virtual bool resize(uint32_t newElements);
        /*!
         * @brief   It removes a specific element in the object
         * @param   index Position of the element in the object to remove
         * @return  True if the object is not corrupted. Otherwise, False.
         */
        virtual void erase(uint32_t index);
        /*!
         * @brief   It removes a specific element in the object
         * @param   index Position of the element in the object to remove
         * @param   erased True if the elements was found and deleted
         * @return  True if the object is not corrupted. Otherwise, False.
         */
        virtual void erase(uint32_t index,bool& erased);
        /*!
         * @brief   It provides the amount of elements of the object
         * @return  The number of elements of size sizeof(T) in the object
         */
        std::size_t size();
        /*!
         * @brief   It indicates if there was a critical failure and the
         *          allocator was not able to recover
         * @return  True if jeopardized. Otherwise, False.
         */
        bool isJeopardized();
        /*!
         * @brief   It returns the value of the requested index
         * @param   index Index to return
         * @param   outOfBoundaries Flag to notify that the requested index
         *          is bigger than size()
         * @return  The value in index of type T
         */
        virtual T at(uint32_t index,bool& outOfBoundaries);
        /*!
         * @brief   It return the value of the requested index
         * @param   index Index to return
         * @note    This operator won't check if the value is within boundaries,
         *          so if index >= size, it's undefined behaviour. For a safe
         *          way, use at()
         */
        virtual T& operator[](uint32_t index);
    protected:
        bool internalFailure;
        uint32_t elements;
};

template <typename T>
class CrcVector: public Vector<T,CrcAllocation> {
    public:
        using Container::aMem;
        using Container::arena;
        using Vector<T,CrcAllocation>::internalFailure;
        using Vector<T,CrcAllocation>::elements;
        /*!
         * @brief   Constructor to receive just an allocator, without initialisers
         * @param   section Reference of an object of typed BasicAllocation
         *          to be used a lower layer to manage the memory
         */
        explicit CrcVector(CrcAllocation& section);
        /*!
         * @brief   Constructor when receiving an allocator and a list to
         *          initialise the cus::Vector object
         * @param   section Reference of an object of typed BasicAllocation
         *          to be used a lower layer to manage the memory
         * @param   cList object used to initialise the object
         */
        explicit CrcVector(CrcAllocation& section,std::initializer_list<T> cList);
        /*!
         * @brief   Copy constructir
         * @note    Temporally deleted (!0-3-5)
         */
        CrcVector(CrcVector<T>&);
        /*!
         * @brief   Copy constructir
         * @note    Temporally deleted (!0-3-5)
         */
        CrcVector<T>& operator=(CrcVector<T>&);

        /*!
         * @brief   Move constructir
         * @note    Temporally deleted (!0-3-5)
         */
        CrcVector(CrcVector&& vector);
        /*!
         * @brief   Copy constructir
         * @note    Temporally deleted (!0-3-5)
         */
        CrcVector<T>& operator=(CrcVector<T>&&);
        /*!
         * @brief   It allows to append an element of type T at the end of
         *          object. It copies the element, so there will be two positions
         *          of memory with the same values.
         * @param   value it's the value to be added at the end of the vector
         * @note    add values dinamically is quite expensive due to the way the
         *          allocator works.
         *          See BasicAllocation in order to know what happens when
         *          an object allows dynamic size.
         *          In order to avoid this, when the size is well-known at
         *          compilation time and fixed, cus::array should be used.
         * @return  True if the object is not corrupted. Otherwise, False.
         */
        bool push_back(T value);
        /*!
         * @brief   It allows to append elements of type T at the end of the
         *          object. It copies, so there will be different areas of
         *          memory with the same values
         * @param   toAppend cus::Vector<T> which contains the elements to append
         * @return  True if the object is not corrupted. Otherwise, False.
         */
        bool push_back(CrcVector<T>& toAppend);
        /*!
         * @brief   It assignes more space for the object. The space is based on
         *          the type T, so different bytes might be allocated for the same
         *          amount of elements fwhen different Ts.
         * @param   newElements amount of elements to increment the Vector
         * @return  True if the resize was valid. Otherwise, False.
         */
        bool resize(uint32_t newElements);
        /*!
         * @brief   It removes a specific element in the object
         * @param   index Position of the element in the object to remove
         * @return  True if the object is not corrupted. Otherwise, False.
         */
        void erase(uint32_t index);
        /*!
         * @brief   It removes a specific element in the object
         * @param   index Position of the element in the object to remove
         * @param   erased True if the elements was found and deleted
         * @return  True if the object is not corrupted. Otherwise, False.
         */
        void erase(uint32_t index,bool& erased);
        /*!
         * @brief   It returns the value of the requested index
         * @param   index Index to return
         * @param   outOfBoundaries Flag to notify that the requested index
         *          is bigger than size()
         * @return  The value in index of type T
         */
        T at(uint32_t index,bool& outOfBoundaries);

        void set(uint32_t index, bool& outOfBoundaries, T value);
};

// Valid types
template class  Vector<int16_t, BasicAllocation>;
template class  Vector<uint16_t, BasicAllocation>;
template class  Vector<unsigned char, BasicAllocation>;
template class  Vector<char, BasicAllocation>;
template class  Vector<unsigned int, BasicAllocation>;
template class  Vector<int, BasicAllocation>;
template class  Vector<unsigned long, BasicAllocation>;
template class  Vector<long, BasicAllocation>;
template class  Vector<float, BasicAllocation>;
template class  Vector<double, BasicAllocation>;
template class  Vector<int16_t, CrcAllocation>;
template class  Vector<uint16_t, CrcAllocation>;
template class  Vector<unsigned char, CrcAllocation>;
template class  Vector<char, CrcAllocation>;
template class  Vector<unsigned int, CrcAllocation>;
template class  Vector<int, CrcAllocation>;
template class  Vector<unsigned long, CrcAllocation>;
template class  Vector<long, CrcAllocation>;
template class  Vector<float, CrcAllocation>;
template class  Vector<double, CrcAllocation>;

template class  CrcVector<int16_t>;
template class  CrcVector<uint16_t>;
template class  CrcVector<unsigned char>;
template class  CrcVector<char>;
template class  CrcVector<unsigned int>;
template class  CrcVector<int>;
template class  CrcVector<unsigned long>;
template class  CrcVector<long>;
template class  CrcVector<float>;
template class  CrcVector<double>;

}; // end namespace

#endif
