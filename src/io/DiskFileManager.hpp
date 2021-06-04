#pragma once

#include "FileManager.hpp"

namespace supermap::io {

/**
 * @brief Actual file system manager.
 */
class DiskFileManager : public FileManager {
  public:
    //! @copydoc supermap::io::FileManager::getInputStream()
    std::unique_ptr<InputStream> getInputStream(const std::filesystem::path &filename, std::uint64_t offset) override;

    //! @copydoc supermap::io::FileManager::getOutputStream()
    std::unique_ptr<OutputStream> getOutputStream(const std::filesystem::path &filename, bool append) override;

    //! @copydoc supermap::io::FileManager::remove()
    void remove(const std::filesystem::path &) override;

    //! @copydoc supermap::io::FileManager::rename()
    void rename(const std::filesystem::path &prev, const std::filesystem::path &next) override;

    //! @copydoc supermap::io::FileManager::swap()
    void swap(const std::filesystem::path &first, const std::filesystem::path &second) override;
};

} // supermap::io
