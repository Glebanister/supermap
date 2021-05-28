#include "FileException.hpp"

namespace supermap {

FileException::FileException(const std::string &fileName, const std::string &message)
    : SupermapException("File Exception has occurred while working with " + fileName + ": " + message) {}

} // supermap

