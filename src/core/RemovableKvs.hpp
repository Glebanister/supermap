#pragma once

#include "KeyValueStorage.hpp"

namespace supermap {

/**
 * @brief A key-value storage, from where keys can be removed.
 * @tparam Key Key type.
 * @tparam Value Value type.
 * @tparam Size Size type.
 */
template <typename Key, typename Value, typename Size>
class RemovableKvs : public KeyValueStorage<Key, Value, Size> {
  public:
    /**
     * @brief Remove @p key from storage.
     * @param key Key to remove.
     */
    virtual void remove(const Key &key) = 0;
};

} // supermap
