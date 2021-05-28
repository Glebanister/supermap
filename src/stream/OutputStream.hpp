#pragma once

#include <fstream>
#include <sstream>
#include <exception/FileException.hpp>

namespace supermap::stream {

class OutputStream {
  public:
    virtual std::ostream &get() = 0;

    virtual void flush() = 0;

    virtual ~OutputStream() = default;
};

class FileOutputStream : public OutputStream {
  public:
    explicit FileOutputStream(const std::string &filename, bool append);

    void flush() override;

    std::ostream &get() override;

  private:
    const std::string filename_;
    std::ofstream ofs_;
};

class StringOutputStream : public OutputStream {
  public:
    explicit StringOutputStream(std::string &buffer, bool append);

    std::ostream &get() override;

    void flush() override;

  private:
    std::string &buffer_;
    std::stringstream stringStream_;
};

} // supermap::stream
