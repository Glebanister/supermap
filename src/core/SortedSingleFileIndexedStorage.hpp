#pragma once

#include <algorithm>

#include "SingleFileIndexedStorage.hpp"
#include "Findable.hpp"

namespace supermap {

/**
 * @brief Single file storage where objects are sorted in increasing order,
 * defined by comparator.
 * @tparam T Stored objects type.
 * @tparam IndexT Storage index type.
 * @tparam RegisterInfo Inner register info type.
 */
template <
    typename T,
    typename IndexT,
    typename RegisterInfo,
    typename FindPattern
>
class SortedSingleFileIndexedStorage : public SingleFileIndexedStorage<T, IndexT, RegisterInfo>,
                                       public Findable<T, FindPattern> {
  public:
    using SingleFileIndexedStorage<T, IndexT, RegisterInfo>::getItemsCount;
    using SingleFileIndexedStorage<T, IndexT, RegisterInfo>::get;
    using SingleFileIndexedStorage<T, IndexT, RegisterInfo>::append;
    using SingleFileIndexedStorage<T, IndexT, RegisterInfo>::appendAll;
    using SingleFileIndexedStorage<T, IndexT, RegisterInfo>::SingleFileIndexedStorage;
    using OrderedStorage<T, IndexT, RegisterInfo>::getRegister;
    using InnerRegisterSupplier = typename SingleFileIndexedStorage<T, IndexT, RegisterInfo>::InnerRegisterSupplier;

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
                                            IsEq isEq,
                                            InnerRegisterSupplier registerSupplier
    ) : SingleFileIndexedStorage<T, IndexT, RegisterInfo>(std::move(dataFileName),
                                                          manager,
                                                          std::move(registerSupplier)) {
        getRegister().reserve(std::distance(begin, end));
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
     * @brief Searches for the fulfillment of the predicate @p equal in all storages,
     * starting from the last added storages.
     * @param pattern Find pattern
     * @param less Predicate, accepts object from storage and pattern, returns if object is less then pattern.
     * @param equal Predicate, accepts object from storage and pattern, returns if this is equals to pattern.
     * @return @p std::nullopt iff object predicate is not fulfilled by any object in all storages,
     * non-empty @p std::optional<T> otherwise.
     */
    std::optional<T> find(
        const FindPattern &pattern,
        std::function<bool(const T &, const FindPattern &)> less,
        std::function<bool(const T &, const FindPattern &)> equal
    ) override {
        if (getItemsCount() == 0) {
            return std::nullopt;
        }
        IndexT firstLeq = 0;
        IndexT lastGt = getItemsCount();
        while (lastGt - firstLeq > 1) {
            IndexT middle = (firstLeq + lastGt) / 2;
            T middleElem = get(middle);
            if (less(middleElem, pattern) || equal(middleElem, pattern)) {
                firstLeq = middle;
            } else {
                lastGt = middle;
            }
        }
        if (firstLeq >= getItemsCount()) {
            return std::nullopt;
        }
        T firstLeqElem = get(firstLeq);
        return equal(firstLeqElem, pattern) ? std::optional{firstLeqElem} : std::nullopt;
    }

    /**
     * @brief Creates merged sorted storage from all @p newer sorted storages.
     * @param newer Sorted storages, which are ordered from least to the most relevant.
     * @param dataFileName New storage file name.
     * @param fileManager Shared access to the file manager.
     * @param isLess Comparator which returns if the first argument is less then the second one.
     * @param isEq Comparator which return if the first argument equals to the second one.
     * @param batchSize The size of the batch of @p T objects that are simultaneously stored in RAM.
     */
    explicit SortedSingleFileIndexedStorage(
        const std::vector<SortedSingleFileIndexedStorage<T, IndexT, RegisterInfo, FindPattern>> &newer,
        std::string dataFileName,
        std::shared_ptr<io::FileManager> fileManager,
        IndexT batchSize,
        InnerRegisterSupplier registerSupplier
    ) : SingleFileIndexedStorage<T, IndexT, RegisterInfo>(
        std::move(dataFileName),
        std::move(fileManager),
        std::move(registerSupplier)
    ) {
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
        getRegister().reserve(totalSize);
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
                if (frontLine[i].value() < frontLine[min].value()) {
                    min = i;
                } else if (frontLine[i].value() == frontLine[min].value()) {
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
