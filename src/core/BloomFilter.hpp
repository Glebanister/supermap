#pragma once

#include <cassert>
#include <random>
#include <sstream>

#include "Filter.hpp"
#include "primitive/KeyValue.hpp"
#include "xxhash.h"

namespace supermap {

/**
 * @brief A filter based on the bloom filtering algorithm. Error probability is set in constructor
 */
template <typename T>
class BloomFilter : public Filter<T> {
  private:
    using BaseFilter = Filter<T>;

  public:
    explicit BloomFilter(double errorProbability) : sizeMultiplier_(-1.44 * std::log2(errorProbability)) {
        constexpr std::size_t keysSize = io::FixedDeserializedSizeRegister<T>::exactDeserializedSize;
        auto numberOfHashFunctions = std::max(1ul, static_cast<std::size_t>(std::ceil(std::log2(keysSize))));
        seeds_.resize(numberOfHashFunctions);
        std::mt19937_64 rnd(std::random_device{}());
        for (auto &seed : seeds_) {
            seed = rnd();
        }
    }
    /**
     * @brief Add an key-index pair to filter.
     * @param keyIndex Pair to add to filter.
     */
    void add(const T &value) override {
        assert(!elements_.empty());
        std::string data = serialize(value);
        for (auto &seed : seeds_) {
            std::size_t index = getHashWithSeed(data.data(), data.length(), seed);
            elements_[index] = true;
        }
    }

    /**
     * @return @p false if @p was never added to filter, anything otherwise.
     */
    bool mightContain(const T &value) const override {
        assert(!elements_.empty());
        std::string data = serialize(value);
        for (auto &seed : seeds_) {
            std::size_t index = getHashWithSeed(data.data(), data.length(), seed);
            if (!elements_[index]) {
                return false;
            }
        }
        return true;
    }

    /**
     * @brief Create cloned version of this filter.
     */
    std::unique_ptr<BaseFilter> clone() const override {
        return std::make_unique<BloomFilter<T>>(*this);
    }

    void reserve(std::uint64_t numberOfElements) override {
        assert(!wasReserved_);
        wasReserved_ = true;
        auto size = static_cast<std::size_t>(std::ceil(static_cast<double>(numberOfElements) * sizeMultiplier_));
        elements_.resize(size);
    };

    BloomFilter(const BloomFilter &other) = default;

  private:
    const double sizeMultiplier_;
    std::vector<XXH64_hash_t> seeds_;
    std::vector<bool> elements_;
    bool wasReserved_ = false;

    std::size_t getHashWithSeed(const void *data, std::size_t len, XXH64_hash_t seed) const {
        return static_cast<std::size_t>(get64bitHashWithSeed(data, len, seed) % elements_.size());
    }

    XXH64_hash_t get64bitHashWithSeed(const void *data, std::size_t len, XXH64_hash_t seed) const {
        return XXH3_64bits_withSeed(data, len, seed);
    }

    std::string serialize(const T &value) const {
        std::stringstream stream;
        io::serialize(value, stream);
        return stream.str();
    }
};

} // supermap
