#pragma once

#include <cstdint>
#include <vector>
#include <cassert>

#include "KeyIndex.hpp"
#include "KeyValue.hpp"
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

  public:
    explicit KeyValueShrinkableStorage(const std::string &storageFilename,
                                       std::shared_ptr<io::FileManager> manager)
        : keyValueStorage_(storageFilename, std::move(manager)) {}

    std::uint64_t add(const Key<KeyLen> &key, ByteArray<ValueLen> &&value) {
        return keyValueStorage_.append({key, std::move(value)});
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
    IndexedData<KeyValue<KeyLen, ValueLen>> keyValueStorage_;
};

namespace io {

template <std::size_t KeyLen, std::size_t ValueLen>
struct DeserializeHelper<StorageValueIgnorer<KeyLen, ValueLen>> : Deserializable<true, KeyLen + ValueLen> {
    static StorageValueIgnorer<KeyLen, ValueLen> deserialize(std::istream &is) {
        StorageValueIgnorer<KeyLen, ValueLen> svi{io::deserialize<Key<KeyLen>>(is)};
        is.seekg(ValueLen, std::ios_base::cur);
        if (is.fail()) {
            throw IOException("StorageValueIgnorer deserialization fail: seekg failed");
        }
        return svi;
    }
};

} // io

} // supermap
