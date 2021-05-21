#pragma once

#include <cstdint>
#include <vector>

#include "KeyOffset.h"
#include "exception/SupermapException.hpp"

namespace supermap {

class DiskDataStorage {
  public:
    struct AddInfo {
        const std::uint64_t offset;
        const bool wasShrunk;
    };

    explicit DiskDataStorage(double maxOccupancy);

    AddInfo add(const std::string &key, std::string&& value);

    std::string getValue(std::uint64_t offset);

    std::vector<KeyOffset> getRecords();

  protected:
    void shrink();

  private:
    const double maxOccupancy_;
};

} // supermap
