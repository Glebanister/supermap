#pragma once

#include "Cloneable.hpp"

#include <string>

namespace supermap {

/**
 * @brief Filter, which can add elements to self and tell
 * which elements are certainly were not added to filter.
 * @tparam ToAdd Parameter to add to filter.
 * @tparam ToContain Parameter to contain in filter.
 */
template <typename ToAdd, typename ToContain>
class Filter : public Cloneable<Filter<ToAdd, ToContain>> {
  public:
    using AddType = ToAdd;
    using ContainType = ToContain;

    /**
     * @brief Add @p elem to filter.
     * @param elem Element to add.
     */
    virtual void add(const ToAdd &elem) = 0;

    /**
     * @brief Check if @p key could be added to filter.
     * It means, that false positive result has place to be,
     * but not false negative.
     * @param elem An element to check.
     * @return if @p false, then @p elem was never added to filter.
     */
    [[nodiscard]] virtual bool mightContain(const ToContain &elem) const = 0;
};

} // supermap
