#pragma once

#include "CountingStorageItemRegister.hpp"

namespace supermap {

/**
 * @brief An abstract storage, where elements has particular order.
 * Element added later is of lower order.
 * @tparam T Storage content type.
 * @tparam IndexT Storage order type.
 * @tparam RegisterType Storage register type.
 */
template <typename T, typename IndexT, typename RegisterType>
class OrderedStorage {
  public:
    /**
     * @brief Add element to the end of storage (which will have the greatest order).
     * Order of added element is @p getLastElementIndex().
     */
    virtual void append(std::unique_ptr<T> &&) = 0;

    /**
     * @brief Add copy of an element to the end of storage (which will have the greatest order).
     * Order of added element is @p getLastElementIndex().
     */
    void appendCopy(T elem) {
        append(std::make_unique<T>(std::move(elem)));
    }

    /**
     * @return Number of registered elements.
     */
    virtual IndexT getItemsCount() const noexcept {
        return register_.getRegisteredItemsInfo().count;
    }

    /**
     * @return Index of the last element in storage.
     */
    IndexT getLastElementIndex() const noexcept {
        auto count = getItemsCount();
        assert(count != 0);
        return count - 1;
    }

    /**
     * @return Registered items information.
     */
    const auto &getRegisterInfo() noexcept {
        return getRegister().getRegisteredItemsInfo();
    }

    virtual ~OrderedStorage() = default;

  protected:
    /**
     * @return Reference on associated storage item register.
     */
    CountingStorageItemRegister<T, IndexT, RegisterType> &getRegister() {
        return register_;
    }

  private:
    CountingStorageItemRegister<T, IndexT, RegisterType> register_;
};

} // supermap
