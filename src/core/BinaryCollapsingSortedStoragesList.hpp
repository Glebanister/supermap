#pragma once

#include <cmath>

#include "primitive/Key.hpp"
#include "SortedStoragesList.hpp"

namespace supermap {

/**
 * @brief List of @p SortedSingleFileIndexedStorage.
 * For each storage, the rank is determined as log_2(SIZE / @p RankOneSize ).
 * Two storages are merged if their ranks are the same.
 * @tparam T Type of storage content.
 * @tparam IndexT Type of storage content index.
 * @tparam RankOneSize Size of block which have rank 1.
 * @tparam InnerRegisterInfo This list storages registers info.
 */
template <
    typename T,
    typename IndexT,
    IndexT RankOneSize,
    typename InnerRegisterInfo,
    typename FindPatternType
>
class BinaryCollapsingSortedStoragesList : public SortedStoragesList<T, IndexT, InnerRegisterInfo, FindPatternType> {
  private:
    using SortedStorage = SortedSingleFileIndexedStorage<T, IndexT, InnerRegisterInfo, FindPatternType>;
    using InnerRegisterSupplier = typename SortedStorage::InnerRegisterSupplier;

    /**
     * @brief A node in linked list of storages.
     */
    struct ListNode {
        explicit ListNode(std::unique_ptr<SortedStorage> &&s, std::shared_ptr<ListNode> n)
            : storage(std::move(s)), next(std::move(n)) {}

        std::unique_ptr<SortedStorage> storage;
        std::shared_ptr<ListNode> next;

        /**
         * @return If node's storage is initialized.
         */
        bool valid() {
            return storage != nullptr;
        }

        /**
         * @return Rank of corresponding storage.
         */
        IndexT getRank() {
            assert(valid());
            IndexT size = storage->getItemsCount();
            assert(size != 0);
            return std::log2((size + RankOneSize - 1) / RankOneSize);
        }
    };

  public:
    /**
     * @brief Creates BinaryCollapsingSortedStoragesList.
     * @param batchSize The size of the batch of @p T objects that are simultaneously stored in RAM.
     * @param innerRegisterSupplier Supplier of registers for all inner storages.
     */
    explicit BinaryCollapsingSortedStoragesList(IndexT batchSize, InnerRegisterSupplier innerRegisterSupplier)
        : head_(nullptr),
          batchSize_(batchSize),
          innerRegisterSupplier_(std::move(innerRegisterSupplier)) {}

    /**
     * @brief Add storage with the largest order to list.
     * @param storage New storage. Its rank must be 0.
     */
    void append(std::unique_ptr<SortedStorage> &&storage) override {
        head_ = std::make_shared<ListNode>(std::move(storage), std::move(head_));
        std::shared_ptr<ListNode> curNode = head_;
        std::shared_ptr<ListNode> nextNode = curNode->next;
        while (nextNode != nullptr) {
            assert(curNode->getRank() <= nextNode->getRank());
            if (curNode->getRank() < nextNode->getRank()) {
                break;
            }
            assert(curNode->valid());
            assert(nextNode->valid());
            SortedStorage mergedKeys(
                std::vector<SortedStorage>{*nextNode->storage, *curNode->storage},
                "collapse",
                curNode->storage->getFileManager(),
                batchSize_,
                innerRegisterSupplier_
            );
            nextNode->storage->resetWith(std::move(mergedKeys));
            head_ = nextNode;
            curNode = nextNode;
            nextNode = nextNode->next;
        }
    }

    /**
     * @brief Searches for the fulfillment of the predicate @p equal in all storages,
     * starting from the last added storages.
     * @param pattern Find pattern.
     * @param less Predicate, accepts object from storage and pattern, returns if object is less then pattern.
     * @param equal Predicate, accepts object from storage and pattern, returns if this is equals to pattern.
     * @return @p std::nullopt iff object predicate is not fulfilled by any object in all storages,
     * non-empty @p std::optional<T> otherwise.
     */
    std::optional<T> find(
        const FindPatternType &pattern,
        std::function<bool(const T &, const FindPatternType &)> less,
        std::function<bool(const T &, const FindPatternType &)> equal
    ) override {
        std::shared_ptr<ListNode> curNode = head_;
        while (curNode != nullptr) {
            assert(curNode->valid());
            std::optional<T> found = curNode->storage->find(pattern, less, equal);
            if (found.has_value()) {
                return found;
            }
            curNode = curNode->next;
        }
        return std::nullopt;
    }

  private:
    std::shared_ptr<ListNode> head_ = nullptr;
    IndexT batchSize_;
    InnerRegisterSupplier innerRegisterSupplier_;
};

} // supermap
