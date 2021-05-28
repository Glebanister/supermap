#pragma once

#include <array>

#include "exception/IllegalArgumentException.hpp"

namespace supermap {

template <std::size_t Len>
struct Key : std::array<std::uint8_t, Len> {
    using std::array<std::uint8_t, Len>::array;

    [[nodiscard]] std::string format() const noexcept {
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
};

} // supermap
