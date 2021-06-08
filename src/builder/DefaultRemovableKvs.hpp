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
class DefaultRemovableKvs : public KeyValueStorage<Key, Value, Size> {
  public:
    using KeyValueStorageMaybeValue = KeyValueStorage<Key, MaybeRemovedValue<Value>, Size>;

    explicit DefaultRemovableKvs(std::unique_ptr<KeyValueStorageMaybeValue> &&innerStorage)
        : storageOfMaybeRemoved_(std::move(innerStorage)) {}

    void add(const Key &key, Value &&value) override {
        assert(storageOfMaybeRemoved_);
        storageOfMaybeRemoved_->add(key, MaybeRemovedValue<Value>{std::move(value), false});
    }

    std::optional<Value> getValue(const Key &key) override {
        assert(storageOfMaybeRemoved_);
        std::optional<MaybeRemovedValue<Value>> optMaybeRemoved = storageOfMaybeRemoved_->getValue(key);
        if (!optMaybeRemoved.has_value()) {
            return std::nullopt;
        }
        MaybeRemovedValue<Value> maybeInnerValue = optMaybeRemoved.value();
        if (maybeInnerValue.removed) {
            return std::nullopt;
        }
        return std::optional{maybeInnerValue.value};
    }

    Size getUpperSizeBound() const override {
        assert(storageOfMaybeRemoved_);
        return storageOfMaybeRemoved_->getUpperSizeBound();
    }

    void remove(const Key &key) override {
        assert(storageOfMaybeRemoved_);
        storageOfMaybeRemoved_->add(key, MaybeRemovedValue<Value>{Value{}, true});
    }

  private:
    std::unique_ptr<KeyValueStorage<Key, MaybeRemovedValue<Value>, Size>> storageOfMaybeRemoved_;
};

} // supermap
