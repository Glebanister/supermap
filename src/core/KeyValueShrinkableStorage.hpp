#pragma once

#include "io/InputIterator.hpp"
#include "io/SerializeHelper.hpp"
#include "io/TemporaryFile.hpp"
#include "SortedSingleFileIndexedStorage.hpp"
#include "primitive/KeyValue.hpp"
#include "primitive/Enum.hpp"

namespace supermap {

template <std::size_t KeyLen, std::size_t ValueLen>
struct StorageValueIgnorer {
    Key<KeyLen> key;
};

template <std::size_t KeyLen, std::size_t ValueLen, typename KV = KeyValue<KeyLen, ValueLen>>
class KeyValueShrinkableStorage : public IndexedStorage<KV, std::uint64_t> {
    using IndexT = std::uint64_t;
    using IndexedStorage<KV, IndexT>::getFileManager;
    using IndexedStorage<KV, IndexT>::getItemsCount;

  public:
    explicit KeyValueShrinkableStorage(
        const std::string &notSortedStorageFilename,
        const std::string &sortedStorageFilename,
        std::shared_ptr<io::FileManager> fileManager
    )
        : sortedStorage_({}, true, sortedStorageFilename, fileManager),
          notSortedStorage_(notSortedStorageFilename, fileManager, 0) {
    }

    explicit KeyValueShrinkableStorage(
        const IndexedStorage<KV, std::uint64_t> &oldStorage,
        const SortedSingleFileIndexedStorage<Enum<Key<KeyLen>>, IndexT> &actualIndex,
        const std::string &notSortedStorageFilename,
        const std::string &sortedStorageFilename,
        std::size_t indexBatchSize
    )
        : sortedStorage_(oldStorage.getItemsCount(), sortedStorageFilename, oldStorage.getFileManager()),
          notSortedStorage_(notSortedStorageFilename, oldStorage.getFileManager(), 0) {
        auto indexIterator = actualIndex.getDataIterator();
        io::OutputIterator<KeyValue<KeyLen, ValueLen>> keyValueOutputIterator
            = getFileManager()->template getOutputIterator<KeyValue<KeyLen, ValueLen>>(
                sortedStorage_.getStorageFilePath(),
                false
            );
        std::vector<Enum<Key<KeyLen>>> currentBatchIndex = indexIterator.collect(indexBatchSize);
        while (!currentBatchIndex.empty()) {
            for (std::size_t i = 0; i < currentBatchIndex.size(); ++i) {
                keyValueOutputIterator.write(KeyValue{
                    std::move(currentBatchIndex[i].elem),
                    oldStorage.get(currentBatchIndex[i].index).value
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

    io::InputIterator<StorageValueIgnorer<KeyLen, ValueLen>> getSortedKeys() {
        return sortedStorage_.template getCustomDataIterator<StorageValueIgnorer<KeyLen, ValueLen>>();
    }

    io::InputIterator<StorageValueIgnorer<KeyLen, ValueLen>> getNotSortedKeys() {
        return notSortedStorage_.template getCustomDataIterator<StorageValueIgnorer<KeyLen, ValueLen>>();
    }

    io::InputIterator<KeyValue<KeyLen, ValueLen>> getSortedEntries() {
        return sortedStorage_.getDataIterator();
    }

    io::InputIterator<KeyValue<KeyLen, ValueLen>> getNotSortedEntries() {
        return notSortedStorage_.getDataIterator();
    }

    [[nodiscard]] SortedSingleFileIndexedStorage<Enum<Key<KeyLen>>, IndexT> shrink(
        IndexT shrinkBatchSize,
        const std::string &newIndexFileName
    ) {
        const std::string shrinkFilenamePrefix = "shrink";
        const std::string tempSortedIndexFilename = shrinkFilenamePrefix + "-sorted-keys";

        std::size_t batchesCount = (getItemsCount() + shrinkBatchSize - 1) / shrinkBatchSize;
        auto notSortedKeysStream =
            io::InputIterator<StorageValueIgnorer<KeyLen, ValueLen>>(
                notSortedStorage_.getFileManager()->getInputStream(notSortedStorage_.getStorageFilePath(), 0)
            );
        std::vector<std::shared_ptr<io::TemporaryFile>> tempFilesLock;
        std::vector<SortedSingleFileIndexedStorage<Enum<Key<KeyLen>>, IndexT>> sortedBatches;
        sortedBatches.reserve(batchesCount + 1);
        tempFilesLock.reserve(batchesCount + 1);

        SortedSingleFileIndexedStorage<Enum<Key<KeyLen>>> exportedKeys
            = exportKeys(shrinkBatchSize, tempSortedIndexFilename);
        tempFilesLock.push_back(exportedKeys.shareStorageFile());

        sortedBatches.push_back(std::move(exportedKeys));
        for (std::size_t batchI = 0; batchI < batchesCount; ++batchI) {
            const std::string batchFileName = shrinkFilenamePrefix + "-batch-" + std::to_string(batchI);
            SortedSingleFileIndexedStorage<Enum<Key<KeyLen>>, IndexT> sortedBatch(
                notSortedKeysStream.template collectWith(
                    [](StorageValueIgnorer<KeyLen, ValueLen> &&svi, std::uint32_t index) {
                        return Enum<Key<KeyLen>>{std::move(svi.key), index};
                    },
                    shrinkBatchSize),
                false,
                batchFileName,
                getFileManager()
            );
            tempFilesLock.push_back(sortedBatch.shareStorageFile());
            assert(sortedBatch.getItemsCount() > 0 && "Batch file can not be empty");
            sortedBatches.push_back(std::move(sortedBatch));
        }
        SortedSingleFileIndexedStorage<Enum<Key<KeyLen>>, IndexT> updatedIndex(
            sortedBatches,
            newIndexFileName,
            getFileManager()
        );

        resetWith(KeyValueShrinkableStorage(
            *this,
            updatedIndex,
            shrinkFilenamePrefix + "-new-not-sorted",
            shrinkFilenamePrefix + "-new-sorted",
            shrinkBatchSize
        ));
        return updatedIndex;
    }

  private:
    SortedSingleFileIndexedStorage<Enum<Key<KeyLen>>> exportKeys(IndexT batchSize,
                                                                 const std::string &keysFilename) {
        SortedSingleFileIndexedStorage<Enum<Key<KeyLen>>> sortedIndex(
            0,
            keysFilename,
            getFileManager()
        );
        auto it = sortedStorage_.template getCustomDataIterator<StorageValueIgnorer<KeyLen, ValueLen>>();
        auto keys = it.collect(batchSize);
        while (!keys.empty()) {
            sortedIndex.appendAll(keys.cbegin(),
                                  keys.cend(),
                                  [](StorageValueIgnorer<KeyLen, ValueLen> svi) { return svi.key; });
            keys = it.collect(batchSize);
        }
        return sortedIndex;
    }

    SortedSingleFileIndexedStorage<KV, std::uint64_t> sortedStorage_;
    SingleFileIndexedStorage<KV, std::uint64_t> notSortedStorage_;
};

namespace io {

template <std::size_t KeyLen, std::size_t ValueLen>
struct DeserializeHelper<StorageValueIgnorer<KeyLen, ValueLen>> : Deserializable<true> {
    static StorageValueIgnorer<KeyLen, ValueLen> deserialize(std::istream &is) {
        StorageValueIgnorer<KeyLen, ValueLen> svi{io::deserialize<Key<KeyLen>>(is)};
        is.seekg(ValueLen, std::ios_base::cur);
        if (is.fail()) {
            throw IOException("StorageValueIgnorer deserialization fail: seekg failed");
        }
        return svi;
    }
};

template <std::size_t KeyLen, std::size_t ValueLen>
struct FixedDeserializedSizeRegister<StorageValueIgnorer<KeyLen, ValueLen>>
    : FixedDeserializedSize<KeyLen + ValueLen> {
};

} // io

} // supermap
