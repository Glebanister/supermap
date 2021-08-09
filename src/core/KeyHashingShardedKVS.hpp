#pragma once

#include "core/KeyValueStorage.hpp"
#include "hasher/HashTools.hpp"

namespace supermap {

/**
 * @brief A key-value storage, that manipulates with N smaller key-value storages as shards.
 * Key is assigned to shard if its hash
 * @tparam Key
 * @tparam Value
 * @tparam IndexT
 */
template <
    typename Key,
    typename Value,
    typename IndexT
>
class KeyHashingShardedKVS : public KeyValueStorage<Key, Value, IndexT> {
  private:
    static constexpr std::uint64_t HASH_SEED = 239;

    using KVS = KeyValueStorage<Key, Value, IndexT>;

  public:
    explicit KeyHashingShardedKVS(
        std::vector<std::unique_ptr<KVS>> &&nestedStorages,
        std::unique_ptr<Hasher> &&hasher
    ) : nestedStorages_(std::move(nestedStorages)),
        hasher_(std::move(hasher)) {}

    void add(const Key &key, Value &&value) override {
        nestedStorages_[getShardIdForKey(key)]->add(key, std::move(value));
    }

    std::optional<Value> getValue(const Key &key) override {
        return nestedStorages_[getShardIdForKey(key)]->getValue(key);
    }

    IndexT getUpperSizeBound() const override {
        IndexT size = 0;
        for (const auto &shard : nestedStorages_) {
            size += shard->getUpperSizeBound();
        }
        return size;
    }

  private:
    std::size_t getShardIdForKey(const Key &key) {
        return hashTools::hashWith(*hasher_, key, HASH_SEED) % nestedStorages_.size();
    }

    std::vector<std::unique_ptr<KVS>> nestedStorages_;
    std::unique_ptr<Hasher> hasher_;
};

} // namespace supermap
