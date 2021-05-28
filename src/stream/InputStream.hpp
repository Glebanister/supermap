#pragma once

#include <fstream>
#include <sstream>
#include <exception/FileException.hpp>

namespace supermap::stream {
class InputStream {
  public:
    virtual std::istream &get() = 0;

    virtual std::size_t availableBytes() = 0;

    virtual ~InputStream() = default;
};

class FileInputStream : public InputStream {
  public:
    explicit FileInputStream(const std::string &filename, std::size_t offset);

    std::istream &get() override;

    std::size_t availableBytes() override;

  private:
    const std::string filename_;
    std::ifstream ifs_;
    std::uint64_t fileSize_;
};

class StringInputStream : public InputStream {
  public:
    explicit StringInputStream(const std::string &str, std::size_t offset);

    std::istream &get() override;

    std::size_t availableBytes() override;

  private:
    std::stringstream stringStream_;
    std::streampos initialPos_;
    const std::size_t stringLength_;
};

} // supermap::stream
