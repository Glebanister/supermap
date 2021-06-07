#pragma once

#include "Filter.hpp"
#include "exception/SupermapException.hpp"

namespace supermap {

template <typename T>
class BloomFilter : public Filter<T> {
  public:
    void add(const T &elem) override {
        throw NotImplementedException("Add key to bloom filter");
    }

    [[nodiscard]] bool mightContain(const T &elem) const override {
        throw NotImplementedException("Might contain key in bloom filter");
    }
};

} // supermap
