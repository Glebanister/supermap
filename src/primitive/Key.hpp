#pragma once

#include <array>
#include <cstring>
#include <cassert>

#include "exception/IllegalArgumentException.hpp"
#include "io/SerializeHelper.hpp"
#include "io/ShallowSerializer.hpp"

namespace supermap {

template <std::size_t Len>
struct Key : public std::array<std::uint8_t, Len> {
    Key() = default;
    Key(const Key &) = default;
    Key(Key &&) noexcept = default;
    Key &operator=(const Key &) = default;
    Key &operator=(Key &&) noexcept = default;

    using std::array<std::uint8_t, Len>::array;

    [[nodiscard]] std::string toString() const noexcept {
        std::string result;
        for (std::size_t i = 0; i < Len; ++i) {
            result += (*this)[i];
        }
        return result;
    }

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

    bool operator<(const Key<Len> &other) {
        return std::memcmp(this->data(), other.data(), Len) < 0;
    }
};

namespace io {

template <std::size_t KeyLen>
struct SerializeHelper<Key<KeyLen>> : ShallowSerializer<Key<KeyLen>> {};

template <std::size_t KeyLen>
struct DeserializeHelper<Key<KeyLen>> : ShallowDeserializer<Key<KeyLen>> {};

template <std::size_t KeyLen>
struct FixedDeserializedSizeRegister<Key<KeyLen>> : FixedDeserializedSize<KeyLen> {};

} // io

} // supermap
