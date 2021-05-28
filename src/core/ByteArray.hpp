#pragma once

#include <cstdint>
#include <memory>
#include <cstring>

#include "io/SerializeHelper.hpp"

#include "exception/IllegalArgumentException.hpp"

namespace supermap {

template <std::size_t Len>
class ByteArray {
  public:
    ByteArray()
        : data_(std::make_unique<std::uint8_t[]>(Len)) {}

    ByteArray(const ByteArray<Len> &other) {
        std::memcpy(data_.get(), other.data_.get(), Len);
    }

    ByteArray(ByteArray<Len> &&other) noexcept
        : data_(std::move(other.data_)) {}

    ByteArray &operator=(const ByteArray<Len> &other) = delete;
    ByteArray &operator=(ByteArray<Len> &&other) = delete;

    [[nodiscard]] std::uint8_t *getBytes() const noexcept {
        return data_.get();
    }

    [[nodiscard]] char *getCharsPointer() const noexcept {
        return reinterpret_cast<char *>(data_.get());
    }

    static ByteArray<Len> fromString(const std::string &str) {
        if (str.length() != Len) {
            throw IllegalArgumentException("String length can not be longer than byte array length");
        }
        ByteArray<Len> arr;
        for (std::size_t i = 0; i < str.length(); ++i) {
            arr.getCharsPointer()[i] = str[i];
        }
        return arr;
    }

    [[nodiscard]] std::string toString() const noexcept {
        std::string str;
        for (std::size_t i = 0; i < Len; ++i) {
            str += getCharsPointer()[i];
        }
        return str;
    }

  private:
    std::unique_ptr<std::uint8_t[]> data_;
};

namespace io {

template <std::size_t Len>
struct SerializeHelper<ByteArray<Len>> : Serializable<true> {
    void serialize(const ByteArray<Len> &ar, std::ostream &os) {
        if (std::size_t written = os.write(ar.getCharsPointer(), Len); written < Len) {
            throw IOException(
                "Unsuccessful serialization, expected to write " + std::to_string(Len) +
                    " bytes, but written only " + std::to_string(written));
        }
    }
};

template <std::size_t Len>
struct DeserializeHelper<ByteArray<Len>> : Deserializable<true, Len> {
    ByteArray<Len> deserialize(std::istream &is) {
        ByteArray<Len> ar;
        if (std::size_t read = is.read(ar.getCharsPointer(), Len); read < Len) {
            throw IOException(
                "Unsuccessful deserialization, expected to read " + std::to_string(Len) +
                    "bytes, but read only " + std::to_string(read));
        }
        return ar;
    }
};

} // io

} // supermap
