#pragma once

namespace supermap {

/**
 * @brief An abstract registrar of storage items,
 * which aims to add new items to the storage.
 * @tparam T Storage content type.
 * @tparam Info Registration information type.
 */
template <typename T, typename Info>
class StorageItemRegister {
  public:
    using ItemsInfo = Info;

    /**
     * @brief Registers an item in storage.
     */
    virtual void registerItem(const T &) = 0;

    /**
     * @return Information about already registered items.
     */
    virtual Info getRegisteredItemsInfo() const noexcept = 0;

    virtual ~StorageItemRegister() = default;
};

} // supermap
