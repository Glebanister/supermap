#pragma once

#include "StorageFilter.hpp"
#include "exception/SupermapException.hpp"

namespace supermap {
class BloomFilter : public StorageFilter {
  public:
    void addKey(const std::string &key) override;

    void removeKey(const std::string &key) override;

    [[nodiscard]] bool shouldCheckStorage(const std::string &key) const override;
};
} // supermap
