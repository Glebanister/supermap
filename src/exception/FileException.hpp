#pragma once

#include "SupermapException.hpp"

namespace supermap {

class FileException : public SupermapException {
  public:
    explicit FileException(const std::string &fileName, const std::string &message);
};

} // supermap
