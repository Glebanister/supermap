#pragma once

#include <memory>

#include "InputStream.hpp"
#include "SerializeHelper.hpp"
#include "exception/IteratorException.hpp"

namespace supermap::io {

template <typename T, typename = std::enable_if_t<SerializeHelper<T>::isSerializable>>
class Parser {
  public:
    explicit Parser(std::unique_ptr<InputStream> &&input)
        : input_(std::move(input)) {}

    static Parser fromFile(const std::string &filename, std::size_t offset = 0) {
        return Parser(std::make_unique<FileInputStream>(filename, offset));
    }

    static Parser fromString(std::string data, std::size_t offset = 0) {
        return Parser(std::make_unique<StringInputStream>(data, offset));
    }

  public:
    Parser() = delete;
    Parser(const Parser &) = default;
    Parser(Parser &&) noexcept = default;

    [[nodiscard]] bool hasNext() const noexcept {
        return input_->availableBytes() >= SerializeHelper<T>::minimalSize;
    }

    T next() {
        return deserialize<T>(input_->get());
    }

    std::unique_ptr<InputStream> input_;
};

} // supermap::io
