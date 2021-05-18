#pragma once

#include <stdexcept>

namespace supermap {
class SupermapException : public std::logic_error {
  public:
    explicit SupermapException(const std::string &message);

    explicit SupermapException(const std::exception &e);
};

class NotImplementedException : public SupermapException {
  public:
    NotImplementedException();
};
} // supermap
