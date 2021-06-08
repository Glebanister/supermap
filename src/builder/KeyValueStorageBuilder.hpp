#pragma once

#include "core/KeyValueStorage.hpp"
#include "core/BST.hpp"
#include "DefaultRemovableKvs.hpp"
#include "DefaultSupermap.hpp"

namespace supermap {

namespace detail {

template <typename Key, typename Value, typename Size>
class RemovableBuilder;

template <typename Key, typename Value, typename Size>
class SupermapBuilder;

template <typename Key, typename Value, typename Size>
class PrimitiveKvsBuilder;

template <typename Key, typename Value, typename Size>
class KeyValueStorageBuilder {
  public:
    virtual std::unique_ptr<KeyValueStorage<Key, Value, Size>> build() = 0;

    auto removable() -> std::enable_if_t<MaybeRemovedHelper<Value>::isMaybeRemoved,
                                         RemovableBuilder<Key, typename MaybeRemovedHelper<Value>::Type, Size>> {
        return RemovableBuilder<Key, typename MaybeRemovedHelper<Value>::Type, Size>(*this);
    }

    auto supermap(typename DefaultSupermap<Key, Value, Size>::BuildParameters params) {
        return SupermapBuilder<Key, Value, Size>(*this, std::move(params));
    }
};

template <typename Key, typename Value, typename Size>
class RemovableBuilder : public KeyValueStorageBuilder<Key, Value, Size> {
  public:
    explicit RemovableBuilder(KeyValueStorage<Key, MaybeRemovedValue<Value>, Size> &parent)
        : parent_(parent) {}

    std::unique_ptr<KeyValueStorage<Key, Value, Size>> build() override {
        return std::make_unique<DefaultRemovableKvs<Key, Value, Size>>(parent_.build());
    }

  private:
    KeyValueStorageBuilder<Key, MaybeRemovedValue<Value>, Size> &parent_;
};

template <typename Key, typename Value, typename Size>
class SupermapBuilder : public KeyValueStorageBuilder<Key, Value, Size> {
  public:
    using BuildParams = typename DefaultSupermap<Key, Value, Size>::BuildParameters;

    explicit SupermapBuilder(KeyValueStorage<Key, Value, Size> &parent, BuildParams params)
        : parent_(parent), params_(std::move(params)) {}

    std::unique_ptr<KeyValueStorage<Key, Value, Size>> build() override {
        return DefaultSupermap<Key, Value, Size>::build(parent_.build(), params_);
    }

  private:
    KeyValueStorageBuilder<Key, Value, Size> &parent_;
    BuildParams params_;
};

template <typename Key, typename Value, typename Size>
class PrimitiveKvsBuilder : public KeyValueStorageBuilder<Key, Value, Size> {
  public:
    using KvsSupplier = std::function<std::unique_ptr<ExtractibleKeyValueStorage<Key, Value, Size>>()>;

    explicit PrimitiveKvsBuilder(KvsSupplier supplier)
        : kvsSupplier_(std::move(supplier)) {}

    std::unique_ptr<KeyValueStorage<Key, Value, Size>> build() override {
        return kvsSupplier_();
    }

  private:
    KvsSupplier kvsSupplier_;
};

}

namespace builder {

template <typename Key, typename Value, typename Size>
auto fromKvs(std::function<std::unique_ptr<KeyValueStorage<Key, Value, Size>>()> kvsSupplier) {
    return detail::PrimitiveKvsBuilder<Key, Value, Size>(std::move(kvsSupplier));
}

template <typename Key, typename Value, typename Size, template <typename, typename, typename> class Kvs>
auto fromPrimitiveKvs() {
    return fromKvs([]() {
        return std::make_unique<Kvs<Key, Value, Size>>();
    });
}

template <typename Key, typename Value, typename Size>
auto fromBst() {
    return fromPrimitiveKvs<Key, Value, Size, BST>();
}

} // builder

} // supermap
