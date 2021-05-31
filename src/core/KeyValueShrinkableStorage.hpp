#pragma once

#include <cstdint>
#include <vector>
#include <cassert>
#include <unordered_map>

#include "primitive/Enum.hpp"
#include "primitive/KeyValue.hpp"
#include "IndexedData.hpp"
#include "SortedIndexedData.hpp"
#include "exception/SupermapException.hpp"

namespace supermap {

template <std::size_t KeyLen, std::size_t ValueLen>
struct StorageValueIgnorer {
    Key<KeyLen> key;
};

template <std::size_t KeyLen, std::size_t ValueLen>
class KeyValueShrinkableStorage : public IndexedData<KeyValue<KeyLen, ValueLen>> {
  public:
    using IndexedData<KeyValue<KeyLen, ValueLen>>::get;
    using IndexedData<KeyValue<KeyLen, ValueLen>>::getDataFileName;
    using IndexedData<KeyValue<KeyLen, ValueLen>>::getManager;
    using IndexedData<KeyValue<KeyLen, ValueLen>>::getCustomDataParser;
    using IndexedData<KeyValue<KeyLen, ValueLen>>::getSize;
    using IndexedData<KeyValue<KeyLen, ValueLen>>::append;

  public:
    explicit KeyValueShrinkableStorage(const std::string &storageFilename,
                                       std::shared_ptr<io::FileManager> manager)
        : IndexedData<KeyValue<KeyLen, ValueLen>>(storageFilename, manager) {
    }

    explicit KeyValueShrinkableStorage(
        const KeyValueShrinkableStorage &other,
        const std::string &storageFilename,
        const IndexedData<Enum<Key<KeyLen>>> &)
        : IndexedData<KeyValue<KeyLen, ValueLen>>(storageFilename, other.getManager()) {
        throw NotImplementedException("KeyValueShrinkableStorage shrinking ctor");
    }

    void resetStorageWith(KeyValueShrinkableStorage &&) {
        throw NotImplementedException("KeyValueShrinkableStorage resetStorageWith");
    }

    std::uint64_t add(const Key<KeyLen> &key, ByteArray<ValueLen> &&value) {
        return append({key, std::move(value)});
    }

    ByteArray<ValueLen> getValue(std::uint64_t index) {
        return get(index).value;
    }

    io::InputIterator<StorageValueIgnorer<KeyLen, ValueLen>> getKeys() {
        return IndexedData<KeyValue<KeyLen, ValueLen>>::template getCustomDataParser<StorageValueIgnorer<KeyLen,
                                                                                                         ValueLen>>();
    }

    std::string getBatchFileName(std::size_t batchId, const std::string &shrinkFileSuffix) {
        return getDataFileName()
            + shrinkFileSuffix
            + std::to_string(batchId);
    }

    SortedIndexedData<Enum<Key<KeyLen>>> shrink(
        std::uint32_t shrinkBatchSize,
        const std::string &shrinkFileSuffix,
        const std::string &temporaryStorageName) {

        std::size_t batchesCount = (getSize() + shrinkBatchSize - 1) / shrinkBatchSize;
        io::InputIterator<StorageValueIgnorer<KeyLen, ValueLen>> keyStream = getKeys();
        std::vector<io::FileManager::TemporaryFile> tempFies;
        std::vector<SortedIndexedData<Enum<Key<KeyLen>>>> sortedBatchKeys;
        auto fileManager = getManager();

        tempFies.reserve(batchesCount + 2);
        sortedBatchKeys.reserve(batchesCount);
        for (std::size_t batchId = 0; batchId < batchesCount; ++batchId) {
            const std::string batchFileName = getBatchFileName(batchId, shrinkFileSuffix);

            tempFies.push_back(fileManager->createTemporaryFile(batchFileName));

            SortedIndexedData<Enum<Key<KeyLen>>> sortedBatch(
                keyStream.template collectWith<Key<KeyLen>>(
                    [](StorageValueIgnorer<KeyLen, ValueLen> &&svi) {
                        return svi.key;
                    },
                    shrinkBatchSize),
                batchFileName,
                fileManager
            );

            assert(sortedBatch.getSize() > 0 && "Batch file can not be empty");
            sortedBatchKeys.push_back(std::move(sortedBatch));
        }

        SortedIndexedData<Enum<Key<KeyLen>>> mergedIndex = SortedIndexedData(
            std::move(sortedBatchKeys),
            getDataFileName() + shrinkFileSuffix,
            fileManager
        );
        resetStorageWith(KeyValueShrinkableStorage<KeyLen, ValueLen>(*this, temporaryStorageName, mergedIndex));
        return mergedIndex;
    }

  private:
    explicit KeyValueShrinkableStorage(std::uint64_t containedKeys,
                                       std::string storageFilename,
                                       std::shared_ptr<io::FileManager> manager)
        : IndexedData<KeyValue<KeyLen, ValueLen>>(containedKeys,
                                                  storageFilename,
                                                  storageFilename,
                                                  manager) {}
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
