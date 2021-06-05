#pragma once

#include "io/InputIterator.hpp"
#include "io/SerializeHelper.hpp"
#include "io/TemporaryFile.hpp"
#include "SortedSingleFileIndexedStorage.hpp"
#include "primitive/KeyValue.hpp"

namespace supermap {

/**
 * @brief Dedicated to skip value deserialization during storage reading.
 * @tparam Key key type.
 * @tparam ValueLen value type.
 */
template <typename Key, typename ValueLen>
struct StorageValueIgnorer {
    Key key;
};

/**
 * @brief Storage, which contains keys and associated values.
 * Consists of two parts: the one, where keys are sorted and unique
 * and the one where there is no particular order.
 * @tparam Key key type.
 * @tparam Value value type.
 * @tparam IndexT type of storage index.
 */
template <
    typename Key,
    typename Value,
    typename IndexT,
    typename IndexRegisterInfo
>
class KeyValueShrinkableStorage : public IndexedStorage<KeyValue<Key, Value>,
                                                        IndexT,
                                                        VoidRegister<KeyValue<Key, Value>>> {
    using KV = KeyValue<Key, Value>;
    using KVS = IndexedStorage<KV, IndexT, VoidRegister<KV>>;
    using KVS::getItemsCount;
    using KVS::getRegister;
    using KeyIndex = KeyValue<Key, IndexT>;
    using KeyIndexStorage = SortedSingleFileIndexedStorage<KeyIndex, IndexT, IndexRegisterInfo>;
    using ValueIgnorer = StorageValueIgnorer<Key, Value>;

  public:
    /**
     * @brief Creates an empty storage.
     * @param notSortedStorageFilename Name of file where not sorted key-values will be stored.
     * @param sortedStorageFilename Name of file where shuffled key-values will be stored.
     * @param fileManager Shared access to the file manager.
     */
    explicit KeyValueShrinkableStorage(
        const std::string &notSortedStorageFilename,
        const std::string &sortedStorageFilename,
        std::shared_ptr<io::FileManager> fileManager
    ) : sortedStorage_(sortedStorageFilename, fileManager),
        notSortedStorage_(notSortedStorageFilename, fileManager) {
    }

    /**
     * @brief Creates new storage, copying values from @p oldStorage.
     * Those value indexes will be taken from @p actualIndex in
     * sorted order. New all new key-values will be sorted
     * (if @p actualIndex is sorted), so all key-values will
     * be in sorted storage. Not sorted storage will be empty.
     * @param oldStorage Storage, from where all values will be taken from.
     * @param actualIndex Indexes of actual values.
     * @param notSortedStorageFilename New not sorted storage filename.
     * @param sortedStorageFilename New sorted storage filename.
     * @param indexBatchSize The size of the batch of
     * @p KeyValue<T,IndexT> objects that are simultaneously stored in RAM.
     */
    explicit KeyValueShrinkableStorage(
        const KeyValueShrinkableStorage &oldStorage,
        const KeyIndexStorage &actualIndex,
        const std::string &notSortedStorageFilename,
        const std::string &sortedStorageFilename,
        std::size_t indexBatchSize
    ) : sortedStorage_(sortedStorageFilename, oldStorage.getFileManager()),
        notSortedStorage_(notSortedStorageFilename, oldStorage.getFileManager()) {
        auto indexIterator = actualIndex.getDataIterator();
        io::OutputIterator<KV> keyValueOutputIterator
            = getFileManager()->template getOutputIterator<KV>(
                sortedStorage_.getStorageFilePath(),
                false
            );
        std::vector<KeyIndex> currentBatchIndex = indexIterator.collect(indexBatchSize);
        while (!currentBatchIndex.empty()) {
            sortedStorage_.appendAll(
                currentBatchIndex.begin(),
                currentBatchIndex.end(),
                [&](const KeyIndex &keyIndex) {
                    return KeyValue{
                        keyIndex.key,
                        oldStorage.get(keyIndex.value).value
                    };
                }
            );
            currentBatchIndex = indexIterator.collect(indexBatchSize);
        }
        keyValueOutputIterator.flush();
    }

    /**
     * @brief Replaces @p this storage with @p other. @p other will be @p moved-from.
     * @param other Replacement.
     */
    void resetWith(KeyValueShrinkableStorage &&other) {
        sortedStorage_.resetWith(std::move(other.sortedStorage_));
        notSortedStorage_.resetWith(std::move(other.notSortedStorage_));
    }

    /**
     * @return Shared with this storage file manager access.
     */
    [[nodiscard]] std::shared_ptr<io::FileManager> getFileManager() const noexcept {
        return sortedStorage_.getFileManager();
    }

    /**
     * @return File accosted with sorted storage.
     */
    [[nodiscard]] std::shared_ptr<io::TemporaryFile> shareSortedStorageFile() const noexcept {
        return sortedStorage_.shareStorageFile();
    }

    /**
     * @return File accosted with not sorted storage.
     */
    [[nodiscard]] std::shared_ptr<io::TemporaryFile> shareNotSortedStorageFile() const noexcept {
        return notSortedStorage_.shareStorageFile();
    }

    /**
     * @brief Appends @p item to the storage. It can not be easily appended
     * to the sorted storage, hence, it is being appended to the not sorted storage.
     * New value is considered to be the most relevant for key.
     * @param item Object to append.
     */
    void append(std::unique_ptr<KV> &&item) override {
        notSortedStorage_.append(std::move(item));
    }

    /**
     * @brief Gets key-value pair which has index @p index in this storage.
     * Behavior is undefined if index overflows storage size.
     * @param index Index of key-value pair to read.
     * @return Read key-value pair.
     */
    KV get(IndexT index) const override {
        if (index >= sortedStorage_.getItemsCount()) {
            return notSortedStorage_.get(index - sortedStorage_.getItemsCount());
        }
        return sortedStorage_.get(index);
    }

    /**
     * @return Number of key-value pairs in storage (sum of sorted and not sorted storage sizes).
     */
    [[nodiscard]] IndexT getItemsCount() const noexcept override {
        return sortedStorage_.getItemsCount() + notSortedStorage_.getItemsCount();
    }

    /**
     * @return Number of key-value pairs in sorted storage.
     */
    [[nodiscard]] IndexT getSortedItemsCount() const noexcept {
        return sortedStorage_.getItemsCount();
    }

    /**
     * @return Number of key-value pairs in not sorted storage.
     */
    [[nodiscard]] IndexT getNotSortedItemsCount() const noexcept {
        return notSortedStorage_.getItemsCount();
    }

    /**
     * @return Sorted keys input iterator.
     */
    io::InputIterator<ValueIgnorer, IndexT> getSortedKeys() {
        return sortedStorage_.template getCustomDataIterator<ValueIgnorer>();
    }

    /**
     * @return Not sorted keys input iterator.
     */
    io::InputIterator<ValueIgnorer, IndexT> getNotSortedKeys() {
        return notSortedStorage_.template getCustomDataIterator<ValueIgnorer>();
    }

    /**
     * @return Sorted entries input iterator.
     */
    io::InputIterator<KV, IndexT> getSortedEntries() {
        return sortedStorage_.getDataIterator();
    }

    /**
     * @return Not sorted entries input iterator.
     */
    io::InputIterator<KV, IndexT> getNotSortedEntries() {
        return notSortedStorage_.getDataIterator();
    }

    /**
     * @brief Shrinks not sorted storage. After command completion
     * not sorted storage is empty and there are the most relevant
     * values are paired with keys in sorted storage.
     * @param shrinkBatchSize The size of the batch of
     * @p KeyValue<T,IndexT> objects that are simultaneously stored in RAM.
     * @param newIndexFileName File, where new index with the most relevant key
     * value positions will be stored.
     * @return Sorted storage of @p KeyValue<Key,IndexT>
     */
    [[nodiscard]] KeyIndexStorage shrink(IndexT shrinkBatchSize, const std::string &newIndexFileName) {

        const std::string shrinkFilenamePrefix = "shrink";
        const std::string tempSortedIndexFilename = shrinkFilenamePrefix + "-sorted-keys";

        std::size_t batchesCount = (notSortedStorage_.getItemsCount() + shrinkBatchSize - 1) / shrinkBatchSize;
        auto notSortedKeysStream = getNotSortedKeys();
        std::vector<std::shared_ptr<io::TemporaryFile>> tempFilesLock;
        std::vector<KeyIndexStorage> sortedBatches;
        sortedBatches.reserve(batchesCount + 1);
        tempFilesLock.reserve(batchesCount + 1);

        KeyIndexStorage exportedKeys
            = exportKeys(shrinkBatchSize, tempSortedIndexFilename);
        tempFilesLock.push_back(exportedKeys.shareStorageFile());

        sortedBatches.push_back(std::move(exportedKeys));
        IndexT sortedStorageSize = sortedStorage_.getItemsCount();
        for (std::size_t batchI = 0; batchI < batchesCount; ++batchI) {
            const std::string batchFileName = shrinkFilenamePrefix + "-batch-" + std::to_string(batchI);
            std::vector<KeyIndex> notSortedKeys = notSortedKeysStream.collectWith(
                [sortedStorageSize](ValueIgnorer &&svi, IndexT index) {
                    return KeyIndex{std::move(svi.key), index + sortedStorageSize};
                },
                shrinkBatchSize
            );
            KeyIndexStorage sortedBatch(
                notSortedKeys.begin(),
                notSortedKeys.end(),
                false,
                batchFileName,
                getFileManager(),
                [](const KeyIndex &a, const KeyIndex &b) { return a.key < b.key; },
                [](const KeyIndex &a, const KeyIndex &b) { return a.key == b.key; }
            );
            tempFilesLock.push_back(sortedBatch.shareStorageFile());
            assert(sortedBatch.getItemsCount() > 0 && "Batch file can not be empty");
            sortedBatches.push_back(std::move(sortedBatch));
        }

        KeyIndexStorage updatedIndex(
            sortedBatches,
            newIndexFileName,
            getFileManager(),
            shrinkBatchSize
        );

        resetWith(KeyValueShrinkableStorage(
            *this,
            updatedIndex,
            shrinkFilenamePrefix + "-new-not-sorted",
            shrinkFilenamePrefix + "-new-sorted",
            shrinkBatchSize
        ));

        KeyIndexStorage actualExportedKeys
            = exportKeys(shrinkBatchSize, shrinkFilenamePrefix + "-actual-index");
        tempFilesLock.push_back(actualExportedKeys.shareStorageFile());
        updatedIndex.resetWith(std::move(actualExportedKeys));
        return updatedIndex;
    }

  private:
    /**
     * @brief Exports all pairs key-index to file with name @p keysFilename.
     * @param batchSize The size of the batch of
     * @p KeyValue<T,IndexT> objects that are simultaneously stored in RAM.
     * @param keysFilename Name of export file.
     * @return Storage instance.
     */
    KeyIndexStorage exportKeys(IndexT batchSize, const std::string &keysFilename) {
        KeyIndexStorage sortedIndex(keysFilename, getFileManager());
        auto it = sortedStorage_.template getCustomDataIterator<ValueIgnorer>();
        auto keys = it.collectWith([](ValueIgnorer &&kvi, IndexT ind) {
            return KeyIndex{std::move(kvi.key), ind};
        }, batchSize);
        while (!keys.empty()) {
            sortedIndex.appendAll(keys.cbegin(), keys.cend());
            keys = it.collectWith([](ValueIgnorer &&kvi, IndexT ind) {
                return KeyIndex{std::move(kvi.key), ind};
            }, batchSize);
        }
        return sortedIndex;
    }

    SortedSingleFileIndexedStorage<KV, IndexT, VoidRegister<KV>> sortedStorage_;
    SingleFileIndexedStorage<KV, IndexT, VoidRegister<KV>> notSortedStorage_;
};

namespace io {

/**
 * @brief @p DeserializeHelper template specialization for @p StorageValueIgnorer.
 * @tparam Key key type.
 * @tparam Value value type.
 */
template <typename Key, typename Value>
struct DeserializeHelper<StorageValueIgnorer<Key, Value>> : Deserializable<true> {
    /**
     * @brief Deserializes @p StorageValueIgnorer from input stream, reading key,
     * ignoring the value.
     * @param is Deserialization input stream.
     * @return StorageValueIgnorer instance.
     */
    static StorageValueIgnorer<Key, Value> deserialize(std::istream &is) {
        StorageValueIgnorer<Key, Value> svi{io::deserialize<Key>(is)};
        is.seekg(FixedDeserializedSizeRegister<Value>::exactDeserializedSize, std::ios_base::cur);
        if (is.fail()) {
            throw IOException("StorageValueIgnorer deserialization fail: seekg failed");
        }
        return svi;
    }
};

/**
 * @brief Register of @p StorageValueIgnorer deserialized size.
 * @tparam Key key type.
 * @tparam Value value type.
 */
template <typename Key, typename Value>
struct FixedDeserializedSizeRegister<StorageValueIgnorer<Key, Value>>
    : FixedDeserializedSize<FixedDeserializedSizeRegister<Key>::exactDeserializedSize
                                + FixedDeserializedSizeRegister<Value>::exactDeserializedSize> {
};

} // io

} // supermap
