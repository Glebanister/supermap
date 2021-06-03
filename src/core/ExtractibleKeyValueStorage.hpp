#pragma once

#include "KeyValueStorage.hpp"

namespace supermap {

template <typename Key, typename Value, typename IndexT>
class ExtractibleKeyValueStorage : public KeyValueStorage<Key, Value, IndexT> {
  public:
    virtual std::vector<KeyValue<Key, Value>> extract() && = 0;
};

} // supermap
