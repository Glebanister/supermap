#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "KeyIndex.hpp"
#include "exception/SupermapException.hpp"

namespace supermap {

template <std::size_t KeyLen>
class DiskIndex {
  public:
    explicit DiskIndex(const std::vector<KeyIndex<KeyLen>> &) {
        throw NotImplementedException();
    }

    std::uint64_t getOffset(const std::string &) {
        throw NotImplementedException();
    }

    void addAll(std::vector<KeyIndex<KeyLen>> &) {
        throw NotImplementedException();
    }

    void clear() {
        throw NotImplementedException();
    }
};

} // supermap
