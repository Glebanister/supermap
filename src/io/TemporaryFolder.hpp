#pragma once

#include <filesystem>

namespace supermap::io {

class TemporaryFolder {
  public:
    explicit TemporaryFolder(std::string name, bool isFilesystem)
        : isFilesystem_(isFilesystem), name_(std::move(name)) {
        if (isFilesystem_) {
            std::filesystem::create_directory(name_);
        }
    }

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
