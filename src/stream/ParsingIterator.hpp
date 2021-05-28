#pragma once

#include <memory>

#include "InputStream.hpp"
#include "SerializeHelper.hpp"
#include "exception/IteratorException.hpp"

namespace supermap::stream {

template <typename T, typename = std::enable_if_t<SerializeHelper<T>::isSerializable>>
class ParsingIterator {
  public:
    static ParsingIterator fromFile(const std::string &filename, std::size_t offset = 0) {
        return ParsingIterator(std::make_unique<FileInputStream>(filename, offset));
    }

    static ParsingIterator fromString(std::string data, std::size_t offset = 0) {
        return ParsingIterator(std::make_unique<StringInputStream>(data, offset));
    }

  public:
    ParsingIterator() = delete;
    ParsingIterator(const ParsingIterator &) = default;
    ParsingIterator(ParsingIterator &&) noexcept = default;

    [[nodiscard]] bool hasNext() const noexcept {
        return input_->availableBytes() >= SerializeHelper<T>::minimalSize;
    }

    T next() {
        return deserialize<T>(input_->get());
    }

  private:
    explicit ParsingIterator(std::unique_ptr<InputStream> &&input)
        : input_(std::move(input)) {}

    std::unique_ptr<InputStream> input_;
};

} // supermap::stream
