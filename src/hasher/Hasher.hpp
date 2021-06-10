#pragma once

#include "core/Cloneable.hpp"
#include "primitive/ByteArray.hpp"

#include <string>

namespace supermap {

/**
 * @brief An abstract hasher, which can hash std::string and supermap::ByteArray to std::uint64_t
 */

class Hasher : public Cloneable<Hasher> {
  public:
    /**
     * @brief Hash @p elem to std::uint64_t using @p seed.
     * @param elem std::string to hash.
     * @param seed std::uint64_t used in hash function.
     */
    [[nodiscard]] virtual std::uint64_t hash(const std::string&, std::uint64_t seed) = 0;

    /**
     * @brief Hash @p elem with length @p len to std::uint64_t using @p seed.
     * @param elem void* data to hash.
     * @param len std::size_t length of data.
     * @param seed std::uint64_t used in hash function.
     */
    [[nodiscard]] virtual std::uint64_t hash(const void* data, std::size_t len, std::uint64_t seed) = 0;
};

} // supermap
