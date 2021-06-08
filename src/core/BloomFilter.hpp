#pragma once

#include "Filter.hpp"
#include "primitive/KeyValue.hpp"

namespace supermap {

/**
 * @brief A filter based on the bloom filtering algorithm.
 */
template <typename Index, typename T, typename KI = KeyValue<T, Index>>
class BloomFilter : public Filter<KI, T> {
  private:
    using BaseFilter = Filter<KI, T>;

  public:
    /**
     * @brief Add an key-index pair to filter.
     * @param keyIndex Pair to add to filter.
     */
    void add(const KI &) override {
        // add keyIndex.key
    }

    /**
     * @return @p false if @p was never added to filter, anything otherwise.
     */
    bool mightContain(const T &) const override {
        // check if elem was added
        return true;
    }

    /**
     * @brief Create cloned version of this filter.
     */
    std::unique_ptr<BaseFilter> clone() const override {
        // Copy data to new filter here
        return std::make_unique<BloomFilter<Index, T>>();
    }

  private:
    // data...
};

} // supermap
