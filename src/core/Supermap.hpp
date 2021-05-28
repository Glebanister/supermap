#pragma once

#include <memory>

#include "KeyValueStorage.hpp"
#include "KeyValueShrinkableStorage.hpp"
#include "DiskIndex.hpp"

namespace supermap {

template <std::size_t KeyLen, std::size_t ValueLen>
class Supermap : public KeyValueStorage<KeyLen, ValueLen> {
  public:
    explicit Supermap(std::unique_ptr<KeyValueStorage<KeyLen, ValueLen>> &&innerStorage,
                      std::unique_ptr<KeyValueShrinkableStorage<KeyLen, ValueLen>> &&diskDataStorage,
                      std::unique_ptr<DiskIndex<KeyLen>> &&diskIndex)
        : innerStorage_(std::move(innerStorage)),
          diskDataStorage_(std::move(diskDataStorage)),
          diskIndex_(std::move(diskIndex)) {}

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

  private:
    std::unique_ptr<KeyValueStorage<KeyLen, ValueLen>> innerStorage_;
    std::unique_ptr<KeyValueShrinkableStorage<KeyLen, ValueLen>> diskDataStorage_;
    std::unique_ptr<DiskIndex<KeyLen>> diskIndex_;
};

} // supermap
