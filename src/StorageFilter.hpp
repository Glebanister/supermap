#pragma once

#include <string>

namespace supermap {

class StorageFilter {
  public:
    virtual void addKey(const std::string &key) = 0;
    virtual void removeKey(const std::string &key) = 0;
    [[nodiscard]] virtual bool shouldCheckStorage(const std::string &key) const = 0;

    virtual ~StorageFilter() = default;
};

} // supermap
