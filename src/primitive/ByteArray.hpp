#pragma once

#include <cstdint>
#include <memory>
#include <cstring>

#include "io/SerializeHelper.hpp"
#include "exception/IllegalArgumentException.hpp"

namespace supermap {

/**
 * @brief An array that stores a fixed number of bytes in the heap.
 * @tparam Len @p ByteArray length.
 */
template <std::size_t Len>
class ByteArray {
  public:
    /**
     * @brief Creates an empty @p ByteArray. Note that memory will be allocated,
     * but won't be initialized.
     */
    ByteArray()
        : data_(std::make_unique<std::uint8_t[]>(Len)) {}

    /**
     * @brief Copy ctor.
     * @param other Copied @p ByteArray.
     */
    ByteArray(const ByteArray<Len> &other) : ByteArray() {
        std::memcpy(data_.get(), other.data_.get(), Len);
    }

    /**
     * @brief Move ctor.
     * @param other Moved @p ByteArray.
     */
    ByteArray(ByteArray<Len> &&other) noexcept
        : data_(std::move(other.data_)) {}

    /**
     * @brief Copy assignment operator.
     * @param other Copied @p ByteArray.
     * @return Assigned @p ByteArray reference.
     */
    ByteArray &operator=(const ByteArray<Len> &other) {
        std::memcpy(data_.get(), other.data_.get(), Len);
        return *this;
    }

    /**
     * @brief Move assignment operator.
     * @param other Moved @p ByteArray.
     * @return Assigned @p ByteArray reference.
     */
    ByteArray &operator=(ByteArray<Len> &&other) noexcept {
        data_.reset(std::move(other.data_.release()));
        return *this;
    }

    /**
     * @brief Accesses associated memory pointer.
     * The length of the returned array corresponds to the template parameter.
     * @return Memory pointer (@p std::uint8_t).
     */
    [[nodiscard]] std::uint8_t *getBytes() const noexcept {
        return data_.get();
    }

    /**
     * @brief Same as @p ByteArray::getBytes, memory pointer is being casted to @p char*.
     * @return Memory pointer (@p char).
     */
    [[nodiscard]] char *getCharsPointer() const noexcept {
        return reinterpret_cast<char *>(data_.get());
    }

    /**
     * @brief Creates @p ByteArray of bytes that contain the @p str.
     * @param str The string from whose data you want to make an array.
     * The length of must match the template parameter.
     * @return ByteArray with copy of @p str data.
     * @throws IllegalArgumentException if @p str length is not equals to @p Len.
     */
    static ByteArray<Len> fromString(const std::string &str) {
        if (str.length() != Len) {
            throw IllegalArgumentException(
                "String length can not be different to template size parameter, expected " + std::to_string(Len));
        }
        ByteArray<Len> arr;
        for (std::size_t i = 0; i < str.length(); ++i) {
            arr.getCharsPointer()[i] = str[i];
        }
        return arr;
    }

    /**
     * @brief Converts bytes from array to @p std::string. All data is being copied.
     * @return string of length @p Len with all @p ByteArray data.
     */
    [[nodiscard]] std::string toString() const {
        std::string str;
        for (std::size_t i = 0; i < Len; ++i) {
            str += getCharsPointer()[i];
        }
        return str;
    }

    /**
     * @brief Compares equality of this ByteArray to @other.
     * @param other Compared @p ByteArray.
     * @returnp @p true if and only if memory sections are fully match.
     */
    bool operator==(const ByteArray<Len> &other) const {
        return std::memcmp(getCharsPointer(), other.getCharsPointer(), Len) == 0;
    }

  private:
    std::unique_ptr<std::uint8_t[]> data_;
};

namespace io {

/**
 * @brief @p SerializeHelper template specialization for @p ByteArray.
 * @tparam Len @p ByteArray length.
 */
template <std::size_t Len>
struct SerializeHelper<ByteArray<Len>> : Serializable<true> {
    /**
     * @brief Serializes all memory from ByteArray @p ar to output stream @p os.
     * @param ar Serialized array.
     * @param os Serialization output stream.
     * @throws IOException if output stream is not good after writing.
     */
    static void serialize(const ByteArray<Len> &ar, std::ostream &os) {
        os.write(ar.getCharsPointer(), Len);
        if (!os.good()) {
            throw IOException("Unsuccessful serialization, expected to write " + std::to_string(Len) + " bytes");
        }
    }
};

/**
 * @brief @p DeserializeHelper template specialization for @p ByteArray.
 * @tparam Len @p ByteArray length.
 */
template <std::size_t Len>
struct DeserializeHelper<ByteArray<Len>> : Deserializable<true> {
    /**
     * @brief Deserializes @p Len bytes from input input stream @p is and creates @p ByteArray.
     * @param is Deserialization input stream.
     * @return Deserialized @p ByteArray.
     * @throws IOException if input stream is not good after reading.
     */
    static ByteArray<Len> deserialize(std::istream &is) {
        ByteArray<Len> ar;
        is.read(ar.getCharsPointer(), Len);
        if (!is.good()) {
            throw IOException(
                "Unsuccessful deserialization, expected to read " + std::to_string(Len) +
                    " bytes, but read only " + std::to_string(is.gcount()));
        }
        return ar;
    }
};

/**
 * @brief @p FixedDeserializedSizeRegister template specialization for @p ByteArray.
 * @tparam Len array length.
 */
template <std::size_t Len>
struct FixedDeserializedSizeRegister<ByteArray<Len>> : FixedDeserializedSize<Len> {};

} // io

} // supermap
