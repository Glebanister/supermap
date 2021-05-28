#pragma once

#include "FileManager.hpp"

namespace supermap::io {

class DiskFileManager : public FileManager {
  public:
    std::unique_ptr<InputStream> getInputStream(const std::filesystem::path &filename, std::size_t offset) override;

    std::unique_ptr<OutputStream> getOutputStream(const std::filesystem::path &filename, bool append) override;
};

} // supermap::io
