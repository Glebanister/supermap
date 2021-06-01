#pragma once

#include <algorithm>
#include <optional>

#include "SingleFileIndexedStorage.hpp"

namespace supermap {

template <typename T, typename IndexT = std::size_t>
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

    explicit SortedSingleFileIndexedStorage(std::vector<T> data,
                                            bool sorted,
                                            std::string dataFileName,
                                            std::shared_ptr<io::FileManager> manager)
        : SingleFileIndexedStorage<T, IndexT>(std::move(dataFileName), manager, 0) {
        appendAll(data.cbegin(), sorted ? data.cend() : sortedEndIterator(data));
    }

    SortedSingleFileIndexedStorage fromSorted(
        std::vector<T> data,
        const std::string &dataFileName,
        std::shared_ptr<io::FileManager> manager) {
        return SortedIndexedStorage(std::move(data), false, dataFileName, manager);
    }

    static typename std::vector<T>::const_iterator sortedEndIterator(std::vector<T> &v) {
        std::stable_sort(v.begin(), v.end());
        return std::unique(v.begin(), v.end());
    }

    SortedSingleFileIndexedStorage fromNotSorted(
        std::vector<T> data,
        const std::string &dataFileName,
        std::shared_ptr<io::FileManager> manager) {
        return SortedIndexedStorage(std::move(data), true, dataFileName, manager);
    }

    explicit SortedSingleFileIndexedStorage(
        const std::vector<SortedSingleFileIndexedStorage<T, IndexT>> &newer,
        std::string dataFileName,
        std::shared_ptr<io::FileManager> fileManager
    ) : SingleFileIndexedStorage<T, IndexT>(std::move(dataFileName), std::move(fileManager), 0) {

        const std::size_t storagesCount = newer.size();
        std::vector<std::size_t> currentFrontPointers(storagesCount);
        std::vector<std::optional<T>> frontLine(storagesCount, std::nullopt);
        std::uint64_t totalSize = 0;
        for (std::size_t i = 0; i < storagesCount; ++i) {
            if (newer[i].getItemsCount() > 0) {
                frontLine[i].emplace(newer[i].get(0));
                totalSize += newer[i].getItemsCount();
            }
        }

        for (std::uint64_t elemI = 0; elemI < totalSize; ++elemI) {
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
                }
            }
            assert(hasMin);
            T minItem = frontLine[min].value();
            if (++currentFrontPointers[min] < newer[min].getItemsCount()) {
                frontLine[min].emplace(newer[min].get(currentFrontPointers[min]));
            } else {
                frontLine[min].reset();
            }
            append(std::move(minItem)); // TODO: appendAll has lesser Write Amplification
        }
    }
};

} // supermap
