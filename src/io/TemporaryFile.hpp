#pragma once

#include "FileManager.hpp"

namespace supermap::io {

/**
 * @brief Temporary file in FileManager, which is created during ctor and deleted with dtor.
 */
class TemporaryFile {
  public:
    /**
     * @brief Initializes TemporaryFile, which corresponds an actual file in FileManager.
     * @param path Name of file to create.
     * @param manager File system manager.
     */
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

    /**
     * @brief Freeze file deletion: it won't be deleted by this object.
     */
    void cancelDeletion() noexcept {
        canceledDeletion_ = true;
    }

    /**
     * @return Path of corresponding file.
     */
    [[nodiscard]] const std::string &getPath() const noexcept {
        return path_;
    }

    /**
     * @return Corresponding file manager.
     */
    [[nodiscard]] std::shared_ptr<FileManager> getFileManager() const noexcept {
        return manager_;
    }

    /**
     * @brief Removes file from file system iff deletion was not canceled.
     */
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
