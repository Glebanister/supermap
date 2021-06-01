#pragma once

#include <deque>

#include "FileManager.hpp"

namespace supermap::io {

class RamFileManager : public FileManager {
  private:
    struct File {
        std::filesystem::path name;
        std::string content;
    };

  public:
    std::unique_ptr<InputStream> getInputStream(const std::filesystem::path &filename, std::uint64_t offset) override;

    std::unique_ptr<OutputStream> getOutputStream(const std::filesystem::path &filename, bool append) override;

    void remove(const std::filesystem::path &path) override;

    void rename(const std::filesystem::path &prev, const std::filesystem::path &next) override;

    void swap(const std::filesystem::path &, const std::filesystem::path &) override;

  private:
    [[nodiscard]] std::deque<File>::const_iterator getFileIterator(const std::filesystem::path &path) const;

    [[nodiscard]] std::deque<File>::const_iterator getFileIteratorNoThrow(const std::filesystem::path &path) const noexcept;

    [[nodiscard]] File &accessFile(const std::deque<File>::const_iterator &it);

    std::deque<File> files;
};

} // supermap::io
