#pragma once

#include <filesystem>

#include "InputStream.hpp"
#include "OutputStream.hpp"
#include "InputIterator.hpp"
#include "OutputIterator.hpp"

namespace supermap::io {

class FileManager {
  public:
    virtual std::unique_ptr<InputStream> getInputStream(const std::filesystem::path &path, std::uint64_t offset) = 0;

    virtual std::unique_ptr<OutputStream> getOutputStream(const std::filesystem::path &path, bool append) = 0;

    virtual void remove(const std::filesystem::path &path) = 0;

    virtual void rename(const std::filesystem::path &prevPath, const std::filesystem::path &nextPath) = 0;

    virtual void clear(const std::filesystem::path &path) {
        remove(path);
        create(path);
    }

    virtual void create(const std::filesystem::path &path) {
        getOutputStream(path, false)->flush();
    }

    virtual void swap(const std::filesystem::path &, const std::filesystem::path &) = 0;

    template <typename T>
    OutputIterator<T> getOutputIterator(const std::filesystem::path &filename, bool append) {
        return OutputIterator<T>(getOutputStream(filename, append));
    }

    template <typename T>
    InputIterator<T> getInputIterator(const std::filesystem::path &filename, std::uint64_t offset) {
        return InputIterator<T>(getInputStream(filename, offset));
    }

    virtual ~FileManager() = default;
};

} // supermap
