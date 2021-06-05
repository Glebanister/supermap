#pragma once

#include <optional>
#include <functional>

namespace supermap {

/**
 * @brief An abstract storage where element of type @p T can be found.
 * @tparam T Findable object type.
 */
template <typename T>
class Findable {
  public:
    /**
     * @brief Finds an objects, that fulfills @p equal unary predicate.
     * @param less Unary predicate which indicates if an argument is less than the one required.
     * @param equal Unary predicate which indicates if an argument equals to the one required.
     * @return @p std::nullopt if there is no element, which fulfills predicate.
     * Not empty @p std::optional<T> is returned otherwise.
     */
    virtual std::optional<T> find(std::function<bool(const T &)> less, std::function<bool(const T &)> equal) = 0;
};

} // supermap
