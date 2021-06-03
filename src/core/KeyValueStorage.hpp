#pragma once

#include <string>

namespace supermap {

template <typename Key, typename Value, typename IndexT>
class KeyValueStorage {
  public:
    virtual void add(const Key &key, Value &&value) = 0;

    virtual bool containsKey(const Key &key) = 0;

    virtual const Value &getValue(const Key &key) = 0;

    virtual IndexT getSize() const noexcept = 0;

    virtual ~KeyValueStorage() = default;
};

} // supermap
