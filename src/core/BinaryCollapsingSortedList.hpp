#pragma once

#include <cmath>

#include "primitive/Key.hpp"
#include "SortedSingleFileIndexedStorage.hpp"

namespace supermap {

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
    template <typename Less, typename Equal>
    void pushFront(std::unique_ptr<SortedStorage> &&storage, Less less, Equal equal) {
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
                equal
            );
            nextNode->storage->resetWith(std::move(mergedKeys));
            head_ = nextNode;
            curNode = nextNode;
            nextNode = nextNode->next;
        }
    }

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
