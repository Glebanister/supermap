#include "doctest.h"

#include "core/BloomFilter.hpp"
#include "primitive/Key.hpp"

#include <string>
#include <random>
#include <vector>

std::string generateStringWithLength(std::uint32_t length) {
    static std::mt19937 rnd(std::random_device{}());
    static const char charset[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    std::string ret;
    for (std::uint32_t k = 0; k < length; ++k) {
        auto charIndex = static_cast<std::uint32_t>(rnd() % (sizeof(charset) - 1));
        ret += charset[charIndex];
    }
    return ret;
}

TEST_CASE ("Small keys") {
    constexpr std::uint32_t steps = 1000;
    constexpr std::uint32_t length = 1;
    supermap::BloomFilter<supermap::Key<length>> filter(1 / 32.0);
    filter.reserve(steps);

    std::vector<supermap::Key<length>> keys;
    for (std::uint32_t k = 0; k < steps; ++k) {
        std::string str = generateStringWithLength(length);
        supermap::Key<length> key = supermap::Key<length>::fromString(str);
        filter.add(key);
        CHECK(filter.mightContain(key));
        keys.push_back(key);
    }
    for (const auto &key : keys) {
        CHECK(filter.mightContain(key));
    }
}

TEST_CASE ("Big keys") {
    constexpr std::uint32_t steps = 1000;
    constexpr std::uint32_t length = 10000;
    supermap::BloomFilter<supermap::Key<length>> filter(1 / 32.0);
    filter.reserve(steps);

    std::vector<supermap::Key<length>> keys;
    for (std::uint32_t k = 0; k < steps; ++k) {
        std::string str = generateStringWithLength(length);
        supermap::Key<length> key = supermap::Key<length>::fromString(str);
        filter.add(key);
        CHECK(filter.mightContain(key));
        keys.push_back(key);
    }
    for (const auto &key : keys) {
        CHECK(filter.mightContain(key));
    }
}
