#pragma once

#include "primitive/Key.hpp"
#include "SortedSingleFileIndexedStorage.hpp"

namespace supermap {

template <std::size_t KeyLen, typename IndexT>
class BinaryCollapsingIndex : public SortedSingleFileIndexedStorage<Key<KeyLen>, IndexT> {
  public:

  private:
    std::size_t rank_;
};

} // supermap
