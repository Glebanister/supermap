#pragma once

#include "SupermapException.hpp"

namespace supermap {

/**
 * @brief Thrown when file exception is occurred.
 */
class FileException : public SupermapException {
  public:
    explicit FileException(const std::string &fileName, const std::string &message);
};

} // supermap
