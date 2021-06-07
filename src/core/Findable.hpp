#pragma once

#include <optional>
#include <functional>

namespace supermap {

/**
 * @brief An abstract storage where element of type @p T can be found.
 * @tparam T Findable object type.
 * @tparam P Find pattern type.
 */
template <typename T, typename P>
class Findable {
  public:

    /**
     * @brief Searches for the fulfillment of the predicate @p equal in all storages,
     * starting from the last added storages.
     * @param pattern Find pattern
     * @param less Predicate, accepts object from storage and pattern, returns if object is less then pattern.
     * @param equal Predicate, accepts object from storage and pattern, returns if this is equals to pattern.
     * @return @p std::nullopt iff object predicate is not fulfilled by any object in all storages,
     * non-empty @p std::optional<T> otherwise.
     */
    virtual std::optional<T> find(
        const P &pattern,
        std::function<bool(const T &, const P &)> less,
        std::function<bool(const T &, const P &)> equal
    ) = 0;
};

} // supermap
