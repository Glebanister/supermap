#pragma once

#include <memory>

#include "OutputStream.hpp"
#include "SerializeHelper.hpp"
#include "exception/IteratorException.hpp"

namespace supermap::io {

template <typename T, typename = std::enable_if_t<SerializeHelper<T>::isSerializable>>
class OutputIterator {
  public:
    explicit OutputIterator(std::unique_ptr<OutputStream> &&os)
        : os_(std::move(os)) {}

    static OutputIterator toFile(const std::string &filename, bool append) {
        return OutputIterator(std::make_unique<FileOutputStream>(filename, append));
    }

    static OutputIterator toString(std::string &buffer, bool append) {
        return OutputIterator(std::make_unique<StringOutputStream>(buffer, append));
    }

  public:
    void write(const T &obj) {
        serialize(obj, os_->get());
    }

    template <typename CollectionT>
    void writeAll(const CollectionT &collection) {
        for (const T &obj : collection) {
            serialize(obj, os_->get());
        }
    }

    void flush() {
        os_->flush();
    }

    std::unique_ptr<OutputStream> os_;
};

} // supermap::io
