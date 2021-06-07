#pragma once

#include <stdexcept>

namespace supermap {

/**
 * @brief Any exception that happened in @p supermap scope.
 */
class SupermapException : public std::logic_error {
  public:
    explicit SupermapException(const std::string &message);

    explicit SupermapException(const std::exception &e);
};

/**
 * @brief An implementation stub.
 */
class NotImplementedException : public SupermapException {
  public:
    explicit NotImplementedException(const std::string &message = "");
};

/**
 * @brief Thrown when invalid operation with input or output is executed.
 */
class IOException : public SupermapException {
  public:
    explicit IOException(const std::string &message);
};

/**
 * @brief Thrown when accessing value of non-existent key.
 */
class KeyException : public SupermapException {
  public:
    explicit KeyException(const std::string &key, const std::string &message);

    explicit KeyException(const std::string &message);
};
} // supermap
