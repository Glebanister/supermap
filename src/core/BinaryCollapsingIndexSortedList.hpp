#pragma once

#include <cmath>

#include "primitive/Key.hpp"
#include "SortedSingleFileIndexedStorage.hpp"

namespace supermap {

template <typename T, typename IndexT, IndexT RankOneSize>
class BinaryCollapsingIndexSortedList
    : std::enable_shared_from_this<BinaryCollapsingIndexSortedList<T, IndexT, RankOneSize>> {
  public:
    template <typename Iterator, typename IsLess, typename IsEq>
    explicit BinaryCollapsingIndexSortedList(Iterator begin,
                                             Iterator end,
                                             bool sorted,
                                             std::string dataFileName,
                                             std::shared_ptr<io::FileManager> manager,
                                             IsLess isLess,
                                             IsEq isEq
    ) : keys_(std::move(begin),
              std::move(end),
              sorted,
              std::move(dataFileName),
              std::move(manager),
              std::move(isLess),
              std::move(isEq)),
        tail_(nullptr) {}

    template <typename IsLess, typename IsEq>
    explicit BinaryCollapsingIndexSortedList(
        SortedSingleFileIndexedStorage<T, IndexT> &&sortedKeys,
        std::string dataFileName,
        std::shared_ptr<io::FileManager> fileManager,
        IsLess isLess,
        IsEq isEq
    ) : keys_({std::move(sortedKeys)},
              std::move(dataFileName),
              std::move(fileManager),
              std::move(isLess),
              std::move(isEq)),
        tail_(nullptr) {}

    IndexT getRank() const noexcept {
        return std::log2(keys_.getItemsCount() / RankOneSize);
    }

    void setTail(std::shared_ptr<BinaryCollapsingIndexSortedList> tail) {
        if (getRank() > tail->getRank()) {
            throw SupermapException("Head rank is greater than tail rank");
        }
        tail_ = std::move(tail);
        collapseWithTail();
    }

    template <typename LessUnary, typename EqUnary>
    std::optional<T> find(LessUnary less, EqUnary eq) {
        std::optional<T> found = keys_.find(less, eq);
        if (found.has_value()) {
            return found;
        }
        return tail_
               ? tail_->find(std::move(less), std::move(eq))
               : std::nullopt;
    }

    template <typename Less, typename Equal>
    std::shared_ptr<BinaryCollapsingIndexSortedList> collapseWithTail(Less less, Equal equal) {
        if (!tail_) {
            return std::enable_shared_from_this<
                BinaryCollapsingIndexSortedList<T, IndexT, RankOneSize>
            >::shared_from_this();
        }
        IndexT tailRank = tail_->getRank();
        IndexT thisRank = getRank();
        assert(thisRank <= tailRank);
        if (thisRank < tailRank) {
            return std::enable_shared_from_this<
                BinaryCollapsingIndexSortedList<T, IndexT, RankOneSize>
            >::shared_from_this();
        }
        SortedSingleFileIndexedStorage<T, IndexT> mergedKeys(
            {*tail_, *this},
            "collapse",
            keys_.getFileManager(),
            std::move(less),
            std::move(equal)
        );
        tail_->keys().resetWith(std::move(mergedKeys));
        return tail_ = tail_->collapseWithTail();
    }

  protected:
    SortedSingleFileIndexedStorage<T, IndexT> &keys() noexcept {
        return keys_;
    }

  private:
    SortedSingleFileIndexedStorage<T, IndexT> keys_;
    std::shared_ptr<BinaryCollapsingIndexSortedList> tail_;
};

} // supermap
