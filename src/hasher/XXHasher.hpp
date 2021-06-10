#pragma once

#include "Hasher.hpp"
#include "xxhash.h"

namespace supermap {

/**
 * @brief A hasher based on xxhash
 */
class XXHasher : public Hasher {
  public:
    [[nodiscard]] std::uint64_t hash(const std::string& string, std::uint64_t seed) override {
        const void* data = string.c_str();
        std::size_t len = string.length();
        return get64bitHashWithSeed(data, len, seed);
    }

    [[nodiscard]] std::uint64_t hash(const void* data, std::size_t len, std::uint64_t seed) override {
        return get64bitHashWithSeed(data, len, seed);
    }

    [[nodiscard]] std::unique_ptr<Hasher> clone() const override {
        return std::make_unique<XXHasher>();
    }

  private:
    static std::uint64_t get64bitHashWithSeed(const void *data, std::size_t len, XXH64_hash_t seed) {
        return XXH3_64bits_withSeed(data, len, seed);
    }
};

} // supermap