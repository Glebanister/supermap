#pragma once

#include <cstdint>
#include <type_traits>

#include "io/FileManager.hpp"
#include "OrderedStorage.hpp"

namespace supermap {

/**
 * @brief A storage that represents something from which you can
 * take an element by index and add an element to the end.
 * @tparam T Type of contained objects.
 * @tparam IndexT Type of storage index.
 * @tparam RegisterInfo Inner register info type.
 */
template <typename T, typename IndexT, typename RegisterInfo>
class IndexedStorage : public OrderedStorage<T, IndexT, RegisterInfo> {
  public:
    using OrderedStorage<T, IndexT, RegisterInfo>::OrderedStorage;

    /**
     * @return object @p T which has index @p index.
     */
    virtual T get(IndexT index) const = 0;
};

} // supermap
