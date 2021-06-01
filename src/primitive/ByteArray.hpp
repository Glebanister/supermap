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

    ByteArray(const ByteArray<Len> &other) : ByteArray() {
        std::memcpy(data_.get(), other.data_.get(), Len);
    }

    ByteArray(ByteArray<Len> &&other) noexcept
        : data_(std::move(other.data_)) {}

    ByteArray &operator=(const ByteArray<Len> &other) = delete;

    ByteArray &operator=(ByteArray<Len> &&other) noexcept {
        data_.reset(std::move(other.data_.release()));
        return *this;
    }

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

    bool operator==(const ByteArray<Len> &other) const {
        return std::memcmp(getCharsPointer(), other.getCharsPointer(), Len) == 0;
    }

  private:
    std::unique_ptr<std::uint8_t[]> data_;
};

namespace io {

template <std::size_t Len>
struct SerializeHelper<ByteArray<Len>> : Serializable<true> {
    static void serialize(const ByteArray<Len> &ar, std::ostream &os) {
        os.write(ar.getCharsPointer(), Len);
        if (!os.good()) {
            throw IOException("Unsuccessful serialization, expected to write " + std::to_string(Len) + " bytes");
        }
    }
};

template <std::size_t Len>
struct DeserializeHelper<ByteArray<Len>> : Deserializable<true> {
    static ByteArray<Len> deserialize(std::istream &is) {
        ByteArray<Len> ar;
        is.read(ar.getCharsPointer(), Len);
        if (!is.good()) {
            throw IOException(
                "Unsuccessful deserialization, expected to read " + std::to_string(Len) +
                    "bytes, but read only " + std::to_string(is.gcount()));
        }
        return ar;
    }
};

template <std::size_t Len>
struct FixedDeserializedSizeRegister<ByteArray<Len>> : FixedDeserializedSize<Len> {};

} // io

} // supermap
