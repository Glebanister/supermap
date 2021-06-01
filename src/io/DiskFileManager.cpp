#include <cstring>

#include "DiskFileManager.hpp"

namespace supermap::io {

std::unique_ptr<InputStream> DiskFileManager::getInputStream(const std::filesystem::path &filename,
                                                             std::uint64_t offset) {
    return std::make_unique<FileInputStream>(filename, offset);
}

std::unique_ptr<OutputStream> DiskFileManager::getOutputStream(const std::filesystem::path &filename, bool append) {
    return std::make_unique<FileOutputStream>(filename, append);
}

void DiskFileManager::remove(const std::filesystem::path &p) {
    if (!std::filesystem::remove(p)) {
        throw FileException(p, "Can not delete file");
    }
}

void DiskFileManager::rename(const std::filesystem::path &prev, const std::filesystem::path &next) {
    try {
        std::filesystem::rename(prev, next);
    } catch (const std::filesystem::filesystem_error &er) {
        throw FileException(prev, er.what());
    }
}

void DiskFileManager::swap(const std::filesystem::path &first, const std::filesystem::path &second) {
    const std::string temp_name = first.string() + "-swap";
    rename(first, temp_name);
    rename(second, first);
    rename(temp_name, second);
}

} // supermap::io
