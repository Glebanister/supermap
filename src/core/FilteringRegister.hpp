#pragma once

#include "StorageItemRegister.hpp"
#include "Filter.hpp"

namespace supermap {

/**
 * @brief A storage register, which registers elements
 * in inner filter.
 * @tparam RegisterT Register content type.
 * @tparam FilterT Filter content type.
 */
template <
    typename RegisterT,
    typename FilterT
>
class FilteringRegister : public StorageItemRegister<RegisterT, std::shared_ptr<Filter<FilterT>>> {
  public:
    using FilterBase = Filter<FilterT>;

    explicit FilteringRegister(
        std::function<std::shared_ptr<FilterBase>()> filterSupplier,
        std::function<FilterT(const RegisterT &)> func
    ) : filter_(filterSupplier()), func_(std::move(func)) {}

    /**
     * @brief Registers an item in storage, by adding it to the inner filter.
     */
    void registerItem(const RegisterT &item) override {
        filter_->add(func_(item));
    }

    /**
     * @return Information about already registered items.
     */
    std::shared_ptr<FilterBase> getRegisteredItemsInfo() const noexcept override {
        return filter_;
    }

    /**
     * @return Ownership of register with same inner filter.
     */
    std::unique_ptr<StorageItemRegister<RegisterT, std::shared_ptr<FilterBase>>>
    clone() const override {
        return std::make_unique<FilteringRegister<RegisterT, FilterT>>(
            [filter = filter_]() { return filter->clone(); },
            func_
        );
    }

    /**
     * @brief Reserve inner filter storage for @p n elements.
     * @param n Reservation size.
     */
    void reserve(std::uint64_t n) override {
        filter_->reserve(n);
    }

  private:
    std::shared_ptr<FilterBase> filter_{};
    std::function<FilterT(const RegisterT &)> func_;
};

} // supermap
