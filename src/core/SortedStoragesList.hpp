#pragma once

#include "SortedSingleFileIndexedStorage.hpp"

namespace supermap {

/**
 * @brief Abstract list of single file sorted storages.
 * @tparam T Each storage content type.
 * @tparam IndexT Each storage order type.
 * @tparam EachStorageRegisterInfo Each storage item register type.
 * @tparam FindPattern Type of pattern to find in list.
 */
template <
    typename T,
    typename IndexT,
    typename EachStorageRegisterInfo,
    typename FindPattern,
    typename SortedStorage = SortedSingleFileIndexedStorage<T, IndexT, EachStorageRegisterInfo, FindPattern>
>
class SortedStoragesList
    : public Findable<T, FindPattern>,
      public OrderedStorage<SortedStorage, IndexT, void> {
  public:
    using ContentType = T;
    using IndexType = IndexT;
    using RegisterType = EachStorageRegisterInfo;
    using FindPatternType = FindPattern;
    using InnerRegisterSupplier = typename OrderedStorage<SortedStorage,
                                                          IndexT,
                                                          void>::CountingRegister::InnerRegisterSupplier;
    using StoragesRegister = VoidRegister<SortedStorage>;

    SortedStoragesList()
        : OrderedStorage<SortedStorage, IndexT, void>([]() { return std::make_unique<StoragesRegister>(); }) {}
};

} // supermap
