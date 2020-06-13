// Let Catch provide main():
#define CATCH_CONFIG_MAIN


#include "catch2/catch.hpp"
#include <allocator.hpp>

const uint32_t SIZE_ARENA=500;
const uint32_t END_ARENA=500;

TEST_CASE( "Basic allocation", \
        "first pointer is located at the beggining of the arena +sizeof(arch_t)" ) {

    char arena[SIZE_ARENA];
    cus::BasicAllocation mockArena(reinterpret_cast<void *>(&arena[0]), \
                                   reinterpret_cast<void *>(&arena[END_ARENA]));

    void * mockRequester;
    const std::size_t sizeB_mockRequester=4;
    bool valid = mockArena.allocate((arch_t)&mockRequester,mockRequester, sizeB_mockRequester);
    REQUIRE( reinterpret_cast<arch_t>(mockRequester) == \
            (reinterpret_cast<arch_t>(&arena[0]) ) );
}

TEST_CASE( "Multiple allocations", \
        "All the new allocations will use the next free memory" ) {
    char arena[SIZE_ARENA];
    cus::BasicAllocation mockArena(reinterpret_cast<void *>(&arena[0]), \
                                   reinterpret_cast<void *>(&arena[END_ARENA]));

    void * mockRequester_a;
    const std::size_t sizeB_mockRequester_a=4;
    void * mockRequester_b;
    const std::size_t sizeB_mockRequester_b=16;

    bool valid_a=mockArena.allocate((arch_t)&mockRequester_a,mockRequester_a,sizeB_mockRequester_a);
    bool valid_b=mockArena.allocate((arch_t)&mockRequester_b,mockRequester_b,sizeB_mockRequester_b);

    REQUIRE( reinterpret_cast<arch_t>(mockRequester_a) == \
             (reinterpret_cast<arch_t>(&arena[0]) ) );
    REQUIRE( reinterpret_cast<arch_t>(mockRequester_b) == \
             (reinterpret_cast<arch_t>(mockRequester_a) + sizeB_mockRequester_a) );
    REQUIRE( valid_a == true );
    REQUIRE( valid_b == true );
}

TEST_CASE( "Max. allocated size", \
        "sizeArena >= (allocations*size) + \
        (allocations * 3 * sizeof(arch_t)) )" ) {
    char arena[SIZE_ARENA];
    cus::BasicAllocation mockArena(reinterpret_cast<void *>(&arena[0]), \
                                   reinterpret_cast<void *>(&arena[END_ARENA]));
    uint32_t elements=0;
    const std::size_t sizeB_mockRequester=4;
    uint32_t maxAllocations=((SIZE_ARENA / (sizeB_mockRequester+3*sizeof(arch_t)) ));
    void * mockRequester[maxAllocations];
    while(elements < maxAllocations) {
        bool valid=mockArena.allocate((arch_t)&mockRequester[elements],\
                mockRequester[elements],sizeB_mockRequester);
        REQUIRE( valid == true );
        elements++;
    }
    void * mockRequester2;
    bool valid=mockArena.allocate((arch_t)&mockRequester2,mockRequester2,sizeB_mockRequester);
    REQUIRE( valid == false );
}

TEST_CASE( "Arena limits the operations", \
        "The arena limits all the operations" ) {
    char arena[SIZE_ARENA*2];
    cus::BasicAllocation mockArena(reinterpret_cast<void *>(&arena[1]), \
                                   reinterpret_cast<void *>(&arena[END_ARENA+1]));
    arena[SIZE_ARENA+1]=0xA5;
    arena[0]=0xA5;

    uint32_t elements=0;
    const std::size_t sizeB_mockRequester=4;
    uint32_t maxAllocations=((SIZE_ARENA)/ (sizeB_mockRequester+(3*sizeof(arch_t))));
    void * mockRequester[maxAllocations];
    while(elements < maxAllocations) {

        bool valid=mockArena.allocate((arch_t)&mockRequester[elements],\
                mockRequester[elements],sizeB_mockRequester);
        *((char *)mockRequester + elements)=0x5A;
        REQUIRE( valid == true );
        elements++;
    }
    void * mockRequester2;
    bool valid=mockArena.allocate((arch_t)&mockRequester2,mockRequester2,sizeB_mockRequester);
    REQUIRE( valid == false );
    REQUIRE( ((((uint32_t)arena[SIZE_ARENA+1])<<24)>>24) == (uint32_t)0xA5 );
    REQUIRE( ((((uint32_t)arena[0])<<24)>>24) == 0xA5 );
}


TEST_CASE( "Clashes passing objects", "the same object cannot allocate two times" ) {
    char arena[SIZE_ARENA];
    cus::BasicAllocation mockArena(reinterpret_cast<void *>(&arena[0]), \
                                   reinterpret_cast<void *>(&arena[END_ARENA]));

    void * mockRequester=nullptr;
    const std::size_t sizeB_mockRequester=4;

    bool valid_1 = mockArena.allocate((arch_t)&mockRequester,mockRequester,sizeB_mockRequester);
    void * tempMockRequester = mockRequester;
    bool valid_2 = mockArena.allocate((arch_t)&mockRequester,mockRequester,sizeB_mockRequester);

    REQUIRE(valid_1 == true );
    REQUIRE(valid_2 == false );
}


// check when sections
TEST_CASE( "Clashed during allocation", "two arenas cannot share space" ) {
    char arena[SIZE_ARENA];
    cus::BasicAllocation mockArena_a(reinterpret_cast<void *>(&arena[0]), \
                                   reinterpret_cast<void *>(&arena[END_ARENA]));
    cus::BasicAllocation mockArena_b(reinterpret_cast<void *>(&arena[0]), \
                                   reinterpret_cast<void *>(&arena[END_ARENA]));

    void * mockRequester_a;
    void * mockRequester_b;

    bool valid_a = mockArena_a.allocate((arch_t)&mockRequester_a,mockRequester_a, \
            (std::size_t)4);
    bool valid_b = mockArena_b.allocate((arch_t)&mockRequester_b,mockRequester_b, \
            (std::size_t)4);

    REQUIRE(reinterpret_cast<arch_t>(mockRequester_a) != reinterpret_cast<arch_t>(mockRequester_b));
}


TEST_CASE( "Basic reallocation", "Element is resized if there is enough space") {
    char arena[SIZE_ARENA];
    cus::BasicAllocation mockArena(reinterpret_cast<void *>(&arena[0]), \
                                   reinterpret_cast<void *>(&arena[END_ARENA]));

    void * mockRequester;
    const std::size_t sizeB_mockRequester=4;

    bool validAlloc = mockArena.allocate((arch_t)&mockRequester,mockRequester,\
            sizeB_mockRequester);
    REQUIRE( validAlloc == true );

    bool validRealloc=mockArena.reallocate(mockRequester,sizeB_mockRequester,\
            sizeB_mockRequester*2);
    REQUIRE( validRealloc == true );
}

TEST_CASE( "Multiple reallocations", "Next elements have to be reorganized") {
    char arena[SIZE_ARENA];
    cus::BasicAllocation mockArena(reinterpret_cast<void *>(&arena[0]), \
                                   reinterpret_cast<void *>(&arena[END_ARENA-1]));

    void * mockRequester_a = nullptr;
    void * mockRequester_b = nullptr;
    void * mockRequester_b_prev_reorg = nullptr;
    const std::size_t sizeB_mockRequester_a=4;
    const std::size_t sizeB_mockRequester_a_new=16;
    const std::size_t sizeB_mockRequester_b=4;

    bool validAlloc_a = mockArena.allocate((arch_t)&mockRequester_a,mockRequester_a,\
            sizeB_mockRequester_a);
    REQUIRE( validAlloc_a == true );

    bool validAlloc_b = mockArena.allocate((arch_t)&mockRequester_b,mockRequester_b,\
            sizeB_mockRequester_b);
    REQUIRE( validAlloc_b == true );
    mockRequester_b_prev_reorg = mockRequester_b;

    bool validRealloc=mockArena.reallocate(mockRequester_a,sizeB_mockRequester_a,\
            sizeB_mockRequester_a_new);
    REQUIRE( validRealloc == true );

    REQUIRE( reinterpret_cast<arch_t>(mockRequester_b) == \
            (reinterpret_cast<arch_t>(mockRequester_b_prev_reorg) + \
             (sizeB_mockRequester_a_new-sizeB_mockRequester_a)) );
}

TEST_CASE( "Max reallocated size", \
        "sizeArena >= (allocations*size) + (3 * sizeof(arch_t) )") {
    char arena[SIZE_ARENA];
    cus::BasicAllocation mockArena(reinterpret_cast<void *>(&arena[0]), \
                                   reinterpret_cast<void *>(&arena[END_ARENA]));

    uint32_t elements=0;
    std::size_t sizeB_mockRequester=1;
    std::size_t sizeB_mockRequester_next=sizeB_mockRequester+sizeB_mockRequester;
    uint32_t maxAllocations=((SIZE_ARENA)-(3*sizeof(arch_t)))/sizeB_mockRequester;
    void * mockRequester;
    bool valid=mockArena.allocate((arch_t)&mockRequester,mockRequester, \
            sizeB_mockRequester);
    elements++;
    REQUIRE( valid == true );

    while(true) {
        bool validRealloc=mockArena.reallocate(mockRequester,sizeB_mockRequester,\
            sizeB_mockRequester_next);
        sizeB_mockRequester=sizeB_mockRequester_next;
        sizeB_mockRequester_next+=1;

        elements++;
        if(elements<=maxAllocations) {
            REQUIRE( validRealloc == true );
        } else {
            REQUIRE( validRealloc == false );
            break;
        }
    }

}

TEST_CASE( "Max reallocated from one object", \
        "One reallocation cannot be bigger than arena") {
    char arena[SIZE_ARENA];
    cus::BasicAllocation mockArena(reinterpret_cast<void *>(&arena[0]), \
                                   reinterpret_cast<void *>(&arena[END_ARENA]));

    uint32_t elements=0;
    std::size_t sizeB_mockRequester=4;
    //uint32_t freeDataSpace = (SIZE_ARENA/2) - ((2*sizeof(arch_t) + (3*sizeof(arch_t))));
    uint32_t freeDataSpace = (SIZE_ARENA) - ((sizeof(arch_t) + (3*sizeof(arch_t))));

    {
        void * mockRequester;
        bool valid=mockArena.allocate((arch_t)&mockRequester,mockRequester,\
                sizeB_mockRequester);
        REQUIRE( valid == true );
        bool validRealloc=mockArena.reallocate(mockRequester,sizeB_mockRequester,\
                freeDataSpace);
        REQUIRE( validRealloc == true );
    }
}

TEST_CASE( "Reject reallocations", \
        "A rejected reallocation doesn't change the current situation") {
    char arena[SIZE_ARENA];
    cus::BasicAllocation mockArena(reinterpret_cast<void *>(&arena[0]), \
                                   reinterpret_cast<void *>(&arena[END_ARENA]));

    uint32_t elements=0;
    std::size_t sizeB_mockRequester=4;
    uint32_t freeDataSpace = (SIZE_ARENA) - (3*sizeof(arch_t));

    {
        void * mockRequester;
        bool valid=mockArena.allocate((arch_t)&mockRequester,mockRequester,\
                sizeB_mockRequester);
        REQUIRE( valid == true );

        bool validRealloc=mockArena.reallocate(mockRequester,sizeB_mockRequester,\
                freeDataSpace+1);
        REQUIRE( validRealloc == false );

        bool validNewAlloc=mockArena.reallocate(mockRequester,sizeB_mockRequester,\
                freeDataSpace-1);
        REQUIRE( validNewAlloc == true );
    }
}

TEST_CASE( "Basic deallocation", \
        "The entire object must be easily deallocated") {
    char arena[SIZE_ARENA];
    cus::BasicAllocation mockArena(reinterpret_cast<void *>(&arena[0]), \
                                   reinterpret_cast<void *>(&arena[END_ARENA]));

    uint32_t elements=0;
    //uint32_t freeDataSpace = (SIZE_ARENA/2) - ((2*sizeof(arch_t) + (3*sizeof(arch_t))));
    uint32_t freeDataSpace = (SIZE_ARENA) - ((sizeof(arch_t) + (3*sizeof(arch_t))));

    {
        void * mockRequester;
        bool valid=mockArena.allocate((arch_t)&mockRequester,mockRequester,\
                freeDataSpace-1);
        REQUIRE( valid == true );

        void * secondMockRequester;
        bool validSecondAlloc=mockArena.allocate((arch_t)&secondMockRequester, \
                secondMockRequester, freeDataSpace-1);
        REQUIRE( validSecondAlloc == false );

        bool validDealloc=mockArena.deallocate((arch_t)&mockRequester);
        REQUIRE( validDealloc == true );

        bool validSecondAlloc_retry=mockArena.allocate((arch_t)&secondMockRequester, \
                secondMockRequester, freeDataSpace-1);
        REQUIRE( validSecondAlloc_retry == true );
    }
}

TEST_CASE( "Multiple deallocation", \
        "After consecutive allocations and deallocations, available memory has to \
        be the same than before allocations deallocations") {
    char arena[SIZE_ARENA];
    cus::BasicAllocation mockArena(reinterpret_cast<void *>(&arena[0]), \
                                   reinterpret_cast<void *>(&arena[END_ARENA]));

    uint32_t elements=0;
    //uint32_t freeDataSpace = (SIZE_ARENA/2) - ((2*sizeof(arch_t) + (3*sizeof(arch_t))));
    uint32_t freeDataSpace = (SIZE_ARENA) - ((sizeof(arch_t) + (3*sizeof(arch_t))));

    void * mockRequester_a;
    void * mockRequester_b;
    void * mockRequester_c;
    void * mockRequester;
    uint32_t sizeElement = freeDataSpace/4;
    {
        for(uint32_t rewrites=0;rewrites<1000;rewrites++) {
            (void)mockArena.allocate((arch_t)&mockRequester_a, \
                    mockRequester_a, sizeElement);
            //REQUIRE( reinterpret_cast<arch_t>(mockRequester_a) == \
            //        (reinterpret_cast<arch_t>(&arena[0]) + sizeof(arch_t)) );
            REQUIRE( reinterpret_cast<arch_t>(mockRequester_a) == \
                    (reinterpret_cast<arch_t>(&arena[0]) ) );

            (void)mockArena.allocate((arch_t)&mockRequester_b, \
                    mockRequester_b, sizeElement);
            REQUIRE( reinterpret_cast<arch_t>(mockRequester_b) == \
                    (reinterpret_cast<arch_t>(mockRequester_a) +
                     sizeElement) );

            (void)mockArena.allocate((arch_t)&mockRequester_c, \
                    mockRequester_c, sizeElement);
            REQUIRE( reinterpret_cast<arch_t>(mockRequester_c) == \
                    ( reinterpret_cast<arch_t>(mockRequester_b) +
                     sizeElement) );

            (void)mockArena.deallocate((arch_t)&mockRequester_a);
            (void)mockArena.deallocate((arch_t)&mockRequester_b);
            (void)mockArena.deallocate((arch_t)&mockRequester_c);

            bool validAlloc = mockArena.allocate((arch_t)&mockRequester, \
                    mockRequester, sizeElement);
            REQUIRE( validAlloc == true );
            //REQUIRE( reinterpret_cast<arch_t>(mockRequester_a) == \
                    (reinterpret_cast<arch_t>(&arena[0]) + sizeof(arch_t)) );
            REQUIRE( reinterpret_cast<arch_t>(mockRequester_a) == \
                    (reinterpret_cast<arch_t>(&arena[0]) ) );

            bool validDealloc=mockArena.deallocate((arch_t)&mockRequester);
            REQUIRE( validDealloc == true );
        }
    }
}


TEST_CASE( "Invalid deallocation", \
        "Invalid deallocations does not affect the system") {
    char arena[SIZE_ARENA];
    cus::BasicAllocation mockArena(reinterpret_cast<void *>(&arena[0]), \
                                   reinterpret_cast<void *>(&arena[END_ARENA]));

    uint32_t elements=0;
    //uint32_t freeDataSpace = (SIZE_ARENA/2) - ((2*sizeof(arch_t) + (3*sizeof(arch_t))));
    uint32_t freeDataSpace = (SIZE_ARENA) - ((sizeof(arch_t) + (3*sizeof(arch_t))));

    void * mockRequester_a;
    void * mockRequester_b;
    void * mockRequester_c;
    void * mockRequester_invalid;
    void * mockRequester;
    uint32_t sizeElement = freeDataSpace/4;
    {
        for(uint32_t rewrites=0;rewrites<1000;rewrites++) {
            (void)mockArena.allocate((arch_t)&mockRequester_a, \
                    mockRequester_a, sizeElement);
            (void)mockArena.deallocate((arch_t)&mockRequester_invalid);
            //REQUIRE( reinterpret_cast<arch_t>(mockRequester_a) == \
                    (reinterpret_cast<arch_t>(&arena[0]) + sizeof(arch_t)) );
            REQUIRE( reinterpret_cast<arch_t>(mockRequester_a) == \
                    (reinterpret_cast<arch_t>(&arena[0]) ) );

            (void)mockArena.allocate((arch_t)&mockRequester_b, \
                    mockRequester_b, sizeElement);
            (void)mockArena.deallocate((arch_t)&mockRequester_invalid);
            REQUIRE( reinterpret_cast<arch_t>(mockRequester_b) == \
                    (reinterpret_cast<arch_t>(mockRequester_a) +
                     sizeElement) );

            (void)mockArena.allocate((arch_t)&mockRequester_c, \
                    mockRequester_c, sizeElement);
            (void)mockArena.deallocate((arch_t)&mockRequester_invalid);
            REQUIRE( reinterpret_cast<arch_t>(mockRequester_c) == \
                    ( reinterpret_cast<arch_t>(mockRequester_b) +
                     sizeElement) );

            (void)mockArena.deallocate((arch_t)&mockRequester_invalid);
            (void)mockArena.deallocate((arch_t)&mockRequester_a);
            (void)mockArena.deallocate((arch_t)&mockRequester_b);
            (void)mockArena.deallocate((arch_t)&mockRequester_c);

            bool validAlloc = mockArena.allocate((arch_t)&mockRequester, \
                    mockRequester, sizeElement);
            REQUIRE( validAlloc == true );
            //REQUIRE( reinterpret_cast<arch_t>(mockRequester_a) == \
                    (reinterpret_cast<arch_t>(&arena[0]) + sizeof(arch_t)) );
            REQUIRE( reinterpret_cast<arch_t>(mockRequester_a) == \
                    (reinterpret_cast<arch_t>(&arena[0]) ) );

            bool validDealloc=mockArena.deallocate((arch_t)&mockRequester);
            REQUIRE( validDealloc == true );
        }
    }
}


TEST_CASE( "Basic remove elements", \
           "Subparts of an object can be easily removed") {
    char arena[SIZE_ARENA];
    cus::BasicAllocation mockArena(reinterpret_cast<void *>(&arena[0]), \
                                   reinterpret_cast<void *>(&arena[END_ARENA]));

    const std::size_t sizeB_mockRequester=16;
    const std::size_t sizeB_mockRequester_toRemove=10;

    {
        void * mockRequester;
        bool valid=mockArena.allocate((arch_t)&mockRequester,mockRequester, \
                sizeB_mockRequester);
        REQUIRE( valid == true );

        bool validRemove=mockArena.removeElement((arch_t)&mockRequester, \
                reinterpret_cast<void *>((char *)mockRequester + \
                        (sizeB_mockRequester-sizeB_mockRequester_toRemove)),\
                        sizeB_mockRequester_toRemove);
        REQUIRE( validRemove == true );
    }
}

TEST_CASE( "Remove elements and restore", \
           "All the removed elements will reorganize the next ones") {
    char arena[SIZE_ARENA];
    cus::BasicAllocation mockArena(reinterpret_cast<void *>(&arena[0]), \
                                   reinterpret_cast<void *>(&arena[END_ARENA]));

    const std::size_t sizeB_mockRequester=16;
    const std::size_t sizeB_mockRequester_toRemove=10;

    {
        void * mockRequester;
        bool valid=mockArena.allocate((arch_t)&mockRequester,mockRequester, \
                sizeB_mockRequester);
        REQUIRE( valid == true );
        REQUIRE( mockArena.elements() == 1);

        void * mockRequester_temp;
        bool validTemp=mockArena.allocate((arch_t)&mockRequester_temp, \
                mockRequester_temp, sizeB_mockRequester);
        REQUIRE( reinterpret_cast<arch_t>(mockRequester_temp) == \
                reinterpret_cast<arch_t>(mockRequester) + sizeB_mockRequester );
        REQUIRE( mockArena.elements() == 2);

        bool validRemove=mockArena.removeElement((arch_t)&mockRequester, \
                reinterpret_cast<void *>((char *)mockRequester + \
                        (sizeB_mockRequester-sizeB_mockRequester_toRemove)),\
                        sizeB_mockRequester_toRemove);
        REQUIRE( validRemove == true );
        REQUIRE( mockArena.elements() == 2);

        REQUIRE( reinterpret_cast<arch_t>(mockRequester_temp) == \
                reinterpret_cast<arch_t>(mockRequester) + \
                (sizeB_mockRequester - sizeB_mockRequester_toRemove) );
    }
}


TEST_CASE( "Remove the whole element", \
        "When the size of the element is removed, the element is deallocated") {
    char arena[SIZE_ARENA];
    cus::BasicAllocation mockArena(reinterpret_cast<void *>(&arena[0]), \
                                   reinterpret_cast<void *>(&arena[END_ARENA]));

    uint32_t elements=0;
    //uint32_t freeDataSpace = (SIZE_ARENA/2) - ((2*sizeof(arch_t) + (3*sizeof(arch_t))));
    uint32_t freeDataSpace = (SIZE_ARENA) - ((sizeof(arch_t) + (3*sizeof(arch_t))));

    void * mockRequester_a;
    void * mockRequester_b;
    void * mockRequester_c;
    void * mockRequester;
    uint32_t sizeElement = freeDataSpace/4;
    {
        for(uint32_t rewrites=0;rewrites<1000;rewrites++) {
            (void)mockArena.allocate((arch_t)&mockRequester_a, \
                    mockRequester_a, sizeElement);
            //REQUIRE( reinterpret_cast<arch_t>(mockRequester_a) == \
                    (reinterpret_cast<arch_t>(&arena[0]) + sizeof(arch_t)) );
            REQUIRE( reinterpret_cast<arch_t>(mockRequester_a) == \
                    (reinterpret_cast<arch_t>(&arena[0]) ) );
            REQUIRE( mockArena.elements() == 1);

            (void)mockArena.allocate((arch_t)&mockRequester_b, \
                    mockRequester_b, sizeElement);
            REQUIRE( reinterpret_cast<arch_t>(mockRequester_b) == \
                    (reinterpret_cast<arch_t>(mockRequester_a) +
                     sizeElement) );
            REQUIRE( mockArena.elements() == 2);

            (void)mockArena.allocate((arch_t)&mockRequester_c, \
                    mockRequester_c, sizeElement);
            REQUIRE( reinterpret_cast<arch_t>(mockRequester_c) == \
                    ( reinterpret_cast<arch_t>(mockRequester_b) +
                     sizeElement) );
            REQUIRE( mockArena.elements() == 3);

            (void)mockArena.removeElement((arch_t)&mockRequester_a, \
                                          mockRequester_a, \
                                          sizeElement);
            REQUIRE( mockArena.elements() == 2);

            (void)mockArena.removeElement((arch_t)&mockRequester_b, \
                                          mockRequester_b, \
                                          sizeElement);
            REQUIRE( mockArena.elements() == 1);

            (void)mockArena.removeElement((arch_t)&mockRequester_c,\
                                          mockRequester_c, \
                                          sizeElement);
            REQUIRE( mockArena.elements() == 0);

            bool validAlloc = mockArena.allocate((arch_t)&mockRequester, \
                    mockRequester, sizeElement);
            REQUIRE( validAlloc == true );
            //REQUIRE( reinterpret_cast<arch_t>(mockRequester_a) == \
                    (reinterpret_cast<arch_t>(&arena[0]) + sizeof(arch_t)) );
            REQUIRE( reinterpret_cast<arch_t>(mockRequester_a) == \
                    (reinterpret_cast<arch_t>(&arena[0]) ) );
            REQUIRE( mockArena.elements() == 1);

            bool validDealloc=mockArena.removeElement((arch_t)&mockRequester, \
                                                      mockRequester,
                                                      sizeElement);
            REQUIRE( mockArena.elements() == 0);
            REQUIRE( validDealloc == true );
        }
    }
}








// CrcAllocation
//


TEST_CASE( "Basic CrcAllocation", \
        "first pointer is located at the beggining of the arena +sizeof(arch_t)" ) {
    char arena[SIZE_ARENA];
    cus::CrcAllocation mockArena(reinterpret_cast<void *>(&arena[0]), \
                                   reinterpret_cast<void *>(&arena[END_ARENA]));

    void * mockRequester;
    const std::size_t sizeB_mockRequester=4;
    bool valid = mockArena.allocate((arch_t)&mockRequester,mockRequester, sizeB_mockRequester);
    REQUIRE( reinterpret_cast<arch_t>(mockRequester) == \
            (reinterpret_cast<arch_t>(&arena[0]) + sizeof(arch_t)) );
}

TEST_CASE( "Multiple CrcAllocations", \
        "All the new allocations will use the next free memory" ) {
    char arena[SIZE_ARENA];
    cus::CrcAllocation mockArena(reinterpret_cast<void *>(&arena[0]), \
                                   reinterpret_cast<void *>(&arena[END_ARENA]));

    void * mockRequester_a;
    const std::size_t sizeB_mockRequester_a=4;
    void * mockRequester_b;
    const std::size_t sizeB_mockRequester_b=16;

    bool valid_a=mockArena.allocate((arch_t)&mockRequester_a,mockRequester_a,sizeB_mockRequester_a);
    bool valid_b=mockArena.allocate((arch_t)&mockRequester_b,mockRequester_b,sizeB_mockRequester_b);

    REQUIRE( reinterpret_cast<arch_t>(mockRequester_a) == \
             (reinterpret_cast<arch_t>(&arena[0]) + sizeof(arch_t)) );
    REQUIRE( reinterpret_cast<arch_t>(mockRequester_b) == \
             (reinterpret_cast<arch_t>(mockRequester_a) + sizeB_mockRequester_a) );
    REQUIRE( valid_a == true );
    REQUIRE( valid_b == true );
}

TEST_CASE( "Max. CrcAllocates size", \
        "sizeArena/2 >= (allocations*size) + \
        (allocations * 3 * sizeof(arch_t)) + (sizeof(arch_t))" ) {
    char arena[SIZE_ARENA];
    cus::CrcAllocation mockArena(reinterpret_cast<void *>(&arena[0]), \
                                   reinterpret_cast<void *>(&arena[END_ARENA]));

    uint32_t elements=0;
    const std::size_t sizeB_mockRequester=4;
    uint32_t maxAllocations=((SIZE_ARENA/2)-(sizeof(arch_t))) / \
            (sizeB_mockRequester+(3*sizeof(arch_t)));
    void * mockRequester[maxAllocations];
    while(true) {
        bool valid=mockArena.allocate((arch_t)&mockRequester[elements], \
                mockRequester[elements],sizeB_mockRequester);

        if(elements<maxAllocations) {
            REQUIRE( valid == true );
        } else {
            REQUIRE( valid == false );
            break;
        }
        elements++;
    }
}


TEST_CASE( "Max reallocated size when CrcAllocation", \
        "sizeArena/2 >= (allocations*size) + (3 * sizeof(arch_t) + (sizeof(arch_t)))") {
    char arena[SIZE_ARENA];
    cus::CrcAllocation mockArena(reinterpret_cast<void *>(&arena[0]), \
                                   reinterpret_cast<void *>(&arena[END_ARENA]));

    uint32_t elements=0;
    std::size_t sizeB_mockRequester=4;
    std::size_t sizeB_mockRequester_next=sizeB_mockRequester+sizeB_mockRequester;
    uint32_t maxAllocations=((SIZE_ARENA/2)-((3*sizeof(arch_t))+sizeof(arch_t)))/sizeB_mockRequester;

    void * mockRequester;
    bool valid=mockArena.allocate((arch_t)&mockRequester,mockRequester,sizeB_mockRequester);
    elements+=2;
    REQUIRE( valid == true );

    while(elements <= maxAllocations) {
        bool validRealloc=mockArena.reallocate(mockRequester,sizeB_mockRequester,\
            sizeB_mockRequester_next);
        sizeB_mockRequester=sizeB_mockRequester_next;
        sizeB_mockRequester_next+=4;
        REQUIRE( validRealloc == true );
        elements++;
    }
    bool validRealloc=mockArena.reallocate(mockRequester,sizeB_mockRequester,\
            sizeB_mockRequester_next);
    REQUIRE( validRealloc == false );
}


TEST_CASE( "Max reallocated from one object when CrcAllocation", \
        "One reallocation cannot be bigger than arena") {
    char arena[SIZE_ARENA];
    cus::CrcAllocation mockArena(reinterpret_cast<void *>(&arena[0]), \
                                   reinterpret_cast<void *>(&arena[END_ARENA]));

    uint32_t elements=0;
    std::size_t sizeB_mockRequester=4;
    uint32_t freeDataSpace = (SIZE_ARENA/2) - ((2*sizeof(arch_t) + (3*sizeof(arch_t))));

    {
        void * mockRequester;
        bool valid=mockArena.allocate((arch_t)&mockRequester,mockRequester,\
                sizeB_mockRequester);
        REQUIRE( valid == true );
        bool validRealloc=mockArena.reallocate(mockRequester,sizeB_mockRequester,\
                freeDataSpace);
        REQUIRE( validRealloc == true );
    }
}

TEST_CASE( "Reject reallocations when CrcAllocation", \
        "A rejected reallocation doesn't change the current situation") {
    char arena[SIZE_ARENA];
    cus::CrcAllocation mockArena(reinterpret_cast<void *>(&arena[0]), \
                                   reinterpret_cast<void *>(&arena[END_ARENA]));

    uint32_t elements=0;
    std::size_t sizeB_mockRequester=4;
    uint32_t freeDataSpace = (SIZE_ARENA/2) - ((sizeof(arch_t) + (3*sizeof(arch_t))));

    {
        void * mockRequester;
        bool valid=mockArena.allocate((arch_t)&mockRequester,mockRequester,\
                sizeB_mockRequester);
        REQUIRE( valid == true );

        bool validRealloc=mockArena.reallocate(mockRequester,sizeB_mockRequester,\
                freeDataSpace+1);
        REQUIRE( validRealloc == false );

        bool validNewAlloc=mockArena.reallocate(mockRequester,sizeB_mockRequester,\
                freeDataSpace-1);
        REQUIRE( validNewAlloc == true );
    }
}

TEST_CASE( "Basic deallocation when CrcAllocation", \
        "The entire object must be easily deallocated") {
    char arena[SIZE_ARENA];
    cus::CrcAllocation mockArena(reinterpret_cast<void *>(&arena[0]), \
                                   reinterpret_cast<void *>(&arena[END_ARENA]));

    uint32_t elements=0;
    uint32_t freeDataSpace = (SIZE_ARENA/2) - ((2*sizeof(arch_t) + (3*sizeof(arch_t))));

    {
        void * mockRequester;
        bool valid=mockArena.allocate((arch_t)&mockRequester,mockRequester,\
                freeDataSpace-1);
        REQUIRE( valid == true );

        void * secondMockRequester;
        bool validSecondAlloc=mockArena.allocate((arch_t)&secondMockRequester, \
                secondMockRequester, freeDataSpace-1);
        REQUIRE( validSecondAlloc == false );

        bool validDealloc=mockArena.deallocate((arch_t)&mockRequester);
        REQUIRE( validDealloc == true );

        bool validSecondAlloc_retry=mockArena.allocate((arch_t)&secondMockRequester, \
                secondMockRequester, freeDataSpace-1);
        REQUIRE( validSecondAlloc_retry == true );
    }
}

TEST_CASE( "Multiple deallocation when CrcAllocation", \
        "After consecutive allocations and deallocations, available memory has to \
        be the same than before allocations deallocations") {
    char arena[SIZE_ARENA];
    cus::CrcAllocation mockArena(reinterpret_cast<void *>(&arena[0]), \
                                   reinterpret_cast<void *>(&arena[END_ARENA]));

    uint32_t elements=0;
    uint32_t freeDataSpace = (SIZE_ARENA/2) - ((2*sizeof(arch_t) + (3*sizeof(arch_t))));

    void * mockRequester_a;
    void * mockRequester_b;
    void * mockRequester_c;
    void * mockRequester;
    uint32_t sizeElement = freeDataSpace/4;
    {
        for(uint32_t rewrites=0;rewrites<1000;rewrites++) {
            (void)mockArena.allocate((arch_t)&mockRequester_a, \
                    mockRequester_a, sizeElement);
            REQUIRE( reinterpret_cast<arch_t>(mockRequester_a) == \
                    (reinterpret_cast<arch_t>(&arena[0]) + sizeof(arch_t)) );

            (void)mockArena.allocate((arch_t)&mockRequester_b, \
                    mockRequester_b, sizeElement);
            REQUIRE( reinterpret_cast<arch_t>(mockRequester_b) == \
                    (reinterpret_cast<arch_t>(mockRequester_a) +
                     sizeElement) );

            (void)mockArena.allocate((arch_t)&mockRequester_c, \
                    mockRequester_c, sizeElement);
            REQUIRE( reinterpret_cast<arch_t>(mockRequester_c) == \
                    ( reinterpret_cast<arch_t>(mockRequester_b) +
                     sizeElement) );

            (void)mockArena.deallocate((arch_t)&mockRequester_a);
            (void)mockArena.deallocate((arch_t)&mockRequester_b);
            (void)mockArena.deallocate((arch_t)&mockRequester_c);

            bool validAlloc = mockArena.allocate((arch_t)&mockRequester, \
                    mockRequester, sizeElement);
            REQUIRE( validAlloc == true );
            REQUIRE( reinterpret_cast<arch_t>(mockRequester_a) == \
                    (reinterpret_cast<arch_t>(&arena[0]) + sizeof(arch_t)) );

            bool validDealloc=mockArena.deallocate((arch_t)&mockRequester);
            REQUIRE( validDealloc == true );
        }
    }
}


TEST_CASE( "Invalid deallocation when CrcAllocation", \
        "Invalid deallocations does not affect the system") {
    char arena[SIZE_ARENA];
    cus::CrcAllocation mockArena(reinterpret_cast<void *>(&arena[0]), \
                                   reinterpret_cast<void *>(&arena[END_ARENA]));

    uint32_t elements=0;
    uint32_t freeDataSpace = (SIZE_ARENA/2) - ((2*sizeof(arch_t) + (3*sizeof(arch_t))));

    void * mockRequester_a;
    void * mockRequester_b;
    void * mockRequester_c;
    void * mockRequester_invalid;
    void * mockRequester;
    uint32_t sizeElement = freeDataSpace/4;
    {
        for(uint32_t rewrites=0;rewrites<1000;rewrites++) {
            (void)mockArena.allocate((arch_t)&mockRequester_a, \
                    mockRequester_a, sizeElement);
            (void)mockArena.deallocate((arch_t)&mockRequester_invalid);
            REQUIRE( reinterpret_cast<arch_t>(mockRequester_a) == \
                    (reinterpret_cast<arch_t>(&arena[0]) + sizeof(arch_t)) );

            (void)mockArena.allocate((arch_t)&mockRequester_b, \
                    mockRequester_b, sizeElement);
            (void)mockArena.deallocate((arch_t)&mockRequester_invalid);
            REQUIRE( reinterpret_cast<arch_t>(mockRequester_b) == \
                    (reinterpret_cast<arch_t>(mockRequester_a) +
                     sizeElement) );

            (void)mockArena.allocate((arch_t)&mockRequester_c, \
                    mockRequester_c, sizeElement);
            (void)mockArena.deallocate((arch_t)&mockRequester_invalid);
            REQUIRE( reinterpret_cast<arch_t>(mockRequester_c) == \
                    ( reinterpret_cast<arch_t>(mockRequester_b) +
                     sizeElement) );

            (void)mockArena.deallocate((arch_t)&mockRequester_invalid);
            (void)mockArena.deallocate((arch_t)&mockRequester_a);
            (void)mockArena.deallocate((arch_t)&mockRequester_b);
            (void)mockArena.deallocate((arch_t)&mockRequester_c);

            bool validAlloc = mockArena.allocate((arch_t)&mockRequester, \
                    mockRequester, sizeElement);
            REQUIRE( validAlloc == true );
            REQUIRE( reinterpret_cast<arch_t>(mockRequester_a) == \
                    (reinterpret_cast<arch_t>(&arena[0]) + sizeof(arch_t)) );

            bool validDealloc=mockArena.deallocate((arch_t)&mockRequester);
            REQUIRE( validDealloc == true );
        }
    }
}

TEST_CASE( "Basic mirroring", \
        "Half of the arena is mirrored") {
    char arena[SIZE_ARENA];
    cus::CrcAllocation mockArena(reinterpret_cast<void *>(&arena[0]), \
                                   reinterpret_cast<void *>(&arena[END_ARENA]));

    {
        for(uint32_t byteArena=(sizeof(arch_t));(byteArena<SIZE_ARENA/2);byteArena++) {
            arena[byteArena]=0xFF;
        }

        mockArena.updateMirror();

        for(uint32_t byteArena=(sizeof(arch_t));byteArena<(SIZE_ARENA/2);byteArena++) {
            REQUIRE( (arena[byteArena]&0xFF) == 0xFF );
        }
        for(uint32_t byteArena=((SIZE_ARENA/2)+sizeof(arch_t)); \
                byteArena < SIZE_ARENA;byteArena++) {
            REQUIRE( (arena[byteArena]&0xFF) == 0x00 );
        }
    }
}

TEST_CASE( "Inverted copy", \
        "The mirror is an inverted copy of the original data") {
    char arena[SIZE_ARENA];
    cus::CrcAllocation mockArena(reinterpret_cast<void *>(&arena[0]), \
                                   reinterpret_cast<void *>(&arena[END_ARENA]));

    {
        char counterOrig=0;
        for(uint32_t byteArena=(0+sizeof(arch_t));(byteArena<SIZE_ARENA/2);byteArena++) {
            arena[byteArena]=counterOrig++;
        }
        mockArena.updateMirror();

        char counterData=0;
        for(uint32_t byteArena=(0+sizeof(arch_t));byteArena<(SIZE_ARENA/2);byteArena++) {
            REQUIRE( (arena[byteArena]) == counterData++ );
        }

        char counterMirror=0;
        for(uint32_t byteArena=((SIZE_ARENA/2)+sizeof(arch_t)); \
                byteArena < SIZE_ARENA;byteArena++) {
            REQUIRE( (arena[byteArena]) == ~(counterMirror++) );
        }

        REQUIRE( counterOrig == counterData );
        REQUIRE( counterData == counterMirror );
    }
}

TEST_CASE( "Basic check", \
        "Check the CRCs") {
    char arena[SIZE_ARENA];
    cus::CrcAllocation mockArena(reinterpret_cast<void *>(&arena[0]), \
                                   reinterpret_cast<void *>(&arena[END_ARENA]));

    {
        char counter=0;
        for(uint32_t byteArena=(sizeof(arch_t));(byteArena<SIZE_ARENA/2);byteArena++) {
            arena[byteArena]=counter++;
        }
        mockArena.updateMirror();
        bool valid = mockArena.checkConsistency();
        REQUIRE( valid == true );
    }
}

TEST_CASE( "Recover data", \
        "The mirroring can recover the data if one of the mirrors is valid") {
    char arena[SIZE_ARENA];
    cus::CrcAllocation mockArena(reinterpret_cast<void *>(&arena[0]), \
                                   reinterpret_cast<void *>(&arena[END_ARENA]));

    {
        char counter=0;
        for(uint32_t byteArena=(0+sizeof(arch_t));(byteArena<SIZE_ARENA/2);byteArena++) {
            arena[byteArena]=counter++;
        }
        mockArena.updateMirror();
        // corrupt the orig side
        arena[(SIZE_ARENA/2)-(SIZE_ARENA/4)]=0x5A;
        bool valid = mockArena.checkConsistency();
        REQUIRE( valid == true );
    }


    {
        char counter=0;
        for(uint32_t byteArena=(0+sizeof(arch_t));(byteArena<SIZE_ARENA/2);byteArena++) {
            arena[byteArena]=counter++;
        }
        mockArena.updateMirror();
        // corrupt the orig side
        arena[(SIZE_ARENA/2)+(SIZE_ARENA/4)]=0x5A;
        bool valid = mockArena.checkConsistency();
        REQUIRE( valid == true );
    }

    {
        char counter=0;
        for(uint32_t byteArena=(0+sizeof(arch_t));(byteArena<SIZE_ARENA/2);byteArena++) {
            arena[byteArena]=counter++;
        }
        mockArena.updateMirror();
        // corrupt the orig side
        arena[(SIZE_ARENA/2)+(SIZE_ARENA/4)]=0x5A;
        arena[(SIZE_ARENA/2)-(SIZE_ARENA/4)]=0x5A;
        bool valid = mockArena.checkConsistency();
        REQUIRE( valid == false );
    }
}



TEST_CASE( "Restore data", \
        "Check is able to restore the data if possible") {
    char arena[SIZE_ARENA];
    cus::CrcAllocation mockArena(reinterpret_cast<void *>(&arena[0]), \
                                   reinterpret_cast<void *>(&arena[END_ARENA]));

    {
        char counterOrig=0;
        for(uint32_t byteArena=(0+sizeof(arch_t));(byteArena<SIZE_ARENA/2);byteArena++) {
            arena[byteArena]=counterOrig++;
        }
        mockArena.updateMirror();
        arena[20]=0x5A;
        bool valid = mockArena.checkConsistency();

        char counterData=0;
        for(uint32_t byteArena=(0+sizeof(arch_t));byteArena<(SIZE_ARENA/2);byteArena++) {
            REQUIRE( (arena[byteArena]) == counterData++ );
        }

        REQUIRE( arena[(SIZE_ARENA/2)+(20)] == \
                ~arena[20] );
    }

    {
        char counterOrig=0;
        for(uint32_t byteArena=(0+sizeof(arch_t));(byteArena<SIZE_ARENA/2);byteArena++) {
            arena[byteArena]=counterOrig++;
        }
        mockArena.updateMirror();
        arena[(SIZE_ARENA/2)+(20)]=0x5A;

        char counterData=0;
        for(uint32_t byteArena=(0+sizeof(arch_t));byteArena<(SIZE_ARENA/2);byteArena++) {
            REQUIRE( (arena[byteArena]) == counterData++ );
        }

        REQUIRE( arena[(SIZE_ARENA/2)+(20)] == 0x5A );
    }
}
