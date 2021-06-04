#pragma once

#include "FileManager.hpp"
#include "TemporaryFolder.hpp"

namespace supermap::io {

/**
 * @brief Capsule for other @p FileManager. Isolates all inner @p FileManager
 * activity in the @p root.
 */
class EncapsulatedFileManager : public FileManager {
  public:
    /**
     * @param root Isolation root.
     * @param innerManager manager to isolate.
     */
    explicit EncapsulatedFileManager(std::shared_ptr<TemporaryFolder> root,
                                     std::unique_ptr<FileManager> &&innerManager);

    /**
     * @brief Creates full path assuming @p p is a relative path form @p root.
     * Used for all request to @p EncapsulatedFileManager.
     * @param p Path to isolate.
     * @return Isolated path.
     */
    std::filesystem::path makeRootPath(const std::filesystem::path &p);

    //! @copydoc supermap::io::FileManager::getInputStream()
    std::unique_ptr<InputStream> getInputStream(const std::filesystem::path &path, std::uint64_t offset) override;

    //! @copydoc supermap::io::FileManager::getOutputStream()
    std::unique_ptr<OutputStream> getOutputStream(const std::filesystem::path &path, bool append) override;

    //! @copydoc supermap::io::FileManager::remove()
    void remove(const std::filesystem::path &path) override;

    //! @copydoc supermap::io::FileManager::rename()
    void rename(const std::filesystem::path &prevPath, const std::filesystem::path &nextPath) override;

    //! @copydoc supermap::io::FileManager::swap()
    void swap(const std::filesystem::path &a, const std::filesystem::path &b) override;

  private:
    std::shared_ptr<TemporaryFolder> root_;
    std::unique_ptr<FileManager> innerManager_;
};

} // supermap::io
