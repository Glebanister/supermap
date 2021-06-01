#pragma once

#include <fstream>
#include <sstream>
#include <exception/FileException.hpp>
#include <filesystem>

namespace supermap::io {
class InputStream {
  public:
    virtual std::istream &get() = 0;

    virtual std::size_t availableBytes() = 0;

    virtual ~InputStream() = default;
};

class FileInputStream : public InputStream {
  public:
    explicit FileInputStream(const std::filesystem::path &filename, std::uint64_t offset);

    std::istream &get() override;

    std::size_t availableBytes() override;

  private:
    std::ifstream ifs_;
    std::uint64_t fileSize_;
};

class StringInputStream : public InputStream {
  public:
    explicit StringInputStream(const std::string &str, std::uint64_t offset);

    std::istream &get() override;

    std::size_t availableBytes() override;

  private:
    std::stringstream stringStream_;
    std::streampos initialPos_;
    const std::size_t stringLength_;
};

} // supermap::io
