#pragma once

#include <iostream>

#include "SerializeHelper.hpp"

namespace supermap::io {

template <typename T>
struct ShallowSerializer : Serializable<true> {
    static void serialize(const T &value, std::ostream &os) {
        auto posBefore = os.tellp();
        os.write(reinterpret_cast<const char *>(&value), sizeof(T));
        if (os.tellp() - posBefore != sizeof(T)) {
            throw IOException(
                "Attempted to write " + std::to_string(sizeof(T)) +
                    " bytes, but only " + std::to_string(os.tellp() - posBefore) +
                    " succeeded"
            );
        }
    }
};

template <typename T, typename = std::enable_if_t<std::is_default_constructible<T>::value>>
struct ShallowDeserializer : Deserializable<true> {
    static T deserialize(std::istream &is) {
        T obj;
        is.read(reinterpret_cast<char *>(&obj), sizeof(T));
        return obj;
    }
};

template <typename T>
struct ShallowDeserializedSize : FixedDeserializedSize<sizeof(T)> {};

} // supermap::io
