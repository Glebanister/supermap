#include <filesystem>

#include "InputStream.hpp"

namespace supermap::io {

FileInputStream::FileInputStream(const std::filesystem::path &filename, std::size_t offset)
    : ifs_(filename) {
    if (!ifs_.good() || ifs_.bad()) {
        throw FileException(filename, "Unable to open for reading");
    }
    ifs_.seekg(std::ios::end);
    fileSize_ = ifs_.tellg();
    ifs_.seekg(static_cast<std::streamoff>(offset));
}

std::istream &FileInputStream::get() {
    return ifs_;
}

std::size_t FileInputStream::availableBytes() {
    return fileSize_ - ifs_.tellg();
}

StringInputStream::StringInputStream(const std::string &str, std::size_t offset)
    : stringStream_(str.substr(offset)),
      initialPos_(stringStream_.tellg()),
      stringLength_(str.length() - offset) {
}

std::istream &StringInputStream::get() {
    return stringStream_;
}

std::size_t StringInputStream::availableBytes() {
    if (stringStream_.tellg() == -1) {
        return 0;
    }
    std::size_t curPos = stringStream_.tellg() - initialPos_;
    return stringLength_ - curPos;
}

} // supermap::io
