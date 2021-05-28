#pragma once

#include "SupermapException.hpp"

namespace supermap {

class IllegalArgumentException : public SupermapException {
  public:
    explicit IllegalArgumentException(const std::string &message);
};

} // supermap
