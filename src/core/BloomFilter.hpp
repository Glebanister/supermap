#pragma once

#include "Filter.hpp"
#include "exception/SupermapException.hpp"

namespace supermap {

class BloomFilter : public Filter {
  public:
    void addKey(const std::string &key) override;

    [[nodiscard]] bool mightContain(const std::string &key) const override;
};

} // supermap
