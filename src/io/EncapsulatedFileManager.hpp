#pragma once

#include "FileManager.hpp"
#include "TemporaryFolder.hpp"

namespace supermap::io {

class EncapsulatedFileManager : public FileManager {
  public:
    explicit EncapsulatedFileManager(std::shared_ptr<TemporaryFolder> root, std::unique_ptr<FileManager> &&innerManager)
        : root_(std::move(root)), innerManager_(std::move(innerManager)) {
    }

    std::filesystem::path makeRootPath(const std::filesystem::path &p) {
        if (!p.is_relative()) {
            throw FileException(p.string(), "Is not relative");
        }
        return std::filesystem::path(root_->getPath()) / p;
    }

    std::unique_ptr<InputStream> getInputStream(const std::filesystem::path &path, std::uint64_t offset) override {
        return innerManager_->getInputStream(makeRootPath(path), offset);
    }

    std::unique_ptr<OutputStream> getOutputStream(const std::filesystem::path &path, bool append) override {
        return innerManager_->getOutputStream(makeRootPath(path), append);
    }

    void remove(const std::filesystem::path &path) override {
        innerManager_->remove(makeRootPath(path));
    }

    void rename(const std::filesystem::path &prevPath, const std::filesystem::path &nextPath) override {
        innerManager_->rename(makeRootPath(prevPath), makeRootPath(nextPath));
    }

    void swap(const std::filesystem::path &a, const std::filesystem::path &b) override {
        innerManager_->swap(makeRootPath(a), makeRootPath(b));
    }

  private:
    std::shared_ptr<TemporaryFolder> root_;
    std::unique_ptr<FileManager> innerManager_;
};

} // supermap::io
