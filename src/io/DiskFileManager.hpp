#pragma once

#include "FileManager.hpp"

namespace supermap::io {

class DiskFileManager : public FileManager {
  public:
    std::unique_ptr<InputStream> getInputStream(const std::filesystem::path &filename, std::uint64_t offset) override;

    std::unique_ptr<OutputStream> getOutputStream(const std::filesystem::path &filename, bool append) override;

    void remove(const std::filesystem::path &) override;

    void rename(const std::filesystem::path &prev, const std::filesystem::path &next) override;
};

} // supermap::io
