#pragma once

#include "SupermapException.hpp"

namespace supermap {

class IteratorException : public SupermapException {
  public:
    explicit IteratorException(const std::string &message);
};

} // supermap
