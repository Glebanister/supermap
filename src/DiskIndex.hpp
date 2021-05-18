#pragma once

#include <cstdint>
#include <string>

#include "exception/SupermapException.hpp"

namespace supermap {
class DiskIndex {
  public:
    std::uint64_t getOffset(const std::string &key);

    void addKey(const std::string &key, std::uint64_t valueOffset);
};
} // supermap
