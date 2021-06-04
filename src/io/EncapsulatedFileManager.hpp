#pragma once

#include "FileManager.hpp"
#include "TemporaryFolder.hpp"

namespace supermap::io {

class EncapsulatedFileManager : public FileManager {
  public:
    explicit EncapsulatedFileManager(std::shared_ptr<TemporaryFolder> root,
                                     std::unique_ptr<FileManager> &&innerManager);

    std::filesystem::path makeRootPath(const std::filesystem::path &p);

    std::unique_ptr<InputStream> getInputStream(const std::filesystem::path &path, std::uint64_t offset) override;

    std::unique_ptr<OutputStream> getOutputStream(const std::filesystem::path &path, bool append) override;

    void remove(const std::filesystem::path &path) override;

    void rename(const std::filesystem::path &prevPath, const std::filesystem::path &nextPath) override;

    void swap(const std::filesystem::path &a, const std::filesystem::path &b) override;

  private:
    std::shared_ptr<TemporaryFolder> root_;
    std::unique_ptr<FileManager> innerManager_;
};

} // supermap::io
