#pragma once

#include <fstream>
#include <sstream>
#include <exception/FileException.hpp>
#include <filesystem>

namespace supermap::io {

/**
 * @brief Abstract input stream. Only a wrapper to @p std::istream.
 */
class InputStream {
  public:
    /**
     * @return Reference on actual @p std::istream.
     */
    virtual std::istream &get() = 0;

    /**
     * @return Number of bytes available to read from stream.
     */
    virtual std::size_t availableBytes() = 0;

    virtual ~InputStream() = default;
};

/**
 * @brief Specialization of InputStream which encapsulates work with @p std::ifstream.
 * Dedicated to work with file input streams.
 */
class FileInputStream : public InputStream {
  public:
    /**
     * @brief Creates new input stream from file @p filename and offset @p offset.
     * @param filename File to read from.
     * @param offset @p Read offset begin.
     */
    explicit FileInputStream(const std::filesystem::path &filename, std::uint64_t offset);

    /**
     * @return Reference at actual stream. @p std::ifstream originally.
     */
    std::istream &get() override;

    //! @copydoc supermap::io::InputStream::availableBytes()
    std::size_t availableBytes() override;

  private:
    std::ifstream ifs_;
    std::uint64_t fileSize_;
};

/**
 * @brief Specialization of InputStream which encapsulates work with @p std::stringstream.
 * Dedicated to work with strings as input buffers.
 */
class StringInputStream : public InputStream {
  public:
    /**
     * @brief Creates new string input stream.
     * @param str A buffer, from which all data is read.
     * @param offset Read offset begin.
     */
    explicit StringInputStream(const std::string &str, std::uint64_t offset);

    /**
     * @return Reference at actual string stream. @p std::stringstream originally.
     */
    std::istream &get() override;

    //! @copydoc supermap::io::InputStream::availableBytes()
    std::size_t availableBytes() override;

  private:
    std::stringstream stringStream_;
    std::streampos initialPos_;
    const std::size_t stringLength_;
};

} // supermap::io
