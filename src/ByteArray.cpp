#include "ByteArray.hpp"

namespace supermap {

ByteArray::ByteArray(std::size_t length)
    : length_(length),
      data_(std::make_unique<std::uint8_t[]>(length)) {}

ByteArray::ByteArray(const ByteArray &other)
    : ByteArray(other.length_) {
    std::memcpy(data_.get(), other.data_.get(), length_);
}

ByteArray::ByteArray(ByteArray &&other) noexcept
    : length_(other.length_),
      data_(std::move(other.data_)) {}

std::size_t ByteArray::getLength() const noexcept {
    return length_;
}

std::uint8_t *ByteArray::getBytes() const noexcept {
    return data_.get();
}

char *ByteArray::getCharsPointer() const noexcept {
    return reinterpret_cast<char *>(data_.get());
}

} // supermap
