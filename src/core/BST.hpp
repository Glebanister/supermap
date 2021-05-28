#pragma once

#include <string>

#include "KeyValueStorage.hpp"

namespace supermap {

template <std::size_t KeyLen, std::size_t ValueLen>
class BST : public KeyValueStorage<KeyLen, ValueLen> {
  public:
    void add(const Key<KeyLen> &, ByteArray<ValueLen> &&) override {
        throw NotImplementedException();
    }

    void remove(const Key<KeyLen> &) override {
        throw NotImplementedException();
    }

    bool containsKey(const Key<KeyLen> &) override {
        throw NotImplementedException();
    }

    const ByteArray<ValueLen> &getValue(const Key<KeyLen> &) override {
        throw NotImplementedException();
    }
};

} // supermap
