#include "IteratorException.hpp"

namespace supermap {

IteratorException::IteratorException(const std::string &message)
    : SupermapException("IteratorException: " + message) {}

} // supermap
