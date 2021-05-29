#pragma once

#include <memory>

#include "InputStream.hpp"
#include "SerializeHelper.hpp"
#include "exception/IteratorException.hpp"
#include "Iterator.hpp"

namespace supermap::io {

template <typename T, typename = std::enable_if_t<DeserializeHelper<T>::isDeserializable>>
class InputIterator : public Iterator<T> {
  public:
    explicit InputIterator(std::unique_ptr<InputStream> &&input)
        : input_(std::move(input)) {}

    static InputIterator fromFile(const std::string &filename, std::uint64_t offset = 0) {
        return InputIterator(std::make_unique<FileInputStream>(filename, offset));
    }

    static InputIterator fromString(std::string data, std::uint64_t offset = 0) {
        return InputIterator(std::make_unique<StringInputStream>(data, offset));
    }

  public:
    InputIterator() = delete;
    InputIterator(const InputIterator &) = default;
    InputIterator(InputIterator &&) noexcept = default;

    [[nodiscard]] bool hasNext() const noexcept override {
        return input_->availableBytes() >= DeserializeHelper<T>::minimalDeserializedSize;
    }

    T next() override {
        return deserialize<T>(input_->get());
    }

    std::unique_ptr<InputStream> input_;
};

} // supermap::io
