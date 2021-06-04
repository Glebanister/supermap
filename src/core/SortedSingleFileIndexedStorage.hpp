#pragma once

#include <algorithm>
#include <optional>

#include "SingleFileIndexedStorage.hpp"

namespace supermap {

template <typename T, typename IndexT>
class SortedSingleFileIndexedStorage : public SingleFileIndexedStorage<T, IndexT> {
  public:
    using SingleFileIndexedStorage<T, IndexT>::getItemsCount;
    using SingleFileIndexedStorage<T, IndexT>::get;
    using SingleFileIndexedStorage<T, IndexT>::append;
    using SingleFileIndexedStorage<T, IndexT>::appendAll;

    explicit SortedSingleFileIndexedStorage(IndexT size,
                                            std::string dataFileName,
                                            std::shared_ptr<io::FileManager> manager)
        : SingleFileIndexedStorage<T, IndexT>(std::move(dataFileName), manager, size) {
    }

    template <
        typename Iterator,
        typename IsLess,
        typename IsEq
    >
    explicit SortedSingleFileIndexedStorage(Iterator begin,
                                            Iterator end,
                                            bool sorted,
                                            std::string dataFileName,
                                            std::shared_ptr<io::FileManager> manager,
                                            IsLess isLess,
                                            IsEq isEq
    ) : SingleFileIndexedStorage<T, IndexT>(std::move(dataFileName), manager, 0) {
        appendAll(begin, sorted ? end : sortedEndIterator(begin, end, isLess, isEq));
    }

    template <
        typename LessComp,
        typename EqComp,
        typename Iterator,
        typename = std::enable_if_t<std::is_same_v<T, typename std::iterator_traits<Iterator>::value_type>>
    >
    static Iterator sortedEndIterator(
        Iterator begin,
        Iterator end,
        LessComp isLess,
        EqComp isEq
    ) {
        std::reverse(begin, end);
        std::stable_sort(begin, end, isLess);
        return std::unique(begin, end, isEq);
    }

    virtual std::optional<T> find(std::function<bool(const T &)> less, std::function<bool(const T &)> equal) {
        if (getItemsCount() == 0) {
            return std::nullopt;
        }
        IndexT firstLeq = 0;
        IndexT lastGt = getItemsCount();
        while (lastGt - firstLeq > 1) {
            IndexT middle = (firstLeq + lastGt) / 2;
            T middleElem = get(middle);
            if (less(middleElem) || equal(middleElem)) {
                firstLeq = middle;
            } else {
                lastGt = middle;
            }
        }
        if (firstLeq >= getItemsCount()) {
            return std::nullopt;
        }
        T firstLeqElem = get(firstLeq);
        return equal(firstLeqElem) ? std::optional{firstLeqElem} : std::nullopt;
    }

    SortedSingleFileIndexedStorage fromNotSorted(
        std::vector<T> data,
        const std::string &dataFileName,
        std::shared_ptr<io::FileManager> manager) {
        return SortedIndexedStorage(std::move(data), true, dataFileName, manager);
    }

    template <typename IsLess, typename IsEq>
    explicit SortedSingleFileIndexedStorage(
        const std::vector<SortedSingleFileIndexedStorage<T, IndexT>> &newer,
        std::string dataFileName,
        std::shared_ptr<io::FileManager> fileManager,
        IsLess isLess,
        IsEq isEq,
        IndexT batchSize
    ) : SingleFileIndexedStorage<T, IndexT>(std::move(dataFileName), std::move(fileManager), 0) {

        const std::size_t storagesCount = newer.size();
        std::vector<IndexT> currentFrontPointers(storagesCount);
        std::vector<std::optional<T>> frontLine(storagesCount, std::nullopt);
        std::uint64_t totalSize = 0;
        for (std::size_t i = 0; i < storagesCount; ++i) {
            if (newer[i].getItemsCount() > 0) {
                frontLine[i].emplace(newer[i].get(0));
                totalSize += newer[i].getItemsCount();
            }
        }

        auto updateOnce = [&](std::int32_t i) {
            if (++currentFrontPointers[i] < newer[i].getItemsCount()) {
                frontLine[i].emplace(newer[i].get(currentFrontPointers[i]));
            } else {
                frontLine[i].reset();
            }
        };

        std::vector<T> writeBuffer;
        writeBuffer.reserve(batchSize);

        auto dropWriteBuffer = [&]() {
            appendAll(writeBuffer.begin(), writeBuffer.end());
            writeBuffer.clear();
            writeBuffer.reserve(batchSize);
        };

        while (true) {
            std::size_t min = 0;
            bool hasMin = false;
            for (std::int32_t i = storagesCount - 1; i >= 0; --i) {
                if (!frontLine[i].has_value()) {
                    continue;
                }
                if (!hasMin) {
                    hasMin = true;
                    min = i;
                    continue;
                }
                if (isLess(frontLine[i].value(), frontLine[min].value())) {
                    min = i;
                } else if (isEq(frontLine[i].value(), frontLine[min].value())) {
                    updateOnce(i);
                }
            }
            if (!hasMin) {
                break;
            }
            T minItem = frontLine[min].value();
            updateOnce(min);
            writeBuffer.push_back(std::move(minItem));
            if (static_cast<IndexT>(writeBuffer.size()) >= batchSize) {
                dropWriteBuffer();
            }
        }
        dropWriteBuffer();
    }
};

} // supermap
