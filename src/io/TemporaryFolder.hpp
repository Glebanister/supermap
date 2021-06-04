#pragma once

#include <filesystem>

namespace supermap::io {

/**
 * @brief Temporary folder in the file system.
 */
class TemporaryFolder {
  public:
    /**
     * @brief Creates temporary folder in file system.
     * @param name Name of created folder.
     * @param isFilesystem If folder needs to be created in physical file system.
     * If @p false, file system will not be touched.
     */
    explicit TemporaryFolder(std::string name, bool isFilesystem)
        : isFilesystem_(isFilesystem), name_(std::move(name)) {
        if (isFilesystem_) {
            std::filesystem::create_directory(name_);
        }
    }

    /**
     * @return Path of folder.
     */
    [[nodiscard]] const std::string &getPath() const noexcept {
        return name_;
    }

    TemporaryFolder(const TemporaryFolder &) = delete;
    TemporaryFolder &operator=(const TemporaryFolder &) = delete;

    TemporaryFolder &operator=(TemporaryFolder &&other) noexcept {
        if (isFilesystem_) {
            std::filesystem::remove(name_);
        }
        isFilesystem_ = other.isFilesystem_;
        name_ = std::move(other.name_);
        return *this;
    }

    TemporaryFolder(TemporaryFolder &&other) noexcept = default;

    /**
     * @brief Removes folder iff it was once actually created.
     */
    ~TemporaryFolder() {
        if (isFilesystem_) {
            std::filesystem::remove(name_);
        }
    }

  private:
    bool isFilesystem_;
    std::string name_;
};

} // supermap::io
