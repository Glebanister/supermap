#pragma once

#include <string>

namespace supermap {

class StorageFilter {
  public:
    virtual void addKey(const std::string &key) = 0;

    [[nodiscard]] virtual bool mightContain(const std::string &key) const = 0;

    virtual ~StorageFilter() = default;
};

} // supermap
