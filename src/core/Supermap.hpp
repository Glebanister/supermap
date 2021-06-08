#pragma once

#include <chrono>
#include <memory>
#include <random>

#include "KeyValueStorage.hpp"
#include "KeyValueShrinkableStorage.hpp"
#include "SortedStoragesList.hpp"
#include "ExtractibleKeyValueStorage.hpp"
#include "BinaryCollapsingSortedStoragesList.hpp"
#include "FilteringRegister.hpp"
#include "FilteredStorage.hpp"

namespace supermap {

/**
 * @brief Key-value storage.
 * Stores all values on disk. The index is partially stored in RAM.
 * When the index in RAM overflows, it is reset to an index on disk, where it is stored as a binary collapsible list.
 * @tparam Key Type of key.
 * @tparam Value Type of value.
 * @tparam IndexT Type of size.
 * @tparam MaxRamLoad The largest size of an index that can reside in RAM.
 */
template <
    typename Key,
    typename Value,
    typename IndexT
>
class Supermap : public KeyValueStorage<Key, Value, IndexT> {
  public:
    using KeyVal = KeyValue<Key, Value>;
    using KeyIndex = KeyValue<Key, IndexT>;
    using RegisterBase = FilteringRegister<KeyIndex, Key>;
    using FilterType = Filter<Key>;
    using RegisterInfo = typename RegisterBase::ItemsInfo;
    using FilterSupplier = std::function<std::unique_ptr<FilterType>()>;

    using IndexStorageListBase = SortedStoragesList<KeyIndex, IndexT, RegisterInfo, Key>;
    using IndexStorageBase = SortedSingleFileIndexedStorage<KeyIndex, IndexT, RegisterInfo, Key>;
    using RamStorageBase = ExtractibleKeyValueStorage<Key, IndexT, IndexT>;
    using DiskStorage = KeyValueShrinkableStorage<Key, Value, IndexT, RegisterInfo>;

  public:
    explicit Supermap(std::unique_ptr<RamStorageBase> &&innerStorage,
                      std::unique_ptr<DiskStorage> &&diskDataStorage,
                      std::function<std::unique_ptr<IndexStorageBase>(IndexStorageBase &&)> keyIndexStorageSupplier,
                      std::function<std::unique_ptr<IndexStorageListBase>()> indexListSupplier,
                      std::function<std::unique_ptr<RegisterBase>()> registerSupplier,
                      IndexT keyIndexBatchSize,
                      double maxNotSortedPart)
        : innerStorage_(std::move(innerStorage)),
          diskDataStorage_(std::move(diskDataStorage)),
          diskIndex_(indexListSupplier()),
          keyIndexStorageSupplier_(std::move(keyIndexStorageSupplier)),
          indexListSupplier_(std::move(indexListSupplier)),
          registerSupplier_(std::move(registerSupplier)),
          keyIndexBatchSize_(keyIndexBatchSize),
          random(std::chrono::steady_clock::now().time_since_epoch().count()),
          maxNotSortedPart_(maxNotSortedPart) {}

    /**
     * @brief Adds new key-value pair to the storage.
     * @param key Key to add.
     * @param value Associated value.
     */
    void add(const Key &key, Value &&value) override {
        diskDataStorage_->append(std::make_unique<KeyVal>(key, value));
        innerStorage_->add(key, diskDataStorage_->getLastElementIndex());
        if (innerStorage_->getUpperSizeBound() >= keyIndexBatchSize_) {
            dropRamIndexToDisk();
        }

        IndexT notSortedStorageSize = diskDataStorage_->getNotSortedItemsCount();
        IndexT totalStorageSize = diskDataStorage_->getItemsCount();

        double notSortedPart = static_cast<double>(notSortedStorageSize) / static_cast<double>(totalStorageSize);

        if (notSortedPart >= maxNotSortedPart_) {
            dropRamIndexToDisk();
            shrinkDataStorage();
        }
    }

    /**
     * @return Object of type @p Value which corresponds to given key @p k.
     * @throws KeyException If key is not in the storage.
     */
    std::optional<Value> getValue(const Key &k) override {
        IndexT index;
        if (auto optIndex = innerStorage_->getValue(k); optIndex.has_value()) {
            index = optIndex.value();
        } else {
            std::optional<KeyIndex> foundOnDisk = diskIndex_->find(
                k,
                [](const KeyIndex &ki, const Key &key) { return ki.key < key; },
                [](const KeyIndex &ki, const Key &key) { return ki.key == key; }
            );
            if (!foundOnDisk.has_value()) {
                return std::nullopt;
            }
            index = foundOnDisk.value().value;
        }
        return std::optional{diskDataStorage_->get(index).value};
    }

    /**
     * @return Upper bound of number of the unique keys in the storage.
     */
    IndexT getUpperSizeBound() const noexcept override {
        return diskDataStorage_->getSortedItemsCount() + diskDataStorage_->getNotSortedItemsCount();
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
        return addRandomString(indexFilesPrefix + "block-" + std::to_string(diskIndex_->getItemsCount()));
    }

    void dropRamIndexToDisk() {
        if (innerStorage_->getUpperSizeBound() == 0) {
            return;
        }
        std::vector<KeyIndex> newBlockKeyIndex = std::move(*innerStorage_).extract();
        auto newBlock = IndexStorageBase(
            newBlockKeyIndex.begin(),
            newBlockKeyIndex.end(),
            true,
            getNewBlockName(),
            diskDataStorage_->getFileManager(),
            [](const KeyIndex &a, const KeyIndex &b) { return a.key < b.key; },
            [](const KeyIndex &a, const KeyIndex &b) { return a.key == b.key; },
            registerSupplier_
        );
        diskIndex_->append(keyIndexStorageSupplier_(std::move(newBlock)));
    }

    void shrinkDataStorage() {
        auto actualIndex = keyIndexStorageSupplier_(
            diskDataStorage_->shrink(
                keyIndexBatchSize_,
                addRandomString(indexFilesPrefix + "new-keys=" + std::to_string(getUpperSizeBound()))
            ));
        std::unique_ptr<IndexStorageListBase> newIndexList = indexListSupplier_();
        newIndexList->append(std::move(actualIndex));
        diskIndex_ = std::move(newIndexList);
    }

    std::unique_ptr<RamStorageBase> innerStorage_;
    std::unique_ptr<DiskStorage> diskDataStorage_;
    std::unique_ptr<IndexStorageListBase> diskIndex_;
    std::function<std::unique_ptr<IndexStorageBase>(IndexStorageBase &&)> keyIndexStorageSupplier_;
    std::function<std::unique_ptr<IndexStorageListBase>()> indexListSupplier_;
    std::function<std::unique_ptr<RegisterBase>()> registerSupplier_;
    const IndexT keyIndexBatchSize_;
    const std::string indexFilesPrefix = "index-";
    mutable std::mt19937 random;
    const double maxNotSortedPart_;
};

} // supermap
