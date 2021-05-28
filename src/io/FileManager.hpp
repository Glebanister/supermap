#pragma once

#include <filesystem>

#include "InputStream.hpp"
#include "OutputStream.hpp"
#include "Parser.hpp"
#include "Writer.hpp"

namespace supermap::io {

class FileManager {
  public:
    virtual std::unique_ptr<InputStream> getInputStream(const std::filesystem::path &path, std::uint64_t offset) = 0;

    virtual std::unique_ptr<OutputStream> getOutputStream(const std::filesystem::path &path, bool append) = 0;

    virtual void remove(const std::filesystem::path &path) = 0;

    template <typename T>
    Writer<T> getWriter(const std::filesystem::path &filename, bool append) {
        return Writer<T>(getOutputStream(filename, append));
    }

    template <typename T>
    Parser<T> getParser(const std::filesystem::path &filename, std::uint64_t offset) {
        return Parser<T>(getInputStream(filename, offset));
    }

    virtual ~FileManager() = default;
};

} // supermap
