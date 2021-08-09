#pragma once

#include "io/SerializeHelper.hpp"
#include "Hasher.hpp"

namespace supermap::hashTools {

template <typename T>
std::uint64_t hashWith(const Hasher &hasher, const T key, std::uint64_t seed) {
    std::stringstream keyStream;
    io::serialize(key, keyStream);
    return hasher.hash(keyStream.str(), seed);
}

} // namespace supermap
