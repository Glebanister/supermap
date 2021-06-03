#pragma once

namespace supermap {

template <typename Key, typename Value>
struct KeyValue {
    KeyValue(Key key_, Value value_)
        : key(std::move(key_)), value(std::move(value_)) {}

    KeyValue(const KeyValue &) = default;
    KeyValue(KeyValue &&) noexcept = default;
    KeyValue &operator=(const KeyValue &) = default;
    KeyValue &operator=(KeyValue &&) noexcept = default;

    Key key{};
    Value value{};

    bool equals(const KeyValue<Key, Value> &other) const {
        return key == other.key && value == other.value;
    }
};

namespace io {

template <typename Key, typename Value>
struct SerializeHelper<KeyValue<Key, Value>> : Serializable<true> {
    static void serialize(const KeyValue<Key, Value> &keyVal, std::ostream &os) {
        io::serialize(keyVal.key, os);
        io::serialize(keyVal.value, os);
    }
};

template <typename Key, typename Value>
struct DeserializeHelper<KeyValue<Key, Value>> : Deserializable<true> {
    static KeyValue<Key, Value> deserialize(std::istream &is) {
        return KeyValue<Key, Value>
            {
                io::deserialize<Key>(is),
                io::deserialize<Value>(is)
            };
    }
};

template <typename Key, typename Value>
struct FixedDeserializedSizeRegister<KeyValue<Key, Value>>
    : FixedDeserializedSize<
        FixedDeserializedSizeRegister<Key>::exactDeserializedSize
            + FixedDeserializedSizeRegister<Value>::exactDeserializedSize> {
};

} // io

} // supermap
