#pragma once

#include "core/BST.hpp"

namespace supermap {

template <typename Key, typename Value, typename Size>
class BstBuilder {
  public:
    static std::unique_ptr<KeyValueStorage<Key, Value, Size>> build() {
        return std::make_unique<BST<Key, Value, Size>>();
    }
};

} // supermap
