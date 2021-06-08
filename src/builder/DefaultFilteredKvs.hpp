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
    using KI = KeyValue<Key, Size>;
    using FilterBase = Filter<KI, Key>;

  public:
    explicit DefaultFilteredKvs(
        std::unique_ptr<KVS> &&innerStorage,
        std::unique_ptr<FilterBase> &&filter
    ) : innerStorage_(std::move(innerStorage)),
        filter_(std::move(filter)) {}

    void add(const Key &, Value &&) override {
//        assert(storageOfMaybeRemoved_);
//        storageOfMaybeRemoved_->add(key, MaybeRemovedValue<Value>{std::move(value), false});
    }

    std::optional<Value> getValue(const Key &) override {
//        assert(storageOfMaybeRemoved_);
//        std::optional<MaybeRemovedValue<Value>> optMaybeRemoved = storageOfMaybeRemoved_->getValue(key);
//        if (!optMaybeRemoved.has_value()) {
//            return std::nullopt;
//        }
//        MaybeRemovedValue<Value> maybeInnerValue = optMaybeRemoved.value();
//        if (maybeInnerValue.removed) {
//            return std::nullopt;
//        }
//        return std::optional{maybeInnerValue.value};
    }

    Size getUpperSizeBound() const override {
//        assert(storageOfMaybeRemoved_);
//        return storageOfMaybeRemoved_->getUpperSizeBound();
    }

    void remove(const Key &) override {
//        assert(storageOfMaybeRemoved_);
//        storageOfMaybeRemoved_->add(key, MaybeRemovedValue<Value>{Value{}, true});
    }

  private:
    std::unique_ptr<KVS> innerStorage_;
    std::unique_ptr<FilterBase> filter_;
};

} // supermap
