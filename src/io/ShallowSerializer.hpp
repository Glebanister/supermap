#pragma once

#include "SerializeHelper.hpp"

namespace supermap::io {

/**
 * @brief Template for SerializeHelper which serializes only shallow structure data.
 * @tparam T object type to be shallowly serialized.
 * Note, that no pointers are allowed in @p T.
 */
template <typename T>
struct ShallowSerializer : Serializable<true> {
    /**
     * @brief Serialized all @p value shallowly to the output stream @p os.
     * @param value Object to serialize.
     * @param os Serialization stream.
     */
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

/**
 * @brief Template for DeserializeHelper which deserializes object shallowly.
 * @tparam T type which is fine with being shallowly deserialized.
 * Note, that no pointers are allowed in @p T.
 */
template <typename T, typename = std::enable_if_t<std::is_default_constructible<T>::value>>
struct ShallowDeserializer : Deserializable<true> {
    /**
     * @brief Deserializes object from @p is shallowly, assuming it does not have any deep links
     * a.k.a. pointers. Their presence will cause undefined behavior.
     * @param is Deserialization stream.
     * @return Deserialized object.
     */
    static T deserialize(std::istream &is) {
        T obj;
        is.read(reinterpret_cast<char *>(&obj), sizeof(T));
        return obj;
    }
};

/**
 * @brief Type which indicates about shallowly deserialized object size.
 * @tparam T Deserialized object.
 */
template <typename T>
struct ShallowDeserializedSize : FixedDeserializedSize<sizeof(T)> {};

/**
 * @brief Default SerializeHelper for all integral types.
 * @tparam T An integral type.
 */
template <typename T>
struct SerializeHelper<T, std::enable_if_t<std::is_integral_v<T>>> : ShallowSerializer<T> {};

/**
 * @brief Default DeserializeHelper for all integral types.
 * @tparam T An integral type.
 */
template <typename T>
struct DeserializeHelper<T, std::enable_if_t<std::is_integral_v<T>>> : ShallowDeserializer<T> {};

/**
 * @brief Default FixedDeserializedSizeRegister for all integral types.
 * @tparam T An integral type.
 */
template <typename T>
struct FixedDeserializedSizeRegister<T, std::enable_if_t<std::is_integral_v<T>>> : ShallowDeserializedSize<T> {};

} // supermap::io
