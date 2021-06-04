#pragma once

#include <type_traits>
#include <ostream>
#include <istream>

#include "exception/SupermapException.hpp"

namespace supermap::io {

/**
 * @brief Flag if object is serializable.
 * @tparam IsSerializable value.
 */
template <bool IsSerializable>
struct Serializable {
    static constexpr bool isSerializable = IsSerializable;
};

/**
 * @brief Flag if object is deserializable.
 * @tparam IsDeserializable value.
 */
template <bool IsDeserializable>
struct Deserializable {
    static constexpr bool isDeserializable = IsDeserializable;
};

/**
 * @brief Container of @p ExactDeserializedSize for this type,
 * which tells exact size of object of this type after is
 * has been deserialized.
 * @tparam ExactDeserializedSize value.
 */
template <std::size_t ExactDeserializedSize>
struct FixedDeserializedSize {
    static constexpr std::size_t exactDeserializedSize = ExactDeserializedSize;
};

/**
 * @brief Register of all types that have @p exactDeserializedSize.
 * To deserialize any type, @p Register for this type must be declared.
 * @tparam T Deserialized type.
 */
template <typename T, typename = void>
struct FixedDeserializedSizeRegister {};

/**
 * @brief Container for @p serialize function. Must be declared
 * for any type that wants to be serialized.
 * @tparam T type of serialized object.
 */
template <typename T, typename = void>
struct SerializeHelper : Serializable<false> {
    void serialize(const T &, std::ostream &) {
        // will not be called
        throw NotImplementedException();
    }
};

/**
 * @brief Container for @p deserialize function. Must be declared
 * for any type that wants to be deserialized.
 * @tparam T type of deserialized object.
 */
template <typename T, typename = void>
struct DeserializeHelper : Deserializable<false> {
    T deserialize(std::istream &) {
        // will not be called
        throw NotImplementedException();
    }
};

/**
 * @brief Serializes @p value to the output stream @p os using @p SerializeHelper::serialize.
 * @tparam T type of serialized object.
 * @param value Object to serialize.
 * @param os Serialization output stream.
 */
template <typename T, typename = std::enable_if_t<SerializeHelper<T>::isSerializable>>
inline void serialize(const T &value, std::ostream &os) {
    SerializeHelper<T>::serialize(value, os);
}

/**
 * @brief Deserializes object of type @p T from input stream @p is using @p DeserializeHelper::deserialize.
 * @tparam T type of deserialized object.
 * @param is Deserialization input stream.
 * @return Deserialized object.
 */
template <
    typename T,
    typename = std::enable_if_t<DeserializeHelper<T>::isDeserializable>>
inline T deserialize(std::istream &is) {
    return DeserializeHelper<T>::deserialize(is);
}

} // supermap::io
