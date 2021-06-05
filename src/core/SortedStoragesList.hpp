#pragma once

#include "SortedSingleFileIndexedStorage.hpp"

namespace supermap {

/**
 * @brief Abstract list of single file sorted storages.
 * @tparam T Each storage content type.
 * @tparam IndexT Each storage order type.
 * @tparam EachStorageRegisterType Each storage item register type.
 * @tparam SortedStorage Sorted storages type.
 * @tparam StoragesRegister List storages register.
 */
template <
    typename T,
    typename IndexT,
    typename EachStorageRegisterType,
    typename SortedStorage = SortedSingleFileIndexedStorage<T, IndexT, EachStorageRegisterType>,
    typename StoragesRegister = VoidRegister<SortedStorage>
>
class SortedStoragesList
    : public Findable<T>,
      public OrderedStorage<SortedStorage, IndexT, StoragesRegister> {
};

} // supermap
