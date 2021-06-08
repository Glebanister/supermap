#pragma once

#include "Cloneable.hpp"

namespace supermap {

/**
 * @brief An abstract registrar of storage items,
 * which aims to add new items to the storage.
 * @tparam T Storage content type.
 * @tparam Info Registration information type.
 */
template <typename T, typename Info>
class StorageItemRegister : public Cloneable<StorageItemRegister<T, Info>> {
  public:
    using ToRegister = T;
    using ItemsInfo = Info;

    /**
     * @brief Registers an item in storage.
     */
    virtual void registerItem(const T &) = 0;

    /**
     * @return Information about already registered items.
     */
    virtual Info getRegisteredItemsInfo() const noexcept = 0;

    /**
     * @brief Reserve filter for storing @p n elements.
     */
    virtual void reserve(std::uint64_t) {}

    virtual ~StorageItemRegister() = default;
};

} // supermap
