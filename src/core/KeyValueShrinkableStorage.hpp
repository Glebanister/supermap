#pragma once

#include <cstdint>
#include <vector>
#include <cassert>
#include <unordered_map>

#include "KeyIndex.hpp"
#include "KeyValue.hpp"
#include "IndexedData.hpp"
#include "SortedIndexedData.hpp"
#include "exception/SupermapException.hpp"

namespace supermap {

template <std::size_t KeyLen, std::size_t ValueLen>
struct StorageValueIgnorer {
    Key<KeyLen> key;
};

template <std::size_t KeyLen, std::size_t ValueLen>
class KeyValueShrinkableStorage {
  public:
    explicit KeyValueShrinkableStorage(const std::string &storageFilename,
                                       std::shared_ptr<io::FileManager> manager)
        : keyValueStorage_(storageFilename, manager) {
    }

    explicit KeyValueShrinkableStorage(
        const KeyValueShrinkableStorage &other, // old storage
        const std::string &storageFilename,
        const IndexedData<KeyIndex<KeyLen>> &) // actual index
        : keyValueStorage_(storageFilename, other.keyValueStorage_.getManager()) {
        throw NotImplementedException("KeyValueShrinkableStorage shrinking ctor");
    }

    void resetStorageWith(KeyValueShrinkableStorage &&) {
        throw NotImplementedException("KeyValueShrinkableStorage resetStorageWith");
    }

    std::uint64_t add(const Key<KeyLen> &key, ByteArray<ValueLen> &&value) {
        return keyValueStorage_.append({key, std::move(value)});
    }

    ByteArray<ValueLen> getValue(std::uint64_t index) {
        return keyValueStorage_.get(index).value;
    }

    io::InputIterator<StorageValueIgnorer<KeyLen, ValueLen>> getKeys() {
        return keyValueStorage_.template getCustomDataParser<StorageValueIgnorer<KeyLen, ValueLen>>();
    }

    std::string getBatchFileName(const std::size_t batchId, const std::string &shrinkFileSuffix) {
        return keyValueStorage_.getDataFileName()
            + shrinkFileSuffix
            + std::to_string(batchId);
    }

    SortedIndexedData<KeyIndex<KeyLen>> shrink(
        std::uint32_t shrinkBatchSize,
        const std::string &shrinkFileSuffix,
        const std::string &temporaryStorageName) {

        std::size_t batchesCount = (getKeysCount() + shrinkBatchSize - 1) / shrinkBatchSize;
        io::InputIterator<StorageValueIgnorer<KeyLen, ValueLen>> keyStream = getKeys();
        std::vector<io::FileManager::TemporaryFile> tempFies;
        std::vector<SortedIndexedData<KeyIndex<KeyLen>>> sortedBatchKeys;
        auto fileManager = keyValueStorage_.getManager();

        tempFies.reserve(batchesCount + 2);
        sortedBatchKeys.reserve(batchesCount);
        for (std::size_t batchId = 0; batchId < batchesCount; ++batchId) {
            const std::string batchFileName = getBatchFileName(batchId, shrinkFileSuffix);

            tempFies.push_back(fileManager->createTemporaryFile(batchFileName));

            std::vector<io::Enum<Key<KeyLen>>> thisBatchKeysRaw =
                keyStream.template collectWith<Key<KeyLen>>(
                    [](StorageValueIgnorer<KeyLen, ValueLen> &&svi) {
                        return svi.key;
                    },
                    shrinkBatchSize);
            std::vector<KeyIndex<KeyLen>> thisBatchKeys;
            thisBatchKeys.reserve(thisBatchKeysRaw.size());
            std::transform(thisBatchKeysRaw.begin(),
                           thisBatchKeysRaw.end(),
                           std::back_inserter(thisBatchKeys),
                           [](const io::Enum<Key<KeyLen>> &raw) {
                               return KeyIndex<KeyLen>{raw.elem, raw.index};
                           });

            SortedIndexedData<KeyIndex<KeyLen>> sortedBatch(
                std::move(thisBatchKeys),
                batchFileName,
                fileManager
            );

            assert(sortedBatch.getSize() > 0 && "Batch file can not be empty");
            sortedBatchKeys.push_back(std::move(sortedBatch));
        }

        SortedIndexedData<KeyIndex<KeyLen>> mergedIndex = SortedIndexedData(
            std::move(sortedBatchKeys),
            keyValueStorage_.getDataFileName() + shrinkFileSuffix,
            fileManager
        );
        resetStorageWith(KeyValueShrinkableStorage<KeyLen, ValueLen>(*this, temporaryStorageName, mergedIndex));
        return mergedIndex;
    }

    [[nodiscard]] std::uint64_t getKeysCount() const noexcept {
        return keyValueStorage_.getSize();
    }

  private:
    explicit KeyValueShrinkableStorage(std::uint64_t containedKeys,
                                       std::string storageFilename,
                                       std::shared_ptr<io::FileManager> manager)
        : keyValueStorage_(containedKeys,
                           storageFilename,
                           storageFilename,
                           manager) {}

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
