#include "SupermapException.hpp"

namespace supermap {
SupermapException::SupermapException(const std::string &message)
    : std::logic_error(message) {}

SupermapException::SupermapException(const std::exception &e)
    : std::logic_error(std::string("cause: ") + e.what()) {}

NotImplementedException::NotImplementedException()
    : SupermapException("Method is not implemented") {}

IOException::IOException(const std::string &message)
    : SupermapException("IOException: " + message) {}
} // supermap
