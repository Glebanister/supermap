#pragma once

#include <cstdint>
#include <vector>

#include "KeyIndex.hpp"
#include "IndexedData.hpp"
#include "exception/SupermapException.hpp"

namespace supermap {

template <std::size_t KeyLen, std::size_t ValueLen>
struct StorageValueIgnorer {
    Key<KeyLen> key;
};

template <std::size_t KeyLen, std::size_t ValueLen>
class KeyValueShrinkableStorage {
  public:
    struct KeyValue {
        Key<KeyLen> key;
        ByteArray<ValueLen> value;
    };

  public:
    explicit KeyValueShrinkableStorage(const std::string &storageFilename,
                                       std::shared_ptr<io::FileManager> manager)
        : keyValueStorage_(storageFilename, manager) {}

    std::uint64_t add(const Key<KeyLen> &key, ByteArray<ValueLen> &&value) {
        keyValueStorage_.append({key, std::move(value)});
    }

    ByteArray<ValueLen> getValue(std::uint64_t index) {
        return keyValueStorage_.get(index).value;
    }

    io::Parser<StorageValueIgnorer<KeyLen, ValueLen>> getKeys() {
        return keyValueStorage_.template getCustomDataParser<StorageValueIgnorer<KeyLen, ValueLen>>();
    }

    void shrink() {
        throw NotImplementedException();
    }

    [[nodiscard]] std::uint64_t getKeysCount() const noexcept {
        return keyValueStorage_.getSize();
    }

  private:
    IndexedData<KeyValue> keyValueStorage_;
};

namespace io {

template <std::size_t KeyLen, std::size_t ValueLen>
struct DeserializeHelper<StorageValueIgnorer<KeyLen, ValueLen>> : Deserializable<true, KeyLen + ValueLen> {
    static StorageValueIgnorer<KeyLen, ValueLen> deserialize(std::istream &is) {
        StorageValueIgnorer<KeyLen, ValueLen> svi{io::deserialize<Key<KeyLen>>(is)};
        is.seekg(sizeof(ValueLen), std::ios_base::cur);
        return svi;
    }
};

} // io

} // supermap
