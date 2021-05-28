#pragma once

namespace supermap {

template <std::size_t KeyLen, std::size_t ValueLen>
struct KeyValue {
    Key<KeyLen> key{};
    ByteArray<ValueLen> value{};

    bool operator==(const KeyValue<KeyLen, ValueLen> &other) const {
        return key == other.key && value == other.value;
    }
};

namespace io {

template <std::size_t KeyLen, std::size_t ValueLen>
struct SerializeHelper<KeyValue<KeyLen, ValueLen>> : Serializable<true> {
    static void serialize(const KeyValue<KeyLen, ValueLen> &keyVal, std::ostream &os) {
        io::serialize(keyVal.key, os);
        io::serialize(keyVal.value, os);
    }
};

template <std::size_t KeyLen, std::size_t ValueLen>
struct DeserializeHelper<KeyValue<KeyLen, ValueLen>> : Deserializable<true, KeyLen + ValueLen> {
    static KeyValue<KeyLen, ValueLen> deserialize(std::istream &is) {
        return KeyValue<KeyLen, ValueLen>
            {io::deserialize<Key < KeyLen>>(is),
            io::deserialize<ByteArray<ValueLen>>(is)};
    }
};

} // io

} // supermap
