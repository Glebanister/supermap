#pragma once

#include "io/InputIterator.hpp"
#include "io/SerializeHelper.hpp"
#include "io/TemporaryFile.hpp"
#include "SortedSingleFileIndexedStorage.hpp"
#include "primitive/KeyValue.hpp"

namespace supermap {

template <typename Key, typename ValueLen>
struct StorageValueIgnorer {
    Key key;
};

template <
    typename Key,
    typename Value,
    typename IndexT
>
class KeyValueShrinkableStorage : public IndexedStorage<KeyValue<Key, Value>, IndexT> {
    using KV = KeyValue<Key, Value>;
    using IndexedStorage<KV, IndexT>::getFileManager;
    using IndexedStorage<KV, IndexT>::getItemsCount;
    using KeyIndex = KeyValue<Key, IndexT>;
    using KeyIndexStorage = SortedSingleFileIndexedStorage<KeyIndex, IndexT>;
    using ValueIgnorer = StorageValueIgnorer<Key, Value>;

  public:
    explicit KeyValueShrinkableStorage(
        const std::string &notSortedStorageFilename,
        const std::string &sortedStorageFilename,
        std::shared_ptr<io::FileManager> fileManager
    )
        : sortedStorage_(0, sortedStorageFilename, fileManager),
          notSortedStorage_(notSortedStorageFilename, fileManager, 0) {
    }

    explicit KeyValueShrinkableStorage(
        const KeyValueShrinkableStorage &oldStorage,
        const KeyIndexStorage &actualIndex,
        const std::string &notSortedStorageFilename,
        const std::string &sortedStorageFilename,
        std::size_t indexBatchSize
    )
        : sortedStorage_(actualIndex.getItemsCount(), sortedStorageFilename, oldStorage.getFileManager()),
          notSortedStorage_(notSortedStorageFilename, oldStorage.getFileManager(), 0) {
        auto indexIterator = actualIndex.getDataIterator();
        io::OutputIterator<KV> keyValueOutputIterator
            = getFileManager()->template getOutputIterator<KV>(
                sortedStorage_.getStorageFilePath(),
                false
            );
        std::vector<KeyIndex> currentBatchIndex = indexIterator.collect(indexBatchSize);
        while (!currentBatchIndex.empty()) {
            for (std::size_t i = 0; i < currentBatchIndex.size(); ++i) {
                keyValueOutputIterator.write(KeyValue{
                    std::move(currentBatchIndex[i].key),
                    oldStorage.get(currentBatchIndex[i].value).value
                });
            }
            currentBatchIndex = indexIterator.collect(indexBatchSize);
        }
        keyValueOutputIterator.flush();
    }

    void resetWith(KeyValueShrinkableStorage &&other) {
        sortedStorage_.resetWith(std::move(other.sortedStorage_));
        notSortedStorage_.resetWith(std::move(other.notSortedStorage_));
    }

    [[nodiscard]] std::shared_ptr<io::FileManager> getFileManager() const noexcept override {
        return sortedStorage_.getFileManager();
    }

    [[nodiscard]] std::shared_ptr<io::TemporaryFile> shareSortedStorageFile() const noexcept {
        return sortedStorage_.shareStorageFile();
    }

    [[nodiscard]] std::shared_ptr<io::TemporaryFile> shareNotSortedStorageFile() const noexcept {
        return notSortedStorage_.shareStorageFile();
    }

    IndexT append(const KV &item) override {
        return sortedStorage_.getItemsCount() + notSortedStorage_.append(item);
    }

    KV get(IndexT index) const override {
        if (index >= sortedStorage_.getItemsCount()) {
            return notSortedStorage_.get(index - sortedStorage_.getItemsCount());
        }
        return sortedStorage_.get(index);
    }

    [[nodiscard]] IndexT getItemsCount() const noexcept override {
        return sortedStorage_.getItemsCount() + notSortedStorage_.getItemsCount();
    }

    [[nodiscard]] IndexT getSortedItemsCount() const noexcept {
        return sortedStorage_.getItemsCount();
    }

    [[nodiscard]] IndexT getNotSortedItemsCount() const noexcept {
        return notSortedStorage_.getItemsCount();
    }

    io::InputIterator<ValueIgnorer, IndexT> getSortedKeys() {
        return sortedStorage_.template getCustomDataIterator<ValueIgnorer>();
    }

    io::InputIterator<ValueIgnorer, IndexT> getNotSortedKeys() {
        return notSortedStorage_.template getCustomDataIterator<ValueIgnorer>();
    }

    io::InputIterator<KV, IndexT> getSortedEntries() {
        return sortedStorage_.getDataIterator();
    }

    io::InputIterator<KV, IndexT> getNotSortedEntries() {
        return notSortedStorage_.getDataIterator();
    }

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
            [](const KeyIndex &a, const KeyIndex &b) { return a.key < b.key; },
            [](const KeyIndex &a, const KeyIndex &b) { return a.key == b.key; },
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
    KeyIndexStorage exportKeys(IndexT batchSize, const std::string &keysFilename) {
        KeyIndexStorage sortedIndex(0, keysFilename, getFileManager());
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

    SortedSingleFileIndexedStorage<KV, IndexT> sortedStorage_;
    SingleFileIndexedStorage<KV, IndexT> notSortedStorage_;
};

namespace io {

template <typename Key, typename Value>
struct DeserializeHelper<StorageValueIgnorer<Key, Value>> : Deserializable<true> {
    static StorageValueIgnorer<Key, Value> deserialize(std::istream &is) {
        StorageValueIgnorer<Key, Value> svi{io::deserialize<Key>(is)};
        is.seekg(FixedDeserializedSizeRegister<Value>::exactDeserializedSize, std::ios_base::cur);
        if (is.fail()) {
            throw IOException("StorageValueIgnorer deserialization fail: seekg failed");
        }
        return svi;
    }
};

template <typename Key, typename Value>
struct FixedDeserializedSizeRegister<StorageValueIgnorer<Key, Value>>
    : FixedDeserializedSize<FixedDeserializedSizeRegister<Key>::exactDeserializedSize
                                + FixedDeserializedSizeRegister<Value>::exactDeserializedSize> {
};

} // io

} // supermap
