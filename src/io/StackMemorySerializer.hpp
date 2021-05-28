#pragma once

#include "SerializeHelper.hpp"

namespace supermap::io {

template <typename T, typename = std::enable_if_t<std::is_default_constructible<T>::value>>
struct StackMemorySerializer : Serializable<true, sizeof(T)> {
    static constexpr std::size_t EACH_SIZE = sizeof(T);

    static void serialize(const T &value, std::ostream &os) {
        for (std::size_t i = 0; i < EACH_SIZE; ++i) {
            os.put(reinterpret_cast<const std::uint8_t *>(&value)[i]);
        }
    }

    static T deserialize(std::istream &is) {
        T obj;
        for (std::size_t i = 0; i < EACH_SIZE; ++i) {
            reinterpret_cast<std::uint8_t *>(&obj)[i] = is.get();
        }
        return obj;
    }
};

} // supermap::io
