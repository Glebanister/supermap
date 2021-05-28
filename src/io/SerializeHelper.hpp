#pragma once

#include <type_traits>
#include <ostream>
#include <istream>

namespace supermap::io {

template <bool IsSerializable, std::size_t MinimalSize = 1>
struct Serializable {
    static constexpr bool isSerializable = IsSerializable;
    static constexpr bool minimalSize = MinimalSize;
};

template <std::size_t Size>
struct EstimateSerializedSize : std::integral_constant<std::size_t, Size> {
};

template <typename T>
struct SerializeHelper : Serializable<false> {
};

template <typename T, typename = std::enable_if_t<SerializeHelper<T>::isSerializable>>
inline void serialize(const T &value, std::ostream &os) {
    SerializeHelper<T>::serialize(value, os);
}

template <typename T, typename = std::enable_if_t<SerializeHelper<T>::isSerializable>>
inline T deserialize(std::istream &os) {
    return SerializeHelper<T>::deserialize(os);
}
} // supermap::io
