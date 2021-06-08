#pragma once

#include "core/KeyValueStorage.hpp"
#include "primitive/Bounds.hpp"
#include "primitive/MaybeRemovedValue.hpp"
#include "io/SerializeHelper.hpp"
#include "exception/SupermapException.hpp"

namespace supermap {

template <
    typename Key,
    typename Value,
    typename Size
>
class DefaultFilteredKvs : public KeyValueStorage<Key, Value, Size> {
  private:
    using KVS = KeyValueStorage<Key, Value, Size>;
    using FilterBase = Filter<Key>;

  public:
    explicit DefaultFilteredKvs(
        std::unique_ptr<KVS> &&innerStorage,
        std::unique_ptr<FilterBase> &&filter
    ) : innerStorage_(std::move(innerStorage)),
        filter_(std::move(filter)) {}

    void add(const Key &key, Value &&value) override {
        filter_->add(key);
        innerStorage_->add(key, std::move(value));
    }

    std::optional<Value> getValue(const Key &key) override {
        if (!filter_->mightContain(key)) {
            return std::nullopt;
        }
        return innerStorage_->getValue(key);
    }

    Size getUpperSizeBound() const override {
        return innerStorage_->getUpperSizeBound();
    }

    void remove(const Key &key) override {
        innerStorage_->remove(key);
    }

  private:
    std::unique_ptr<KVS> innerStorage_;
    std::unique_ptr<FilterBase> filter_;
};

} // supermap
