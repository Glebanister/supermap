#pragma once

#include "Supermap.hpp"
#include "io/EncapsulatedFileManager.hpp"
#include "MockFilter.hpp"

namespace supermap {

template <
    typename Key,
    typename Value,
    typename IndexT
>
class DefaultBuilder {
  public:
    struct BuildParameters {
        IndexT batchSize;
        double maxNotSortedPart;
        std::string folderName;
    };

  public:
    using K = Key;
    using I = IndexT;
    using V = Value;
    using KI = KeyValue<K, I>;
    using B = Bounds<I>;
    using Smap = Supermap<K, V, I>;

    using RamType = supermap::BST<K, I, I>;
    using IndexStorageListBase = typename Smap::IndexStorageListBase;
    using IndexStorageBase = typename Smap::IndexStorageBase;
    using DiskStorage = typename Smap::DiskStorage;
    using RegisterInfo = typename Smap::RegisterInfo;
    using Register = typename Smap::Register;
    using DefaultBinaryCollapsingList = BinaryCollapsingSortedStoragesList<KI, I, RegisterInfo, K>;

    static std::shared_ptr<KeyValueStorage<K, V, B>> build(const BuildParameters &params) {
        std::shared_ptr<supermap::io::FileManager> fileManager
            = std::make_shared<supermap::io::EncapsulatedFileManager>(
                std::make_shared<supermap::io::TemporaryFolder>(params.folderName, true),
                std::make_unique<supermap::io::DiskFileManager>()
            );

        std::function<std::unique_ptr<IndexStorageBase>(IndexStorageBase &&)>
            indexSupplier = [](IndexStorageBase &&sortedStorage) {
            return std::make_unique<IndexStorageBase>(std::move(sortedStorage));
        };

        std::function<std::unique_ptr<IndexStorageListBase>()> indexListSupplier = [maxRamLoad = params.batchSize]() {
            return std::make_unique<DefaultBinaryCollapsingList>(
                maxRamLoad,
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

        return std::make_shared<Smap>(
            std::make_unique<RamType>(),
            std::make_unique<DiskStorage>(
                "storage-not-sorted",
                "storage-sorted",
                fileManager,
                innerRegisterSupplier
            ),
            [](IndexStorageBase &&sortedStorage) {
                return std::make_unique<IndexStorageBase>(std::move(sortedStorage));
            },
            indexListSupplier,
            []() {
                return std::make_unique<MockFilter<KI, K>>
                    ();
            },
            params.batchSize,
            params.maxNotSortedPart
        );
    }
};

} // supermap
