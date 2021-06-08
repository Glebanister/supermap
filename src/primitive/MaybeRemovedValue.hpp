#pragma once

namespace supermap {

/**
 * @brief Optional value container.
 * @tparam Value Maybe contained value.
 */
template <typename Value>
struct MaybeRemovedValue {
    Value value;
    bool removed;

    /**
     * @brief Parses string @p MaybeValue representation.
     * @param str String representation.
     * @return Non-removed @p MaybeValue.
     */
    static MaybeRemovedValue fromString(const std::string &str) {
        return {Value::fromString(str), false};
    }

    [[nodiscard]] std::string toString() const {
        if (removed) {
            throw KeyException("Value is removed");
        }
        return value.toString();
    }
};

template <typename T>
struct MaybeRemovedHelper {
    static constexpr bool isMaybeRemoved = false;
};

template <typename T>
struct MaybeRemovedHelper<MaybeRemovedValue<T>> {
    static constexpr bool isMaybeRemoved = true;
    using Type = T;
};

namespace io {

/**
 * @brief @p SerializeHelper template specialization for @p MaybeRemovedValue.
 * @tparam T Content type.
 */
template <typename T>
struct SerializeHelper<MaybeRemovedValue<T>> : Serializable<true> {
    static void serialize(const MaybeRemovedValue<T> &val, std::ostream &os) {
        io::serialize(val.value, os);
        io::serialize(val.removed, os);
    }
};

/**
 * @brief @p DeserializeHelper template specialization for @p MaybeRemovedValue.
 * @tparam T Content type.
 */
template <typename T>
struct DeserializeHelper<MaybeRemovedValue<T>> : Deserializable<true> {
    static MaybeRemovedValue<T> deserialize(std::istream &is) {
        return {
            io::deserialize<T>(is),
            io::deserialize<bool>(is)
        };
    }
};

/**
 * @brief @p FixedDeserializedSizeRegister template specialization for @p MaybeRemovedValue.
 * @tparam T Content type.
 */
template <typename T>
struct FixedDeserializedSizeRegister<MaybeRemovedValue<T>> : FixedDeserializedSize<
    FixedDeserializedSizeRegister<T>::exactDeserializedSize
        + FixedDeserializedSizeRegister<bool>::exactDeserializedSize
> {
};

} // io

} // supermap
