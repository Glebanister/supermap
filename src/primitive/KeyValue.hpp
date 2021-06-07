#pragma once

namespace supermap {

/**
 * @brief Pair of key and value.
 * @tparam Key any key type.
 * @tparam Value any value type.
 */
template <typename Key, typename Value>
struct KeyValue {
    using KeyType = Key;
    using ValueType = Value;

    KeyValue(Key key_, Value value_)
        : key(std::move(key_)), value(std::move(value_)) {}

    KeyValue(const KeyValue &) = default;
    KeyValue(KeyValue &&) noexcept = default;
    KeyValue &operator=(const KeyValue &) = default;
    KeyValue &operator=(KeyValue &&) noexcept = default;

    Key key{};
    Value value{};

    /**
     * @brief Compares key and value equality with @p operator==.
     * @param other Compared KeyValue pair.
     * @return If keys and values are equal.
     */
    bool equals(const KeyValue<Key, Value> &other) const {
        return key == other.key && value == other.value;
    }

    /**
     * @return If keys are equal.
     */
    bool operator==(const KeyValue<Key, Value> &other) const {
        return key == other.key;
    }

    /**
     * @return If this key less then other key.
     */
    bool operator<(const KeyValue<Key, Value> &other) const {
        return key < other.key;
    }
};

namespace io {

/**
 * @brief @p SerializeHelper specialization for @p KeyValue.
 * @tparam Key
 * @tparam Value
 */
template <typename Key, typename Value>
struct SerializeHelper<KeyValue<Key, Value>> : Serializable<true> {
    /**
     * @brief Serializes @p keyVal to the output stream @p os.
     * Key and value are written consecutively: first the key and then the value.
     * @param keyVal Serialized key-value pair.
     * @param os Serialization stream.
     */
    static void serialize(const KeyValue<Key, Value> &keyVal, std::ostream &os) {
        io::serialize(keyVal.key, os);
        io::serialize(keyVal.value, os);
    }
};

/**
 * @brief @p DeserializeHelper template specialization for @p KeyValue.
 * @tparam Key key type.
 * @tparam Value value type.
 */
template <typename Key, typename Value>
struct DeserializeHelper<KeyValue<Key, Value>> : Deserializable<true> {
    /**
     * @brief Deserializes @p KeyValue from input stream @p is.
     * @param is Deserialization stream.
     * Key and value are must be arranged sequentially: first key and then value.
     * @return Deserialized key-value pair.
     */
    static KeyValue<Key, Value> deserialize(std::istream &is) {
        return KeyValue<Key, Value>
            {
                io::deserialize<Key>(is),
                io::deserialize<Value>(is)
            };
    }
};

/**
 * @brief @p FixedDeserializedSizeRegister template specialization for @p KeyValue.
 * Sets @p exactDeserializedSize as the sum of the same fields in registrars of Key and Value types, if such exist.
 * @tparam Key key type.
 * @tparam Value value type.
 */
template <typename Key, typename Value>
struct FixedDeserializedSizeRegister<KeyValue<Key, Value>>
    : FixedDeserializedSize<
        FixedDeserializedSizeRegister<Key>::exactDeserializedSize
            + FixedDeserializedSizeRegister<Value>::exactDeserializedSize> {
};

} // io

} // supermap
