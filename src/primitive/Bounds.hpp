#pragma once

namespace supermap {

/**
 * @brief Any min-max bounds.
 * @tparam IndexT Bounds type
 */
template <typename IndexT>
struct Bounds { IndexT min, max; };

} // supermap
