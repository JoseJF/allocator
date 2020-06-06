// Let Catch provide main():
#define CATCH_CONFIG_MAIN


#include "catch2/catch.hpp"
#include <allocator.hpp>
#include <vector.hpp>

const uint32_t SIZE_ARENA=500;
const uint32_t END_ARENA=500;

TEST_CASE( "Basic vector", "Create an empty object" ) {
    char arena[SIZE_ARENA];
    cus::BasicAllocation mockArena(reinterpret_cast<void *>(&arena[0]), \
                                   reinterpret_cast<void *>(&arena[END_ARENA]));

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
                                   reinterpret_cast<void *>(&arena[END_ARENA]));

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
                                   reinterpret_cast<void *>(&arena[END_ARENA]));

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
                                   reinterpret_cast<void *>(&arena[END_ARENA]));

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
                                   reinterpret_cast<void *>(&arena[END_ARENA]));

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

    vectorTemp.push_back(vectorA);
    vectorA.push_back(vectorB);
    vectorB.push_back(vectorTemp);

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

TEST_CASE( "Different types", "Different types of vectors can be created" ) {
    char arena[SIZE_ARENA];
    cus::BasicAllocation mockArena(reinterpret_cast<void *>(&arena[0]), \
                                   reinterpret_cast<void *>(&arena[END_ARENA]));

    uint8_t * firstVector;
    uint8_t * secondVector;

    cus::Vector<uint8_t> vectorA(mockArena);
    cus::Vector<uint16_t> vectorB(mockArena);
    cus::Vector<uint32_t> vectorC(mockArena);
    cus::Vector<uint8_t> vectorD(mockArena);
    cus::Vector<uint16_t> vectorE(mockArena);
    cus::Vector<uint32_t> vectorF(mockArena);

    {
        uint32_t i=0;
        bool failure=false;
        while(failure==false) {
            failure |= vectorA.push_back(uint8_t(i));
            failure |= vectorB.push_back(uint16_t(i+1));
            failure |= vectorC.push_back(uint32_t(i+2));
            failure |= vectorD.push_back(uint8_t(i+3));
            failure |= vectorE.push_back(uint16_t(i+4));
            failure |= vectorF.push_back(uint32_t(i+5));
            i++;
        }
        vectorB.~Vector();
        vectorF.~Vector();
        vectorA.~Vector();
    }
    for(uint32_t i = 0;i<vectorD.size();i++) {
        REQUIRE( vectorD[i] == (uint8_t)i+3);
    }
    for(uint32_t i = 0;i<vectorC.size();i++) {
        REQUIRE( vectorC[i] == (uint32_t)i+2);
    }
    for(uint32_t i = 0;i<vectorE.size();i++) {
        REQUIRE( vectorE[i] == (uint16_t)i+4);
    }

    uint32_t used = vectorD.size()*sizeof(uint8_t) + vectorC.size()*sizeof(uint32_t) + \
                    vectorE.size()*sizeof(uint16_t);
    used+=3*(3*sizeof(arch_t));
    uint32_t free = SIZE_ARENA - used;

    cus::Vector<uint8_t> vectorG(mockArena);
    free-=(3*sizeof(arch_t));
    for(uint32_t i=0;i<free;i++) {
        REQUIRE( vectorG.push_back(uint8_t(i)) == false);
    }
    REQUIRE( vectorG.push_back(uint8_t(0xA5)) == true);
}



TEST_CASE( "Erase", "Erase elements" ) {
    char arena[SIZE_ARENA];
    cus::BasicAllocation mockArena(reinterpret_cast<void *>(&arena[0]), \
                                   reinterpret_cast<void *>(&arena[END_ARENA]));

    {
        const uint8_t elements=10;
        cus::Vector<uint8_t> vectorA(mockArena);
        cus::Vector<uint16_t> vectorB(mockArena);
        cus::Vector<uint32_t> vectorC(mockArena);
        {
            uint32_t i=0;
            while(i<elements) {
                vectorA.push_back(uint8_t(i));
                vectorB.push_back(uint16_t(i+1));
                vectorC.push_back(uint32_t(i+2));
                i++;
            }

            for(uint8_t j=0;j<elements-1;j++) {
                vectorB.erase(0);
            }
            REQUIRE( vectorB.size() == 1);
            REQUIRE( vectorB[0] == 10);

            for(uint8_t j=0;j<elements-1;j++) {
                vectorA.erase(0);
            }
            REQUIRE( vectorA.size() == 1);
            REQUIRE( vectorA[0] == 9);

            for(uint8_t j=0;j<elements-1;j++) {
                vectorC.erase(0);
            }
            REQUIRE( vectorC.size() == 1);
            REQUIRE( vectorC[0] == 11);
        }
    }

    // Confirm that destructor wipes the arena
    cus::Vector<uint8_t> vectorA(mockArena);
    for(uint32_t free=0;free<(SIZE_ARENA-3*sizeof(arch_t));free++) {
        REQUIRE( vectorA.push_back((uint8_t)0) == false);
    }
    REQUIRE( vectorA.push_back((uint8_t)0) == true);
}
