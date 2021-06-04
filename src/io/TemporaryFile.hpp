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
    TemporaryFile(const TemporaryFile &) = delete;
    TemporaryFile &operator=(const TemporaryFile &) = delete;

    TemporaryFile &operator=(TemporaryFile &&other) noexcept {
        if (this == &other) {
            return *this;
        }
        if (!canceledDeletion_) {
            manager_->remove(path_);
        }
        path_ = std::move(other.path_);
        manager_ = std::move(other.manager_);
        canceledDeletion_ = other.canceledDeletion_;
        return *this;
    }

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
