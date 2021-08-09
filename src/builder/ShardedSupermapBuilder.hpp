#pragma once

#include "DefaultSupermap.hpp"
#include "core/KeyHashingShardedKVS.hpp"

namespace supermap {

template <typename Key, typename Value, typename IndexT>
class ShardedSupermapBuilder {
  private:
    using KVS = KeyValueStorage<Key, Value, IndexT>;
    using NestedKvs = ExtractibleKeyValueStorage<Key, IndexT, IndexT>;

  public:
    using BuildParameters = typename DefaultSupermap<Key, Value, IndexT>::BuildParameters;

  public:
    static std::unique_ptr<KeyValueStorage<Key, Value, IndexT>> build(
        std::size_t nShards,
        std::unique_ptr<Hasher> &&hasher,
        std::unique_ptr<NestedKvs> &&nested,
        const BuildParameters &params
    ) {
        std::vector<std::unique_ptr<NestedKvs>> nestedStorages(nShards);
        nestedStorages[0] = std::move(nested);
        for (std::size_t i = 1; i < nShards; ++i) {
            nestedStorages[i] = nestedStorages[0]->createLikeThis();
        }
        std::vector<std::unique_ptr<KVS>> storages(nShards);
        for (std::size_t shard = 0; shard < nShards; ++shard) {
            storages[shard] = DefaultSupermap<Key, Value, IndexT>::build(
                std::move(nestedStorages[shard]),
                BuildParameters{
                    params.batchSize,
                    params.maxNotSortedPart,
                    std::filesystem::path(params.folderName) / std::to_string(shard),
                    params.errorProbability,
                }
            );
        }

        return std::make_unique<KeyHashingShardedKVS<Key, Value, IndexT>>(
            std::move(storages),
            std::move(hasher)
        );
    }
};

} // namespace supermap
