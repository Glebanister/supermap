#pragma once

#include "KeyValueStorage.hpp"

namespace supermap {

/**
 * @brief KeyValueStorage that can be extracted to the vector.
 * @tparam Key key type.
 * @tparam Value value type.
 * @tparam IndexT storage index type.
 */
template <typename Key, typename Value, typename IndexT>
class ExtractibleKeyValueStorage : public KeyValueStorage<Key, Value, IndexT> {
  public:

    /**
     * @brief Move all inner key-value pairs to the vector, emptying
     * inner storage, such that the @p getSize will return 0.
     */
    virtual std::vector<KeyValue < Key, Value>> extract() && = 0;
};

} // supermap
