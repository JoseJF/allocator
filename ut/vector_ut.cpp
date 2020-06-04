// Let Catch provide main():
#define CATCH_CONFIG_MAIN


#include "catch2/catch.hpp"
#include <allocator.hpp>
#include <vector.hpp>

const uint32_t SIZE_ARENA=500;

TEST_CASE( "Basic vector", "Create an empty object" ) {
    char arena[SIZE_ARENA];
    cus::BasicAllocation mockArena(reinterpret_cast<void *>(&arena[0]), \
                                   reinterpret_cast<void *>(&arena[SIZE_ARENA-1]));

    cus::Vector<uint8_t> vector1(mockArena);
    vector1.push_back(uint8_t(1));

    REQUIRE( vector1[0] == 1 );
    REQUIRE( vector1.size() == 1 );
    bool oob=true;
    REQUIRE( vector1.at(0,oob) == 1 );
    REQUIRE( oob == false );
}

TEST_CASE( "Initialize the vector", "Construct an initialized object" ) {
    char arena[SIZE_ARENA];
    cus::BasicAllocation mockArena(reinterpret_cast<void *>(&arena[0]), \
                                   reinterpret_cast<void *>(&arena[SIZE_ARENA-1]));

    std::initializer_list<uint8_t> initList{0,1,2,3,4,5,6,7,8,9};
    cus::Vector<uint8_t> vector1(mockArena,initList);

    for(uint8_t i=0;i<initList.size();i++) {
        REQUIRE( vector1[i] == i );
    }
    REQUIRE( vector1.size() == initList.size() );
}

TEST_CASE( "Used space", "A vector can use the whole arena minus metadata" ) {
    char arena[SIZE_ARENA];
    cus::BasicAllocation mockArena(reinterpret_cast<void *>(&arena[0]), \
                                   reinterpret_cast<void *>(&arena[SIZE_ARENA-1]));

    cus::Vector<uint8_t> vector(mockArena);

    uint32_t i=0;
    while(true) {
        bool failure = vector.push_back(i);
        if(failure == true) {
            REQUIRE( vector.isJeopardized() == true );
            break;
        }
        i++;
    }
    REQUIRE( i == ((SIZE_ARENA)-3*sizeof(arch_t)) );
}

TEST_CASE( "Destructor", "A vector releases the used memory" ) {
    char arena[SIZE_ARENA];
    cus::BasicAllocation mockArena(reinterpret_cast<void *>(&arena[0]), \
                                   reinterpret_cast<void *>(&arena[SIZE_ARENA-1]));

    uint8_t * firstVector;
    uint8_t * secondVector;
    {
        const uint8_t mockValue = 0x05;
        cus::Vector<uint8_t> vector(mockArena);
        vector.push_back(uint8_t(mockValue));
        vector.push_back(uint8_t(mockValue+1));
        vector.push_back(uint8_t(mockValue+2));
        firstVector=(uint8_t *)&vector[0];
        REQUIRE( vector[0] == mockValue );
        REQUIRE( vector.isJeopardized() == false );
    }

    {
        const uint8_t mockValue = 0xA0;
        cus::Vector<uint8_t> vector(mockArena);
        vector.push_back(uint8_t(mockValue));
        vector.push_back(uint8_t(mockValue+1));
        vector.push_back(uint8_t(mockValue+2));
        secondVector=(uint8_t *)&vector[0];
        REQUIRE( vector[0] == mockValue );
        REQUIRE( vector.isJeopardized() == false );
    }

    REQUIRE( firstVector == secondVector );
    REQUIRE( reinterpret_cast<arch_t>(firstVector) >= \
              reinterpret_cast<arch_t>(&arena[0]) );
    REQUIRE( reinterpret_cast<arch_t>(firstVector) <= \
              reinterpret_cast<arch_t>(&arena[SIZE_ARENA-1]) );
}


TEST_CASE( "Append to another vector", "When append, both objects are available" ) {
    char arena[SIZE_ARENA];
    cus::BasicAllocation mockArena(reinterpret_cast<void *>(&arena[0]), \
                                   reinterpret_cast<void *>(&arena[SIZE_ARENA-1]));

    uint8_t * firstVector;
    uint8_t * secondVector;

    cus::Vector<uint8_t> vectorA(mockArena);
    cus::Vector<uint8_t> vectorB(mockArena);
    cus::Vector<uint8_t> vectorTemp(mockArena);

    for(uint8_t i=0;i<10;i++) {
        vectorA.push_back(uint8_t(i));
    }

    for(uint8_t i=10;i<20;i++) {
        vectorB.push_back(uint8_t(i));
    }

    //vectorTemp.push_back(vectorA);
    //vectorA.push_back(vectorB);
    //vectorB.push_back(vectorTemp);

    std::cout << "vectorA" << std::endl;
    for(uint8_t i=0;i<vectorA.size();i++) {
        std::cout << (uint32_t)vectorA[i] << std::endl;
    }

    std::cout << "vectorB" << std::endl;
    for(uint8_t i=0;i<vectorB.size();i++) {
        std::cout << (uint32_t)vectorB[i] << std::endl;
    }

    std::cout << "vectorTemp" << std::endl;
    for(uint8_t i=0;i<vectorTemp.size();i++) {
        std::cout << (uint32_t)vectorTemp[i] << std::endl;
    }

    for(uint8_t i=0;i<20;i++) {
        if(i<10) {
            REQUIRE( (uint32_t)vectorA[i] == (uint32_t)i );
            REQUIRE( (uint32_t)vectorB[i] == (uint32_t)10+i );
        } else {
            REQUIRE( (uint32_t)vectorA[i] == (uint32_t)i );
            REQUIRE( (uint32_t)vectorB[i] == (uint32_t)(i-10) );
        }
    }
}


