
/**
 * @brief
 *
 */


#include <iostream>
#include <cstring>
#include <cmath>
#include <vector>
#include <chrono>
#include "allocator.hpp"
#include "vector.hpp"


#define ARCH_ALIGMENT 8
#define ARENA_BYTES 4096

uint8_t section[ARENA_BYTES] __attribute__ ((aligned (ARCH_ALIGMENT)))={0};
arch_t *startSection=(arch_t *)section;
arch_t *endSection=(arch_t *)&section[ARENA_BYTES];

int main() {

}
