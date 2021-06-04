#pragma once

#include <fstream>
#include <sstream>
#include <exception/FileException.hpp>

namespace supermap::io {

/**
 * @brief Abstract output stream. Wraps @p std::ostream.
 */
class OutputStream {
  public:
    /**
     * @return Underlying @p std::ostream reference.
     */
    virtual std::ostream &get() = 0;

    /**
     * @brief Flushes output stream. Guaranteed that all changes written to
     * output stream will be captured after @p flush call.
     */
    virtual void flush() = 0;

    virtual ~OutputStream() = default;
};

/**
 * @brief Output stream specialization to work with files output stream.
 */
class FileOutputStream : public OutputStream {
  public:
    /**
     * @brief Creates new @p FileOutputStream.
     * @param filename File to write into.
     * @param append @p true if need to append to the end of file instead of overwriting it.
     */
    explicit FileOutputStream(const std::string &filename, bool append);

    //! @copydoc OutputStream::flush()
    void flush() override;

    //! @copydoc OutputStream::get()
    std::ostream &get() override;

  private:
    const std::string filename_;
    std::ofstream ofs_;
};

class StringOutputStream : public OutputStream {
  public:
    /**
     * @brief Creates new @p StringOutputStream.
     * @param buffer Buffer to write into.
     * @param append @p true if need to append to the end of buffer instead of overwriting it.
     */
    explicit StringOutputStream(std::string &buffer, bool append);

    //! @copydoc OutputStream::flush()
    std::ostream &get() override;

    //! @copydoc OutputStream::get()
    void flush() override;

  private:
    std::string &buffer_;
    std::stringstream stringStream_;
};

} // supermap::io
