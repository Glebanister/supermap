#include "IllegalArgumentException.hpp"

namespace supermap {

IllegalArgumentException::IllegalArgumentException(const std::string &message)
    : SupermapException("IllegalArgumentException: " + message) {}

} // supermap
