#pragma once

#include <deque>

#include "FileManager.hpp"

namespace supermap::io {

/**
 * @brief Virtual file manager. None of actions are affecting real file system.
 * All changes are stored in operating memory, so everything will be erased
 * after end of program execution.
 */
class RamFileManager : public FileManager {
  private:
    struct File {
        std::string name;
        std::string content;
    };

  public:
    //! @copydoc InputStream::getInputStream()
    std::unique_ptr<InputStream> getInputStream(const std::filesystem::path &filename, std::uint64_t offset) override;

    //! @copydoc InputStream::getOutputStream()
    std::unique_ptr<OutputStream> getOutputStream(const std::filesystem::path &filename, bool append) override;

    //! @copydoc InputStream::remove()
    void remove(const std::filesystem::path &path) override;

    //! @copydoc InputStream::rename()
    void rename(const std::filesystem::path &prev, const std::filesystem::path &next) override;

    //! @copydoc InputStream::swap()
    void swap(const std::filesystem::path &, const std::filesystem::path &) override;

  private:
    [[nodiscard]] std::deque<File>::const_iterator getFileIterator(const std::filesystem::path &path) const;

    [[nodiscard]] std::deque<File>::const_iterator getFileIteratorNoThrow(const std::filesystem::path &path) const noexcept;

    [[nodiscard]] File &accessFile(const std::deque<File>::const_iterator &it);

    std::deque<File> files;
};

} // supermap::io
