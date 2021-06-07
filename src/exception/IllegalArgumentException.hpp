#pragma once

#include "SupermapException.hpp"

namespace supermap {

/**
 * @brief Thrown when some illegal argument is passed.
 */
class IllegalArgumentException : public SupermapException {
  public:
    explicit IllegalArgumentException(const std::string &message);
};

} // supermap
