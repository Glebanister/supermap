#pragma once

#include "FileManager.hpp"

namespace supermap::io {

class TemporaryFile {
  public:
    explicit TemporaryFile(const std::filesystem::path &path, std::shared_ptr<FileManager> manager)
        : path_(path), manager_(std::move(manager)) {
        manager_->create(path);
    }

    TemporaryFile(TemporaryFile &&) = default;
    TemporaryFile &operator=(TemporaryFile &&) = default;

    void cancelDeletion() noexcept {
        canceledDeletion_ = true;
    }

    [[nodiscard]] const std::string &getPath() const noexcept {
        return path_;
    }

    [[nodiscard]] std::shared_ptr<FileManager> getFileManager() const noexcept {
        return manager_;
    }

    ~TemporaryFile() {
        if (!canceledDeletion_) {
            manager_->remove(path_);
        }
    }

  private:
    std::string path_;
    std::shared_ptr<FileManager> manager_;
    bool canceledDeletion_ = false;
};

} // supermap::io
