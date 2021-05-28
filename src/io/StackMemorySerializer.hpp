#pragma once

#include "SerializeHelper.hpp"

namespace supermap::io {

template <typename T>
struct StackMemorySerializer : Serializable<true> {
    static void serialize(const T &value, std::ostream &os) {
        for (std::size_t i = 0; i < sizeof(T); ++i) {
            os.put(reinterpret_cast<const std::uint8_t *>(&value)[i]);
        }
    }
};

template <typename T, typename = std::enable_if_t<std::is_default_constructible<T>::value>>
struct StackMemoryDeserializer : Deserializable<true, sizeof(T)> {
    static T deserialize(std::istream &is) {
        T obj;
        for (std::size_t i = 0; i < sizeof(T); ++i) {
            reinterpret_cast<std::uint8_t *>(&obj)[i] = is.get();
        }
        return obj;
    }
};

} // supermap::io
