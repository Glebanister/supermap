#pragma once

#include "SupermapException.hpp"

namespace supermap {

/**
 * @brief Thrown when invalid operation with iterator is executed.
 */
class IteratorException : public SupermapException {
  public:
    explicit IteratorException(const std::string &message);
};

} // supermap
