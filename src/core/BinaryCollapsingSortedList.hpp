#pragma once

#include <cmath>

#include "primitive/Key.hpp"
#include "SortedSingleFileIndexedStorage.hpp"

namespace supermap {

/**
 * @brief List of @p SortedSingleFileIndexedStorage.
 * For each storage, the rank is determined as log_2(SIZE / @p RankOneSize ).
 * Two storages are merged if their ranks are the same.
 * @tparam T Type of storage content.
 * @tparam IndexT Type of storage content index.
 * @tparam RankOneSize Size of block which have rank 1.
 */
template <typename T, typename IndexT, IndexT RankOneSize>
class BinaryCollapsingSortedList {
  private:
    using SortedStorage = SortedSingleFileIndexedStorage<T, IndexT>;

    struct ListNode {
        explicit ListNode(std::unique_ptr<SortedStorage> &&s, std::shared_ptr<ListNode> n)
            : storage(std::move(s)), next(std::move(n)) {}

        std::unique_ptr<SortedStorage> storage;
        std::shared_ptr<ListNode> next;

        bool valid() {
            return storage != nullptr;
        }

        IndexT getRank() {
            assert(valid());
            IndexT size = storage->getItemsCount();
            assert(size != 0);
            return std::log2((size + RankOneSize - 1) / RankOneSize);
        }
    };

  public:
    /**
     * @brief Accepts ownership of another storage.
     * @tparam Less Type of less functor.
     * @tparam Equal Type of equal functor.
     * @param storage New storage ownership. Its rank must be 1.
     * @param less Functor compares storage elements. Returns if first received object is less then the second.
     * @param equal Functor compares storage elements. Returns if first received object is equal to the second.
     * @param batchSize The size of the batch of @p T objects that are simultaneously stored in RAM.
     */
    template <typename Less, typename Equal>
    void pushFront(std::unique_ptr<SortedStorage> &&storage, Less less, Equal equal, IndexT batchSize) {
        assert(storage != nullptr);
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
                less,
                equal,
                batchSize
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
     * @tparam Less Type of less unary predicate.
     * @tparam Equal Type of equal unary predicate.
     * @param less Predicate, accepts object from storage, returns if it is less than the one required.
     * @param equal Predicate, accepts object from storage, returns if this is the one.
     * @return @p std::nullopt iff object predicate is not fulfilled by any object in all storages,
     * non-empty @p std::optional<T> otherwise.
     */
    template <typename Less, typename Equal>
    std::optional<T> find(Less less, Equal equal) {
        std::shared_ptr<ListNode> curNode = head_;
        while (curNode != nullptr) {
            assert(curNode->valid());
            std::optional<T> found = curNode->storage->find(less, equal);
            if (found.has_value()) {
                return found;
            }
            curNode = curNode->next;
        }
        return std::nullopt;
    }

    /**
     * @brief Traverses through all storages with given consumer.
     * @tparam Consumer Type of consumer.
     * @param consumer Consumer which is applicable for @p const @p &T.
     */
    template <
        typename Consumer,
        typename = std::enable_if_t<std::is_invocable_r_v<void, Consumer &, const SortedStorage &>>
    >
    void consumeStorages(Consumer &consumer) {
        std::shared_ptr<ListNode> curNode = head_;
        while (curNode != nullptr) {
            assert(curNode->valid());
            consumer(*curNode->storage);
            curNode = curNode->next;
        }
    }

  private:
    std::shared_ptr<ListNode> head_ = nullptr;
};

} // supermap
