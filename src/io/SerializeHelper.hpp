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

template <bool IsDeserializable>
struct Deserializable {
    static constexpr bool isDeserializable = IsDeserializable;
};

template <std::size_t ExactDeserializedSize>
struct FixedDeserializedSize {
    static constexpr std::size_t exactDeserializedSize = ExactDeserializedSize;
};

template <typename T, typename = void>
struct FixedDeserializedSizeRegister {};

template <typename T, typename = void>
struct SerializeHelper : Serializable<false> {
    void serialize(const T &, std::ostream &) {
        // will not be called
        throw NotImplementedException();
    }
};

template <typename T, typename = void>
struct DeserializeHelper : Deserializable<false> {
    T deserialize(std::istream &) {
        // will not be called
        throw NotImplementedException();
    }
};

template <typename T, typename = std::enable_if_t<SerializeHelper<T>::isSerializable>>
inline void serialize(const T &value, std::ostream &os) {
    SerializeHelper<T>::serialize(value, os);
}

template <
    typename T,
    typename = std::enable_if_t<DeserializeHelper<T>::isDeserializable>>
inline T deserialize(std::istream &is) {
    return DeserializeHelper<T>::deserialize(is);
}

} // supermap::io
