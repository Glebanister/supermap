#include "EncapsulatedFileManager.hpp"

namespace supermap::io {

EncapsulatedFileManager::EncapsulatedFileManager(std::shared_ptr<TemporaryFolder> root,
                                                 std::unique_ptr<FileManager> &&innerManager)
    : root_(std::move(root)), innerManager_(std::move(innerManager)) {
}

std::filesystem::path EncapsulatedFileManager::makeRootPath(const std::filesystem::path &p) {
    if (!p.is_relative()) {
        throw FileException(p.string(), "Is not relative");
    }
    return std::filesystem::path(root_->getPath()) / p;
}

std::unique_ptr<OutputStream> EncapsulatedFileManager::getOutputStream(const std::filesystem::path &path, bool append) {
    return innerManager_->getOutputStream(makeRootPath(path), append);
}

std::unique_ptr<InputStream> EncapsulatedFileManager::getInputStream(const std::filesystem::path &path,
                                                                     std::uint64_t offset) {
    return innerManager_->getInputStream(makeRootPath(path), offset);
}

void EncapsulatedFileManager::remove(const std::filesystem::path &path) {
    innerManager_->remove(makeRootPath(path));
}

void EncapsulatedFileManager::rename(const std::filesystem::path &prevPath, const std::filesystem::path &nextPath) {
    innerManager_->rename(makeRootPath(prevPath), makeRootPath(nextPath));
}

void EncapsulatedFileManager::swap(const std::filesystem::path &a, const std::filesystem::path &b) {
    innerManager_->swap(makeRootPath(a), makeRootPath(b));
}

} // supermap::io
