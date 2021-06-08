#pragma once

#include "Cloneable.hpp"

#include <string>

namespace supermap {

/**
 * @brief Filter, which can add elements to self and tell
 * which elements are certainly were not added to filter.
 * @tparam T Parameter to add to filter.
 */
template <typename T>
class Filter : public Cloneable<Filter<T>> {
  public:
    /**
     * @brief Add @p elem to filter.
     * @param elem Element to add.
     */
    virtual void add(const T &elem) = 0;

    /**
     * @brief Check if @p key could be added to filter.
     * It means, that false positive result has place to be,
     * but not false negative.
     * @param elem An element to check.
     * @return if @p false, then @p elem was never added to filter.
     */
    [[nodiscard]] virtual bool mightContain(const T &elem) const = 0;

    /**
     * @brief Reserve filter for @p n elements.
     */
    virtual void reserve(std::uint64_t) {};
};

} // supermap
