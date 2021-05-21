#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "KeyOffset.h"
#include "exception/SupermapException.hpp"

namespace supermap {

class DiskIndex {
  public:
    explicit DiskIndex(const std::vector<KeyOffset>& records);

    std::uint64_t getOffset(const std::string &key);

    void addAll(std::vector<KeyOffset>& records);

    void clear();
};

} // supermap
