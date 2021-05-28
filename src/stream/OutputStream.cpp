#include "OutputStream.hpp"

namespace supermap::stream {

FileOutputStream::FileOutputStream(const std::string &filename, bool append)
    : filename_(filename), ofs_(filename, append ? std::ios_base::app : std::ios_base::out) {
    if (!ofs_.good() || ofs_.bad()) {
        throw FileException(filename, "Unable to open for writing");
    }
}

void FileOutputStream::flush() {
    ofs_.flush();
}

std::ostream &FileOutputStream::get() {
    return ofs_;
}

StringOutputStream::StringOutputStream(std::string &buffer, bool append)
    : buffer_(buffer) {
    if (!append) {
        buffer_ = "";
    }
}

std::ostream &StringOutputStream::get() {
    return stringStream_;
}

void StringOutputStream::flush() {
    buffer_.append(stringStream_.str());
    stringStream_.clear();
}

} // supermap::stream
