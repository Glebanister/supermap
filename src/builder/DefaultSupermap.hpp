#pragma once

#include "core/Supermap.hpp"
#include "io/DiskFileManager.hpp"
#include "io/EncapsulatedFileManager.hpp"
#include "core/BloomFilter.hpp"

namespace supermap {

template <
    typename Key,
    typename Value,
    typename IndexT
>
class DefaultSupermap {
  public:
    struct BuildParameters {
        IndexT batchSize;
        double maxNotSortedPart{};
        std::string folderName;
        double errorProbability{};
    };

  public:
    using K = Key;
    using I = IndexT;
    using V = Value;
    using KI = KeyValue<K, I>;
    using Smap = Supermap<K, V, I>;

    using RamStorageBase = typename Smap::RamStorageBase;
    using IndexStorageListBase = typename Smap::IndexStorageListBase;
    using IndexStorageBase = typename Smap::IndexStorageBase;
    using FilteredIndexStorage = FilteredStorage<KI, I, K>;
    using DiskStorage = typename Smap::DiskStorage;
    using RegisterInfo = typename Smap::RegisterInfo;
    using Register = typename Smap::RegisterBase;
    using DefaultBinaryCollapsingList = BinaryCollapsingSortedStoragesList<KI, I, RegisterInfo, K>;

  private:
    using KVS = KeyValueStorage<Key, Value, IndexT>;

  public:
    static std::unique_ptr<KVS> build(
        std::unique_ptr<RamStorageBase> &&nested,
        const BuildParameters &params
    ) {
        std::shared_ptr<supermap::io::FileManager> fileManager
            = std::make_shared<supermap::io::EncapsulatedFileManager>(
                std::make_shared<supermap::io::TemporaryFolder>(params.folderName, true),
                std::make_unique<supermap::io::DiskFileManager>()
            );

        std::function<std::unique_ptr<IndexStorageBase>(IndexStorageBase &&)>
            indexSupplier = [](IndexStorageBase &&sortedStorage) {
            return std::make_unique<FilteredIndexStorage>(std::move(sortedStorage));
        };

        std::function<std::unique_ptr<Register>()>
            innerRegisterSupplier = [errorProbability = params.errorProbability]() {
            return std::make_unique<FilteringRegister<KI, K>>(
                [errorProbability = errorProbability]() {
                    return std::make_unique<BloomFilter<K>>(
                        errorProbability
                    );
                },
                [](const KI &ki) { return ki.key; }
            );
        };

        std::function<std::unique_ptr<IndexStorageListBase>()>
            indexListSupplier = [maxRamLoad = params.batchSize, registerSupplier = innerRegisterSupplier]() {
            return std::make_unique<DefaultBinaryCollapsingList>(
                maxRamLoad,
                registerSupplier
            );
        };

        return std::unique_ptr<KVS>(
            new Smap(
                std::move(nested),
                std::make_unique<DiskStorage>(
                    "storage-not-sorted",
                    "storage-sorted",
                    fileManager,
                    innerRegisterSupplier
                ),
                indexSupplier,
                indexListSupplier,
                innerRegisterSupplier,
                params.batchSize,
                params.maxNotSortedPart
            )
        );
    }
};

} // supermap
