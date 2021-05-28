#include "DiskFileManager.hpp"

namespace supermap::io {

std::unique_ptr<InputStream> DiskFileManager::getInputStream(const std::filesystem::path &filename, std::size_t offset) {
    return std::make_unique<FileInputStream>(filename, offset);
}

std::unique_ptr<OutputStream> DiskFileManager::getOutputStream(const std::filesystem::path &filename, bool append) {
    return std::make_unique<FileOutputStream>(filename, append);
}

} // supermap::io
