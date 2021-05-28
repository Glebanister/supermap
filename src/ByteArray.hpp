#pragma once

#include <cstdint>
#include <memory>
#include <cstring>

namespace supermap {

class ByteArray {
  public:
    explicit ByteArray(std::size_t length);

    ByteArray(const ByteArray &other);

    ByteArray(ByteArray &&other) noexcept;

    ByteArray &operator=(const ByteArray &other) = delete;
    ByteArray &operator=(ByteArray &&other) = delete;

    [[nodiscard]] std::size_t getLength() const noexcept;

    [[nodiscard]] std::uint8_t *getBytes() const noexcept;

    [[nodiscard]] char *getCharsPointer() const noexcept;

  private:
    const std::size_t length_;
    std::unique_ptr<std::uint8_t[]> data_;
};

} // supermap
