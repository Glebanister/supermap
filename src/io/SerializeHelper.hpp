#pragma once

#include <type_traits>
#include <ostream>
#include <istream>

#include "exception/SupermapException.hpp"

namespace supermap::io {

template <bool IsSerializable>
struct Serializable {
    static constexpr bool isSerializable = IsSerializable;
};

template <bool IsDeserializable, std::size_t MinimalDeserializedSize>
struct Deserializable {
    static constexpr bool isDeserializable = IsDeserializable;
    static constexpr bool minimalDeserializedSize = MinimalDeserializedSize;
};

template <typename T>
struct SerializeHelper : Serializable<false> {
    void serialize(const T &, std::ostream &) {
        // will not be called
        throw NotImplementedException();
    }
};

template <typename T>
struct DeserializeHelper : Deserializable<false, 1> {
    T deserialize(std::istream &) {
        // will not be called
        throw NotImplementedException();
    }
};

template <typename T, typename = std::enable_if_t<SerializeHelper<T>::isSerializable>>
inline void serialize(const T &value, std::ostream &os) {
    SerializeHelper<T>::serialize(value, os);
}

template <typename T, typename = std::enable_if_t<SerializeHelper<T>::isSerializable>>
inline T deserialize(std::istream &os) {
    return DeserializeHelper<T>::deserialize(os);
}
} // supermap::io
