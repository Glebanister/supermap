#pragma once

#include <algorithm>
#include <optional>

#include "SingleFileIndexedStorage.hpp"

namespace supermap {

/**
 * @brief Single file storage where objects are sorted in increasing order,
 * defined by comparator.
 * @tparam T Stored objects type.
 * @tparam IndexT Storage index type.
 */
template <typename T, typename IndexT>
class SortedSingleFileIndexedStorage : public SingleFileIndexedStorage<T, IndexT> {
  public:
    using SingleFileIndexedStorage<T, IndexT>::getItemsCount;
    using SingleFileIndexedStorage<T, IndexT>::get;
    using SingleFileIndexedStorage<T, IndexT>::append;
    using SingleFileIndexedStorage<T, IndexT>::appendAll;

    /**
     * @brief Creates a sorted storage instance of size @p size.
     * @param size Size of items in storage file.
     * @param dataFileName Name of file where storage is placed.
     * @param manager Shared access to the file manager.
     */
    explicit SortedSingleFileIndexedStorage(IndexT size,
                                            std::string dataFileName,
                                            std::shared_ptr<io::FileManager> manager)
        : SingleFileIndexedStorage<T, IndexT>(std::move(dataFileName), manager, size) {
    }

    /**
     * @brief Creates new sorted storage from objects collection.
     * @tparam Iterator Collections iterator type.
     * @tparam IsLess Less comparator type.
     * @tparam IsEq Equal comparator type
     * @param begin Collection begin iterator.
     * @param end Collection end iterator.
     * @param sorted Flag if data in collection is sorted.
     * @param dataFileName File where storage will take place.
     * @param manager Shared access to the file manager.
     * @param isLess Comparator which returns if first argument is less then second.
     * @param isEq Comparator which returns if second argument equal to the second.
     */
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

    /**
     * @brief Sorts and uniques collection between @p begin and @p end.
     * The closer object to the end of collection, then it is more relevant.
     * After execution, there are unique, sorted and the most relevant objects
     * in collections.
     * @tparam LessComp Type of @p isLess comparator.
     * @tparam EqComp Type of @p isEq comparator.
     * @tparam Iterator Type of collection iterator.
     * @param begin Collection begin iterator.
     * @param end Collection end iterator.
     * @param isLess Comparator which returns if the first argument is less than the second.
     * @param isEq Comparator which returns if the first argument equals to the second.
     * @return Collection new end iterator.
     */
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

    /**
     * @brief Finds an objects, that fulfills @p equal unary predicate.
     * @param less Unary predicate which indicates if an argument is less than the one required.
     * @param equal Unary predicate which indicates if an argument equals to the one required.
     * @return @p std::nullopt if there is no element, which fulfills predicate.
     * Not empty @p std::optional<T> is returned otherwise.
     */
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

    /**
     * @brief Creates merged sorted storage from all @p newer sorted storages.
     * @tparam IsLess @p isLess comparator type.
     * @tparam IsEq @p isEq comparator type.
     * @param newer Sorted storages, which are ordered from least to the most relevant.
     * @param dataFileName New storage file name.
     * @param fileManager Shared access to the file manager.
     * @param isLess Comparator which returns if the first argument is less then the second one.
     * @param isEq Comparator which return if the first argument equals to the second one.
     * @param batchSize The size of the batch of @p T objects that are simultaneously stored in RAM.
     */
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
