#pragma once

namespace supermap {

/**
 * @brief Any min-max bounds.
 * @tparam IndexT Bounds type
 */
template <typename IndexT>
struct Bounds {
    using Type = IndexT;

    IndexT min, max;
};

template <typename T>
struct IsBounds {
    static constexpr bool isBounds = false;
};

template <typename T>
struct IsBounds<Bounds<T>> {
    static constexpr bool isBounds = true;
    using Type = T;
};

} // supermap
