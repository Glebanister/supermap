#pragma once

#include "SortedSingleFileIndexedStorage.hpp"
#include "FilteringRegister.hpp"

namespace supermap {

/**
 * @brief Sorted single file indexed storage with filter as a additional register.
 * @tparam IndexT Storage order type.
 * @tparam FilterT Filter type. Must be an inheritor of @p supermap::Filter.
 */
template <
    typename Content,
    typename IndexT,
    typename FindPattern,
    typename Register = FilteringRegister<Content, FindPattern>,
    typename RegisterInfo = typename Register::ItemsInfo
>
class FilteredStorage : public SortedSingleFileIndexedStorage<Content, IndexT, RegisterInfo, FindPattern> {
  public:
    using SortedStorage = SortedSingleFileIndexedStorage<Content, IndexT, RegisterInfo, FindPattern>;
    using SortedStorage::find;
    using FilterBase = Filter<FindPattern>;
    using FilterSupplier = std::function<std::unique_ptr<FilterBase>()>;

    /**
     * @brief Create new filtered storage instance calling merge c-tor of @p SortedStorage.
     * @param newer List of sorted storages, ordered from less to the most relevant.
     * @param dataFileName Name of associated data file.
     * @param fileManager Shared access to the file manager.
     * @param batchSize Key-index top simultaneous presence in RAM.
     * @param filterSupplier Supplier of filter which will be attached to every inner storage.
     */
    explicit FilteredStorage(
        const std::vector<SortedStorage> &newer,
        std::string dataFileName,
        std::shared_ptr<io::FileManager> fileManager,
        IndexT batchSize,
        FilterSupplier filterSupplier
    ) : SortedSingleFileIndexedStorage<Content, IndexT, RegisterInfo, FindPattern>(
        newer,
        std::move(dataFileName),
        std::move(fileManager),
        batchSize,
        [filterSupplier]() { return std::make_unique<Register>(std::move(filterSupplier)); }
    ) {}

    explicit FilteredStorage(SortedStorage &&sortedStorage)
        : SortedSingleFileIndexedStorage<Content, IndexT, RegisterInfo, FindPattern>(std::move(sortedStorage)) {}

    /**
     * @brief Searches for the fulfillment of the predicate @p equal in all storages,
     * starting from the last added storages. Accesses disk storage
     * only if @p pattern was not filtered by filter.
     * @param pattern Find pattern
     * @param less Predicate, accepts object from storage and pattern, returns if object is less then pattern.
     * @param equal Predicate, accepts object from storage and pattern, returns if this is equals to pattern.
     * @return @p std::nullopt iff object predicate is not fulfilled by any object in all storages,
     * non-empty @p std::optional<T> otherwise.
     */
    std::optional<Content> find(
        const FindPattern &pattern,
        std::function<bool(const Content &, const FindPattern &)> less,
        std::function<bool(const Content &, const FindPattern &)> equal
    ) override {
        std::shared_ptr<FilterBase> filter = this->getRegisterInfo().additional;
        if (!filter->mightContain(pattern)) {
            return std::nullopt;
        }
        return SortedStorage::find(pattern, std::move(less), std::move(equal));
    }
};

} // supermap
