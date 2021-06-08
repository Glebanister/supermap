#pragma once

#include "Filter.hpp"

namespace supermap {

/**
 * @brief A filter, that does not filters anything.
 * @tparam T Type to add to filter.
 */
template <typename T>
class MockFilter : public Filter<T> {
  public:
    /**
     * @brief Add @p elem to filter. Filter is mock, so nothing is actually happens.
     * @param elem Element to add.
     */
    void add(const T &) override {}

    /**
     * @brief Check if @p key could be added to filter.
     * Filter is mock, so it always assumes, that element could be in the filter.
     * @param elem An element to check.
     * @return Always @p true.
     */
    [[nodiscard]] bool mightContain(const T &) const override {
        return true;
    }

    /**
     * @return Ownership of filter with the same state. State is empty,
     * so just new @p MockFilter is being created.
     */
    std::unique_ptr<Filter<T>> clone() const override {
        return std::make_unique<MockFilter<T>>();
    }
};

} // supermap
