#pragma once

#include <map>

#include "KeyValueStorage.hpp"

namespace supermap {

template <std::size_t KeyLen, std::size_t ValueLen>
class BST : public KeyValueStorage<KeyLen, ValueLen> {
  public:
    void add(const Key<KeyLen> &key, ByteArray<ValueLen> &&value) override {
        map_[key] = std::move(value);
    }

    void remove(const Key<KeyLen> &key) override {
        map_.erase(key);
    }

    bool containsKey(const Key<KeyLen> &key) override {
        return map_.find(key) != map_.end();
    }

    const ByteArray<ValueLen> &getValue(const Key<KeyLen> &key) override {
        return *map_.find(key);
    }

  private:
    std::map<Key<KeyLen>, ByteArray<ValueLen>> map_;
};

} // supermap
