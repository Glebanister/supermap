#pragma once

#include <memory>

#include "OutputStream.hpp"
#include "SerializeHelper.hpp"
#include "exception/IteratorException.hpp"

namespace supermap::io {

template <typename T, typename = std::enable_if_t<SerializeHelper<T>::isSerializable>>
class Writer {
  public:
    explicit Writer(std::unique_ptr<OutputStream> &&os)
        : os_(std::move(os)) {}

    static Writer toFile(const std::string &filename, bool append) {
        return Writer(std::make_unique<FileOutputStream>(filename, append));
    }

    static Writer toString(std::string &buffer, bool append) {
        return Writer(std::make_unique<StringOutputStream>(buffer, append));
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
