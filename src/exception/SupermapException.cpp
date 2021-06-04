#include "SupermapException.hpp"

namespace supermap {
SupermapException::SupermapException(const std::string &message)
    : std::logic_error(message) {}

SupermapException::SupermapException(const std::exception &e)
    : std::logic_error(std::string("cause: ") + e.what()) {}

IOException::IOException(const std::string &message)
    : SupermapException("IOException: " + message) {}

NotImplementedException::NotImplementedException(const std::string &message)
    : SupermapException(std::string() + "NotImplementedException" + (message.empty() ? "" : ": ") + message) {}

KeyException::KeyException(const std::string &key, const std::string &message)
    : SupermapException(std::string() + "KeyException ('" + key + "')" + (message.empty() ? "" : ": ") + message) {}
} // supermap
