#include <chrono>

std::uint32_t timeSeed() {
    return std::chrono::steady_clock::now().time_since_epoch().count();
}

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
