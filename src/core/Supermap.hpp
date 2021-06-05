#pragma once

#include <chrono>
#include <memory>
#include <random>

#include "primitive/Bounds.hpp"
#include "KeyValueStorage.hpp"
#include "KeyValueShrinkableStorage.hpp"
#include "SortedStoragesList.hpp"
#include "ExtractibleKeyValueStorage.hpp"
#include "BinaryCollapsingSortedStoragesList.hpp"

namespace supermap {

/**
 * @brief Key-value storage.
 * Stores all values on disk. The index is partially stored in RAM.
 * When the index in RAM overflows, it is reset to an index on disk, where it is stored as a binary collapsible list.
 * @tparam Key Type of key.
 * @tparam Value Type of value.
 * @tparam IndexT Type of Bounds.
 * @tparam MaxRamLoad The largest size of an index that can reside in RAM.
 * @tparam EachIndexRegType Each index storage register.
 */
template <
    typename Key,
    typename Value,
    typename IndexT,
    IndexT MaxRamLoad,
    typename EachIndexRegType
>
class Supermap : public KeyValueStorage<Key, Value, Bounds<IndexT>> {
  public:
    using KeyVal = KeyValue<Key, Value>;
    using KeyIndex = KeyValue<Key, IndexT>;
    using IndexList = SortedStoragesList<KeyIndex, IndexT, EachIndexRegType>;
    using RamStorage = ExtractibleKeyValueStorage<Key, IndexT, IndexT>;
    using DiskStorage = KeyValueShrinkableStorage<Key, Value, IndexT, EachIndexRegType>;
    using SortedKeyIndexStorage = SortedSingleFileIndexedStorage<KeyIndex, IndexT, EachIndexRegType>;
    using KeyType = Key;
    using ValueType = Value;
    using IndexType = IndexT;
    using BoundsType = Bounds<IndexT>;
    using DefaultIndexList = BinaryCollapsingSortedStoragesList<KeyIndex, IndexT, MaxRamLoad, EachIndexRegType>;

  public:
    /**
     * @brief Creates an empty Supermap instance.
     * @param innerStorage Implementation of internal (RAM) storage.
     * @param diskDataStorage Implementation of disk storage.
     * @param shouldShrinkChecker A predicate that takes two parameters:
     * the size of the unsorted part of the disk storage and the total size of the storage.
     * The predicate should tell whether shrink should be done.
     * @param indexListSupplier Producer of index lists.
     * @param keyIndexBatchSize BatchSize The size of the batch
     * of @p KeyValue<T,IndexT> objects that are simultaneously stored in RAM.
     */
    explicit Supermap(std::unique_ptr<RamStorage> &&innerStorage,
                      std::unique_ptr<DiskStorage> &&diskDataStorage,
                      std::function<bool(IndexT, IndexT)> shouldShrinkChecker,
                      std::function<std::unique_ptr<IndexList>()> indexListSupplier,
                      IndexT keyIndexBatchSize
    )
        : innerStorage_(std::move(innerStorage)),
          diskDataStorage_(std::move(diskDataStorage)),
          diskIndex_(indexListSupplier()),
          shouldShrinkChecker_(std::move(shouldShrinkChecker)),
          keyIndexBatchSize_(std::move(keyIndexBatchSize)),
          indexListSupplier_(std::move(indexListSupplier)),
          random(std::chrono::steady_clock::now().time_since_epoch().count()) {
    }

    /**
     * @return Number of blocks in collapsing list.
     */
    IndexT getCollapsingBlocksListLength() const {
        return diskIndex_->getItemsCount();
    }

    /**
     * @brief Adds new key-value pair to the storage.
     * @param key Key to add.
     * @param value Associated value.
     */
    void add(const Key &key, Value &&value) override {
        diskDataStorage_->append(std::make_unique<KeyVal>(key, value));
        innerStorage_->add(key, diskDataStorage_->getLastElementIndex());
        if (innerStorage_->getSize() >= MaxRamLoad) {
            dropRamIndexToDisk();
        }
        if (shouldShrinkChecker_(
            diskDataStorage_->getNotSortedItemsCount(),
            diskDataStorage_->getSortedItemsCount() + diskDataStorage_->getNotSortedItemsCount())) {

            dropRamIndexToDisk();
            shrinkDataStorage();
        }
    }

    /**
     * @return If @k is in the storage.
     */
    bool containsKey(const Key &k) override {
        if (innerStorage_->containsKey(k)) {
            return true;
        }
        return diskIndex_->find(
            [&](const KeyIndex &ki) { return ki.key < k; },
            [&](const KeyIndex &ki) { return ki.key == k; }
        ).has_value();
    }

    /**
     * @return Object of type @p Value which corresponds to given key @p k.
     * @throws KeyException If key is not in the storage.
     */
    Value getValue(const Key &k) override {
        IndexT index;
        if (innerStorage_->containsKey(k)) {
            index = innerStorage_->getValue(k);
        } else {
            std::optional<KeyIndex> foundOnDisk = diskIndex_->find(
                [&](const KeyIndex &ki) { return ki.key < k; },
                [&](const KeyIndex &ki) { return ki.key == k; }
            );
            if (!foundOnDisk.has_value()) {
                throw KeyException(k.toString(), "Not Found");
            }
            index = foundOnDisk.value().value;
        }
        return diskDataStorage_->get(index).value;
    }

    /**
     * @return Bounds of number of the unique keys in the storage.
     */
    Bounds<IndexT> getSize() const noexcept override {
        return {
            diskDataStorage_->getSortedItemsCount(),
            diskDataStorage_->getSortedItemsCount() + diskDataStorage_->getNotSortedItemsCount()
        };
    }

  private:
    /**
     * @param s Initial string.
     * @param len Number of random characters.
     * @return Initial string concatenated with @p len random characters.
     */
    [[nodiscard]] std::string addRandomString(std::string s, std::size_t len = 8) const {
        assert(len != 0);
        s += '-';
        for (std::size_t i = 0; i < len; ++i) {
            s += static_cast<char>(random() % ('z' - 'a' + 1) + 'a');
        }
        return s;
    }

    /**
     * @return New collapsing index block name.
     */
    [[nodiscard]] std::string getNewBlockName() const {
        return addRandomString(indexFilesPrefix + "block-" + std::to_string(getCollapsingBlocksListLength()));
    }

    void dropRamIndexToDisk() {
        if (innerStorage_->getSize() == 0) {
            return;
        }
        std::vector<KeyIndex> newBlockKeyIndex = std::move(*innerStorage_).extract();
        auto newBlock = std::make_unique<SortedKeyIndexStorage>(
            newBlockKeyIndex.begin(),
            newBlockKeyIndex.end(),
            true,
            getNewBlockName(),
            diskDataStorage_->getFileManager(),
            [](const KeyIndex &a, const KeyIndex &b) { return a.key < b.key; },
            [](const KeyIndex &a, const KeyIndex &b) { return a.key == b.key; }
        );
        diskIndex_->append(std::move(newBlock));
    }

    void shrinkDataStorage() {
        auto actualIndex = std::make_unique<SortedKeyIndexStorage>(
            diskDataStorage_->shrink(
                keyIndexBatchSize_,
                addRandomString(indexFilesPrefix + "new-keys=" + std::to_string(getSize().max))
            ));
        std::unique_ptr<IndexList> newIndexList = indexListSupplier_();
        newIndexList->append(std::move(actualIndex));
        diskIndex_ = std::move(newIndexList);
    }

    std::unique_ptr<RamStorage> innerStorage_;
    std::unique_ptr<DiskStorage> diskDataStorage_;
    std::unique_ptr<IndexList> diskIndex_;
    std::function<bool(IndexT, IndexT)> shouldShrinkChecker_;
    const IndexT keyIndexBatchSize_;
    std::function<std::unique_ptr<IndexList>()> indexListSupplier_;
    const std::string indexFilesPrefix = "index-";
    mutable std::mt19937 random;
};

} // supermap
