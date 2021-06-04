#pragma once

#include <cstdint>
#include <type_traits>

#include "io/FileManager.hpp"

namespace supermap {

/**
 * @brief A storage that represents something from which you can
 * take an element by index and add an element to the end.
 * @tparam T Type of contained objects.
 * @tparam IndexT Type of storage index.
 */
template <typename T, typename IndexT>
class IndexedStorage {
  public:
    /**
     * @return Associated FileManager.
     */
    [[nodiscard]] virtual std::shared_ptr<io::FileManager> getFileManager() const noexcept = 0;

    /**
     * @brief Adds new @p item to the end of storage. Hence, its index must be greatest in the collection.
     * @param item Added element.
     * @return Index of added element.
     */
    virtual IndexT append(const T &item) = 0;

    /**
     * @return object @p T which has index @p index.
     */
    virtual T get(IndexT index) const = 0;

    /**
     * @return Number of stored elements.
     */
    virtual IndexT getItemsCount() const noexcept = 0;

    virtual ~IndexedStorage() = default;
};

} // supermap
