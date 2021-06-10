#pragma once

#include <random>
#include <sstream>

#include "Filter.hpp"
#include "primitive/KeyValue.hpp"
#include "exception/IllegalStateException.hpp"
#include "exception/IllegalArgumentException.hpp"
#include "xxhash.h"
#include "hasher/XXHasher.hpp"

namespace supermap {

/**
 * @brief A filter based on the bloom filtering algorithm.
 */
template <typename T>
class BloomFilter : public Filter<T> {
  private:
    using BaseFilter = Filter<T>;

  public:
    explicit BloomFilter(double errorProbability, std::unique_ptr<Hasher> &&hasher) : hasher_(std::move(hasher)) {
        if (errorProbability <= 0 || errorProbability > 1) {
            throw supermap::IllegalArgumentException("Error probability must be a positive number not bigger than 1");
        }
        sizeMultiplier_ = std::max(1.0, -1.44 * std::log2(errorProbability));
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
        if (elements_.empty()) {
            throw supermap::IllegalStateException("Filter size was not reserved or was set to zero");
        }
        std::string data = serialize(value);
        for (auto &seed : seeds_) {
            std::uint64_t index = getHashWithSeed(data, seed);
            elements_[index] = true;
        }
    }

    /**
     * @return @p false if @p was never added to filter, anything otherwise.
     */
    bool mightContain(const T &value) const override {
        if (elements_.empty()) {
            throw supermap::IllegalStateException("Filter size was not reserved or was set to zero");
        }
        std::string data = serialize(value);
        for (auto &seed : seeds_) {
            std::uint64_t index = getHashWithSeed(data, seed);
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
        if (wasReserved_) {
            throw supermap::IllegalStateException("Filter size has been already reserved");
        }
        wasReserved_ = true;
        auto size = static_cast<std::size_t>(std::ceil(static_cast<double>(numberOfElements) * sizeMultiplier_));
        elements_.resize(size);
    };

    BloomFilter(const BloomFilter &other)
        : hasher_(other.hasher_->clone()),
          sizeMultiplier_(other.sizeMultiplier_),
          seeds_(other.seeds_),
          elements_(other.elements_),
          wasReserved_(other.wasReserved_) {
    }

  private:
    std::unique_ptr<Hasher> hasher_;
    double sizeMultiplier_;
    std::vector<XXH64_hash_t> seeds_;
    std::vector<bool> elements_;
    bool wasReserved_ = false;

    std::uint64_t getHashWithSeed(const std::string &string, XXH64_hash_t seed) const {
        return hasher_->hash(string, seed) % elements_.size();
    }

    std::string serialize(const T &value) const {
        std::stringstream stream;
        io::serialize(value, stream);
        return stream.str();
    }
};

} // supermap
