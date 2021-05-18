#pragma once

#include <cstdint>

#include "exception/SupermapException.hpp"

namespace supermap {

class DiskDataStorage {
  public:
    struct AddInfo {
        const std::uint64_t offset;
        const bool updateIndex;
    };

    explicit DiskDataStorage(double maxOccupancy);

    AddInfo addData(const std::string &key, std::string value);

    std::string getData(std::uint64_t offset);

  protected:
    void shrink();

  private:
    const double maxOccupancy_;
};
} // supermap
