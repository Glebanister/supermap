#pragma once

#include <string>

#include "ByteArray.hpp"
#include "Key.hpp"

namespace supermap {

template <std::size_t KeyLen, std::size_t ValueLen>
class KeyValueStorage {
  public:
    virtual void add(const Key<KeyLen> &key, ByteArray<ValueLen> &&value) = 0;

    virtual void remove(const Key<KeyLen> &key) = 0;

    virtual bool containsKey(const Key<KeyLen> &key) = 0;

    virtual const ByteArray<ValueLen> &getValue(const Key<KeyLen> &key) = 0;

    virtual ~KeyValueStorage() = default;
};

} // supermap
