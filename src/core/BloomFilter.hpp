#pragma once

#include "Filter.hpp"
#include "primitive/KeyValue.hpp"

namespace supermap {

/**
 * @brief A filter based on the bloom filtering algorithm.
 */
template <typename T>
class BloomFilter : public Filter<T> {
  private:
    using BaseFilter = Filter<T>;

  public:
    /**
     * @brief Add an key-index pair to filter.
     * @param keyIndex Pair to add to filter.
     */
    void add(const T &) override {
    }

    /**
     * @return @p false if @p was never added to filter, anything otherwise.
     */
    bool mightContain(const T &) const override {
        return true;
    }

    /**
     * @brief Create cloned version of this filter.
     */
    std::unique_ptr<BaseFilter> clone() const override {
        // Copy data to new filter here
        return std::make_unique<BloomFilter<T>>();
    }

  private:
    // data...
};

} // supermap
