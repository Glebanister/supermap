#pragma once

#include "StorageItemRegister.hpp"
#include "Filter.hpp"

namespace supermap {

/**
 * @brief A storage register, which registers elements
 * in inner filter.
 */
template <
    typename FilterToAdd,
    typename FilterToContain
>
class FilteringRegister : public StorageItemRegister<FilterToAdd,
                                                     std::shared_ptr<Filter<FilterToAdd, FilterToContain>>> {
  public:
    using FilterBase = Filter<FilterToAdd, FilterToContain>;

    explicit FilteringRegister(std::function<std::shared_ptr<FilterBase>()> filterSupplier)
        : filter_(filterSupplier()) {}

    /**
     * @brief Registers an item in storage, by adding it to the inner filter.
     */
    void registerItem(const FilterToAdd &item) override {
        filter_->add(item);
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
    std::unique_ptr<StorageItemRegister<FilterToAdd, std::shared_ptr<FilterBase>>>
    clone() const override {
        return std::make_unique<FilteringRegister<FilterToAdd, FilterToContain>>(
            [filter = filter_]() { return filter->clone(); }
        );
    }

  private:
    std::shared_ptr<FilterBase> filter_{};
};

} // supermap
