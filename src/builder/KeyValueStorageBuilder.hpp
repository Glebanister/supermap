#pragma once

#include "core/KeyValueStorage.hpp"
#include "core/BST.hpp"
#include "DefaultRemovableKvs.hpp"
#include "DefaultSupermap.hpp"
#include "DefaultFilteredKvs.hpp"

namespace supermap {

namespace detail {

template <typename Key, typename Value, typename Size>
class FilteredBuilder;

template <typename Key, typename Value, typename Size>
class RemovableBuilder;

template <typename Key, typename Value, typename Size>
class PrimitiveKvsBuilder;

template <typename Key, typename Value, typename Size>
class KeyValueStorageBuilder {
  public:
    virtual std::unique_ptr<KeyValueStorage<Key, Value, Size>> build() = 0;

    auto filtered(std::unique_ptr<Filter<Key>> &&filter) {
        return FilteredBuilder<Key, Value, Size>(*this, std::move(filter));
    }

    auto removable() {
        return RemovableBuilder<Key, typename MaybeRemovedHelper<Value>::Type, Size>(*this);
    }
};

template <typename Key, typename Value, typename Size>
class FilteredBuilder : public KeyValueStorageBuilder<Key, Value, Size> {
  private:
    using FilteredKvs = DefaultFilteredKvs<Key, Value, Size>;

  public:
    explicit FilteredBuilder(
        KeyValueStorageBuilder<Key, Value, Size> &parent,
        std::unique_ptr<Filter<Key>> &&filter
    ) : parent_(parent), filter_(std::move(filter)) {}

    std::unique_ptr<KeyValueStorage<Key, Value, Size>> build() override {
        return std::make_unique<FilteredKvs>(parent_.build(), std::move(filter_));
    }

  private:
    KeyValueStorageBuilder<Key, Value, Size> &parent_;
    std::unique_ptr<Filter<Key>> filter_;
};

template <typename Key, typename Value, typename Size>
class RemovableBuilder : public KeyValueStorageBuilder<Key, Value, Size> {
  private:
    using MaybeRemovedV = MaybeRemovedValue<Value>;
    using MaybeRemovedKvs = KeyValueStorage<Key, MaybeRemovedV, Size>;

  public:
    explicit RemovableBuilder(KeyValueStorageBuilder<Key, MaybeRemovedV, Size> &parent)
        : parent_(parent) {}

    std::unique_ptr<KeyValueStorage<Key, Value, Size>> build() override {
        return std::make_unique<DefaultRemovableKvs<Key, Value, Size>>(parent_.build());
    }

  private:
    KeyValueStorageBuilder<Key, MaybeRemovedValue<Value>, Size> &parent_;
};

template <typename Key, typename Value, typename Size>
class PrimitiveKvsBuilder : public KeyValueStorageBuilder<Key, Value, Size> {
  private:
    using KVS = KeyValueStorage<Key, Value, Size>;

  public:
    explicit PrimitiveKvsBuilder(std::unique_ptr<KVS> &&kvs)
        : kvs_(std::move(kvs)) {}

    std::unique_ptr<KVS> build() override {
        return std::move(kvs_);
    }

  private:
    std::unique_ptr<KVS> kvs_;
};

}

namespace builder {

template <typename Key, typename Value, typename Size>
auto fromKvs(std::unique_ptr<KeyValueStorage<Key, Value, Size>> &&kvs) {
    return detail::PrimitiveKvsBuilder<Key, Value, Size>(std::move(kvs));
}

} // builder

} // supermap
