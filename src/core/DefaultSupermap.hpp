#pragma once

#include "Supermap.hpp"
#include "io/EncapsulatedFileManager.hpp"
#include "MockFilter.hpp"
#include "DefaultRemovableKvs.hpp"

namespace supermap {

template <
    std::size_t KeyLen,
    std::size_t ValueLen,
    std::size_t MaxRamLoad,
    std::size_t MaxNotSortedSize,
    std::size_t KeyIndexBatchSize
>
class DefaultSupermap {
  public:
    using K = Key<KeyLen>;
    using I = std::size_t;
    using V = ByteArray<ValueLen>;
    using MV = MaybeRemovedValue<V>;
    using KI = KeyValue<K, I>;
    using B = Bounds<I>;
    using Default = Supermap<K, MV, I, MaxRamLoad>;

    using RamType = supermap::BST<K, I, I>;
    using IndexStorageListBase = typename Default::IndexStorageListBase;
    using IndexStorageBase = typename Default::IndexStorageBase;
    using DiskStorage = typename Default::DiskStorage;
    using RegisterInfo = typename Default::RegisterInfo;
    using Register = typename Default::Register;
    using DefaultBinaryCollapsingList = BinaryCollapsingSortedStoragesList<KI, I, MaxRamLoad, RegisterInfo, K>;

    static std::shared_ptr<RemovableKvs<K, V, B>> make() {
        using MyBinaryCollapsingList = BinaryCollapsingSortedStoragesList<KI, I, MaxRamLoad, RegisterInfo, K>;

        std::shared_ptr<supermap::io::FileManager> fileManager
            = std::make_shared<supermap::io::EncapsulatedFileManager>(
                std::make_shared<supermap::io::TemporaryFolder>(".supermap", true),
                std::make_unique<supermap::io::DiskFileManager>()
            );

        std::function<bool(I, I)> shouldShrinkChecker = [](I ns, I) { return ns >= MaxNotSortedSize; };

        std::function<std::unique_ptr<IndexStorageBase>(IndexStorageBase &&)>
            indexSupplier = [](IndexStorageBase &&sortedStorage) {
            return std::make_unique<IndexStorageBase>(std::move(sortedStorage));
        };

        std::function<std::unique_ptr<IndexStorageListBase>()> indexListSupplier = []() {
            return std::make_unique<MyBinaryCollapsingList>(
                KeyIndexBatchSize,
                []() {
                    return std::make_unique<FilteringRegister<KI, K>>
                        (
                            []() {
                                return std::make_unique<MockFilter<KI, K>>
                                    ();
                            }
                        );
                }
            );
        };

        std::function<std::unique_ptr<Register>()> innerRegisterSupplier
            = []() {
                return std::make_unique<FilteringRegister<KI, K>>
                    (
                        []() {
                            return std::make_unique<MockFilter<KI, K>>
                                ();
                        }
                    );
            };

        std::unique_ptr<KeyValueStorage<K, MV, B>> kvs = std::make_unique<Default>(
            std::make_unique<RamType>(),
            std::make_unique<DiskStorage>(
                "storage-not-sorted",
                "storage-sorted",
                fileManager,
                innerRegisterSupplier
            ),
            [](I notSortedSize, I) {
                return notSortedSize >= MaxNotSortedSize;
            },
            [](IndexStorageBase &&sortedStorage) {
                return std::make_unique<IndexStorageBase>(std::move(sortedStorage));
            },
            indexListSupplier,
            []() {
                return std::make_unique<MockFilter<KI, K>>
                    ();
            },
            KeyIndexBatchSize
        );

        return std::make_shared<DefaultRemovableKvs<K, V, B>>(std::move(kvs));
    }
};

} // supermap