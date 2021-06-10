#include "IllegalStateException.hpp"

namespace supermap {

IllegalStateException::IllegalStateException(const std::string &message)
    : SupermapException("IllegalStateException: " + message) {}

} // supermap
