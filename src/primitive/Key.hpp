#pragma once

#include <array>
#include <cstring>
#include <cassert>

#include "exception/IllegalArgumentException.hpp"
#include "io/SerializeHelper.hpp"
#include "io/ShallowSerializer.hpp"

namespace supermap {

/**
 * @brief An array which contains @p Len bytes in-place (not in the heap memory).
 * @tparam Len array length.
 */
template <std::size_t Len>
struct Key : public std::array<std::uint8_t, Len> {
    Key() = default;
    Key(const Key &) = default;
    Key(Key &&) noexcept = default;
    Key &operator=(const Key &) = default;
    Key &operator=(Key &&) noexcept = default;

    using std::array<std::uint8_t, Len>::array;

    /**
     * @brief Creates string from Key data copying inner data to new @p std::string.
     * @return String which length corresponds to the template @p Len parameter.
     */
    [[nodiscard]] std::string toString() const noexcept {
        std::string result;
        for (std::size_t i = 0; i < Len; ++i) {
            result += (*this)[i];
        }
        return result;
    }

    /**
     * @brief Creates new Key from its string representation.
     * @param strKey String Key representation. Note, that @p strKey length must be same as @p Len.
     * @return Created @p Key.
     * @throws IllegalArgumentException if @p strKey length is not the same as @p Len.
     */
    static Key<Len> fromString(const std::string &strKey) {
        if (strKey.length() != Len) {
            throw IllegalArgumentException(
                "String key length can not be greater than template size parameter");
        }

        Key<Len> arrKey;
        for (std::size_t i = 0; i < strKey.length(); ++i) {
            arrKey[i] = strKey[i];
        }
        for (std::size_t i = strKey.length(); i < Len; ++i) {
            arrKey[i] = 0;
        }

        return arrKey;
    }

    /**
     * @brief Compares array to other.
     * @param other Compared array.
     * @return if @p this array data is lexicographically less then @p other data.
     */
    bool operator<(const Key<Len> &other) {
        return std::memcmp(this->data(), other.data(), Len) < 0;
    }
};

namespace io {

/**
 * @brief @p SerializeHelper template specialization for @p Key.
 * @tparam KeyLen @p Key length.
 */
template <std::size_t KeyLen>
struct SerializeHelper<Key<KeyLen>> : ShallowSerializer<Key<KeyLen>> {};

template <std::size_t KeyLen>
struct DeserializeHelper<Key<KeyLen>> : ShallowDeserializer<Key<KeyLen>> {};

template <std::size_t KeyLen>
struct FixedDeserializedSizeRegister<Key<KeyLen>> : FixedDeserializedSize<KeyLen> {};

} // io

} // supermap
