#pragma once

#include <memory>

#include "primitive/Bounds.hpp"
#include "KeyValueStorage.hpp"
#include "KeyValueShrinkableStorage.hpp"
#include "BinaryCollapsingSortedList.hpp"
#include "ExtractibleKeyValueStorage.hpp"

namespace supermap {

template <
    typename Key,
    typename Value,
    typename IndexT,
    IndexT MaxRamLoad
>
class Supermap : public KeyValueStorage<Key, Value, Bounds<IndexT>> {
  public:
    using IndexList = BinaryCollapsingSortedList<KeyValue<Key, IndexT>, IndexT, MaxRamLoad>;
    using RamStorage = ExtractibleKeyValueStorage<Key, IndexT, IndexT>;
    using DiskStorage = KeyValueShrinkableStorage<Key, Value, IndexT>;
    using KeyVal = KeyValue<Key, Value>;
    using KeyIndex = KeyValue<Key, IndexT>;
    using SortedKeyIndexStorage = SortedSingleFileIndexedStorage<KeyIndex, IndexT>;
    using KeyType = Key;
    using ValueType = Value;
    using IndexType = IndexT;
    using BoundsType = Bounds<IndexT>;

  public:
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
          indexListSupplier_(std::move(indexListSupplier)) {
    }

    IndexT getCollapsingBlocksListLength() const {
        struct Counter {
            IndexT count = 0;
            void operator()(const SortedKeyIndexStorage &) { ++count; }
        } counter;
        diskIndex_->consumeStorages(counter);
        return counter.count;
    }

    [[nodiscard]] std::string getNewBlockName() const {
        return indexFilesPrefix + "block-" + std::to_string(getCollapsingBlocksListLength());
    }

    void add(const Key &key, Value &&value) override {
        IndexT valueIndex = diskDataStorage_->append(KeyValue{key, std::move(value)});
        innerStorage_->add(key, IndexT(valueIndex));
        if (innerStorage_->getSize() >= MaxRamLoad) {
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
            diskIndex_->pushFront(
                std::move(newBlock),
                [](const KeyIndex &a, const KeyIndex &b) { return a.key < b.key; },
                [](const KeyIndex &a, const KeyIndex &b) { return a.key == b.key; }
            );
        }
        if (shouldShrinkChecker_(
            diskDataStorage_->getNotSortedItemsCount(),
            diskDataStorage_->getSortedItemsCount() + diskDataStorage_->getNotSortedItemsCount())) {
            auto actualIndex = std::make_unique<SortedKeyIndexStorage>(
                diskDataStorage_->shrink(
                    keyIndexBatchSize_,
                    indexFilesPrefix + "new-keys=" + std::to_string(getSize().max)
                ));
            std::unique_ptr<IndexList> newIndexList = indexListSupplier_();
            newIndexList->pushFront(
                std::move(actualIndex),
                [](const KeyIndex &a, const KeyIndex &b) { return a.key < b.key; },
                [](const KeyIndex &a, const KeyIndex &b) { return a.key == b.key; }
            );
            diskIndex_ = std::move(newIndexList);
        }
    }

    bool containsKey(const Key &) override {
        throw NotImplementedException();
    }

    const Value &getValue(const Key &) override {
        throw NotImplementedException();
    }

    Bounds<IndexT> getSize() const noexcept override {
        return {
            diskDataStorage_->getSortedItemsCount(),
            diskDataStorage_->getSortedItemsCount() + diskDataStorage_->getNotSortedItemsCount()
        };
    }

  private:
    std::unique_ptr<RamStorage> innerStorage_;
    std::unique_ptr<DiskStorage> diskDataStorage_;
    std::unique_ptr<IndexList> diskIndex_;
    std::function<bool(IndexT, IndexT)> shouldShrinkChecker_;
    const IndexT keyIndexBatchSize_;
    std::function<std::unique_ptr<IndexList>()> indexListSupplier_;
    const std::string indexFilesPrefix = "index-";
};

} // supermap
