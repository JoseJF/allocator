

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
        explicit Vector(const A& section) noexcept;
        /*!
         * @brief   Constructor when receiving an allocator and a list to
         *          initialise the cus::Vector object
         * @param   section Reference of an object of typed BasicAllocation
         *          to be used a lower layer to manage the memory
         * @param   cList object used to initialise the object
         */
        explicit Vector(const A& section,const std::initializer_list<T> cList) noexcept;
        /*!
         * @brief   Destructor to notify the lower layers that the memory used
         *          by the self is not needed anymore and it has to be released
         */
        virtual ~Vector() noexcept;
        /*!
         * @brief   Copy constructir
         * @note    Temporally deleted (!0-3-5)
         */
        Vector(const Vector& vector) noexcept;
        /*!
         * @brief   Copy constructir
         * @note    Temporally deleted (!0-3-5)
         */
        Vector<T,A>& operator=(const Vector<T,A>&) noexcept;

        /*!
         * @brief   Move constructir
         * @note    Temporally deleted (!0-3-5)
         */
        Vector(const Vector&& vector) noexcept;
        /*!
         * @brief   Copy constructir
         * @note    Temporally deleted (!0-3-5)
         */
        Vector<T,A>& operator=(const Vector<T,A>&&) noexcept;

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
        virtual bool push_back(const T value) noexcept;
        /*!
         * @brief   It allows to append elements of type T at the end of the
         *          object. It copies, so there will be different areas of
         *          memory with the same values
         * @param   toAppend cus::Vector<T> which contains the elements to append
         * @return  True if the object is not corrupted. Otherwise, False.
         */
        virtual bool push_back(const Vector<T,A>& toAppend) noexcept;
        /*!
         * @brief   It assignes more space for the object. The space is based on
         *          the type T, so different bytes might be allocated for the same
         *          amount of elements fwhen different Ts.
         * @param   newElements amount of elements to increment the Vector
         * @return  True if the resize was valid. Otherwise, False.
         */
        virtual bool resize(const uint32_t newElements) noexcept;
        /*!
         * @brief   It removes a specific element in the object
         * @param   index Position of the element in the object to remove
         * @return  True if the object is not corrupted. Otherwise, False.
         */
        virtual void erase(const uint32_t index) noexcept;
        /*!
         * @brief   It removes a specific element in the object
         * @param   index Position of the element in the object to remove
         * @param   erased True if the elements was found and deleted
         * @return  True if the object is not corrupted. Otherwise, False.
         */
        virtual void erase(const uint32_t index,bool& erased) noexcept;
        /*!
         * @brief   It provides the amount of elements of the object
         * @return  The number of elements of size sizeof(T) in the object
         */
        std::size_t size() const noexcept;
        /*!
         * @brief   It indicates if there was a critical failure and the
         *          allocator was not able to recover
         * @return  True if jeopardized. Otherwise, False.
         */
        bool isJeopardized() const noexcept;
        /*!
         * @brief   It returns the value of the requested index
         * @param   index Index to return
         * @param   outOfBoundaries Flag to notify that the requested index
         *          is bigger than size()
         * @return  The value in index of type T
         */
        virtual T at(const uint32_t index,bool& outOfBoundaries) noexcept;
        /*!
         * @brief   It return the value of the requested index
         * @param   index Index to return
         * @note    This operator won't check if the value is within boundaries,
         *          so if index >= size, it's undefined behaviour. For a safe
         *          way, use at()
         */
        virtual T& operator[](const uint32_t index) noexcept;

        virtual const T& operator[](const uint32_t index) const noexcept;
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
        explicit CrcVector(const CrcAllocation& section) noexcept;
        /*!
         * @brief   Constructor when receiving an allocator and a list to
         *          initialise the cus::Vector object
         * @param   section Reference of an object of typed BasicAllocation
         *          to be used a lower layer to manage the memory
         * @param   cList object used to initialise the object
         */
        explicit CrcVector(const CrcAllocation& section, \
                const std::initializer_list<T> cList) noexcept;
        /*!
         * @brief   Copy constructir
         */
        CrcVector(const CrcVector<T>&) noexcept;
        /*!
         * @brief   Copy constructir
         */
        CrcVector<T>& operator=(const CrcVector<T>&) noexcept;

        /*!
         * @brief   Move constructir
         */
        CrcVector(const CrcVector&& vector) noexcept;
        /*!
         * @brief   Copy constructir
         */
        CrcVector<T>& operator=(const CrcVector<T>&&) noexcept;
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
        bool push_back(const T value) noexcept;
        /*!
         * @brief   It allows to append elements of type T at the end of the
         *          object. It copies, so there will be different areas of
         *          memory with the same values
         * @param   toAppend cus::Vector<T> which contains the elements to append
         * @return  True if the object is not corrupted. Otherwise, False.
         */
        bool push_back(const CrcVector<T>& toAppend) noexcept;
        /*!
         * @brief   It assignes more space for the object. The space is based on
         *          the type T, so different bytes might be allocated for the same
         *          amount of elements fwhen different Ts.
         * @param   newElements amount of elements to increment the Vector
         * @return  True if the resize was valid. Otherwise, False.
         */
        bool resize(const uint32_t newElements) noexcept;
        /*!
         * @brief   It removes a specific element in the object
         * @param   index Position of the element in the object to remove
         * @return  True if the object is not corrupted. Otherwise, False.
         */
        void erase(const uint32_t index) noexcept;
        /*!
         * @brief   It removes a specific element in the object
         * @param   index Position of the element in the object to remove
         * @param   erased True if the elements was found and deleted
         * @return  True if the object is not corrupted. Otherwise, False.
         */
        void erase(const uint32_t index,bool& erased) noexcept;
        /*!
         * @brief   It returns the value of the requested index
         * @param   index Index to return
         * @param   outOfBoundaries Flag to notify that the requested index
         *          is bigger than size()
         * @return  The value in index of type T
         */
        T at(const uint32_t index,bool& outOfBoundaries) noexcept;

        void set(const uint32_t index, bool& outOfBoundaries, const T value) noexcept;
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
