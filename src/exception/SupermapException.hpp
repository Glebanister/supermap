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
    explicit NotImplementedException(const std::string &message = "");
};

class IOException : public SupermapException {
  public:
    explicit IOException(const std::string &message);
};

class KeyException : public SupermapException {
  public:
    explicit KeyException(const std::string &key, const std::string &message);
};
} // supermap
