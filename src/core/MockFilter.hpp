#pragma once

#include "Filter.hpp"

namespace supermap {

/**
 * @brief A filter, that does not filters anything.
 * @tparam ToAdd Type to add to filter.
 * @tparam ToContain Type to contain in filter.
 */
template <typename ToAdd, typename ToContain>
class MockFilter : public Filter<ToAdd, ToContain> {
  public:
    /**
     * @brief Add @p elem to filter. Filter is mock, so nothing is actually happens.
     * @param elem Element to add.
     */
    void add(const ToAdd &) override {}

    /**
     * @brief Check if @p key could be added to filter.
     * Filter is mock, so it always assumes, that element could be in the filter.
     * @param elem An element to check.
     * @return Always @p true.
     */
    [[nodiscard]] bool mightContain(const ToContain &) const override {
        return true;
    }

    /**
     * @return Ownership of filter with the same state. State is empty,
     * so just new @p MockFilter is being created.
     */
    std::unique_ptr<Filter<ToAdd, ToContain>> clone() const override {
        return std::make_unique<MockFilter<ToAdd, ToContain>>();
    }
};

} // supermap
