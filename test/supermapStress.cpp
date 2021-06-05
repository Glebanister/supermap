#include "doctest.h"

#include <random>

#include "core/Supermap.hpp"
#include "core/BST.hpp"
#include "primitive/ByteArray.hpp"
#include "primitive/Key.hpp"
#include "io/EncapsulatedFileManager.hpp"
#include "io/TemporaryFolder.hpp"
#include "io/DiskFileManager.hpp"

extern std::uint32_t timeSeed();

template <
    std::size_t KeyLen,
    std::size_t ValueLen,
    std::size_t MaxRamLoad,
    std::size_t MaxNotSortedSize,
    std::size_t KeyIndexBatchSize,
    char AlphabetBegin,
    char AlphabetEnd
>
void stressTestSupermap(std::size_t iterations, std::size_t seed) {
    using namespace supermap;

    using MySupermap = Supermap<
        Key<KeyLen>,
        ByteArray<ValueLen>,
        std::size_t,
        MaxRamLoad,
        VoidRegister<KeyValue<Key<KeyLen>, std::size_t>>
    >;
    using RamType = BST<typename MySupermap::KeyType,
                        typename MySupermap::IndexType,
                        typename MySupermap::IndexType>;
    using SupermapKvs = KeyValueStorage<typename MySupermap::KeyType,
                                        typename MySupermap::ValueType,
                                        typename MySupermap::BoundsType>;
    using IndexList = typename MySupermap::DefaultIndexList;

    std::shared_ptr<io::FileManager> fileManager
        = std::make_shared<io::EncapsulatedFileManager>(
            std::make_shared<io::TemporaryFolder>("test-folder", true),
            std::make_unique<io::DiskFileManager>()
        );

    std::shared_ptr<SupermapKvs> kvs = std::make_shared<MySupermap>(
        std::make_unique<RamType>(),
        std::make_unique<typename MySupermap::DiskStorage>(
            "storage-not-sorted",
            "storage-sorted",
            fileManager
        ),
        [](typename MySupermap::IndexType notSortedSize, typename MySupermap::IndexType) {
            return notSortedSize >= MaxNotSortedSize;
        },
        []() {
            return std::make_unique<IndexList>(
                KeyIndexBatchSize
            );
        },
        KeyIndexBatchSize
    );

    std::map<Key<KeyLen>, ByteArray<ValueLen>> expectedMap;
    std::mt19937 rand(seed);
    auto randValue = [&]() {
        std::string v;
        for (std::size_t i = 0; i < ValueLen; ++i) {
            v += static_cast<char>(rand() % (AlphabetEnd - AlphabetBegin + 1) + AlphabetBegin);
        }
        return ByteArray<ValueLen>::fromString(v);
    };
    auto randKey = [&]() {
        std::string k;
        for (std::size_t i = 0; i < KeyLen; ++i) {
            k += static_cast<char>(rand() % (AlphabetEnd - AlphabetBegin + 1) + AlphabetBegin);
        }
        return Key<KeyLen>::fromString(k);
    };

    for (std::size_t iter = 0; iter < iterations; ++iter) {
        switch (rand() % 3) {
            case 0: {
                auto key = randKey();
                auto value = randValue();
                kvs->add(key, ByteArray<ValueLen>(value));
                expectedMap[key] = value;
            }
                break;
            case 1: {
                auto key = randKey();
                bool expectedContains = expectedMap.find(key) != expectedMap.end();
                CHECK_EQ(expectedContains, kvs->containsKey(key));
                if (expectedContains) {
                    CHECK_EQ(expectedMap[key], kvs->getValue(key));
                }
            }
                break;
            case 2: {
                auto key = randKey();
                CHECK_EQ((expectedMap.find(key) != expectedMap.end()), kvs->containsKey(key));
            }
                break;
            default: break;
        }
    }
}

TEST_SUITE("Supermap Stress") {

TEST_CASE("Supermap Stress 1") {
    stressTestSupermap<
        1,   // Key
        1,   // Value
        1,   // RamLoad
        1,   // MaxNotSortedSize,
        1,   // KeyIndexBatchSize,
        '0', // AlphabetBegin
        '1'  // AlphabetEnd
    >(2000, timeSeed());
}

TEST_CASE("Supermap Stress 2") {
    stressTestSupermap<
        2,   // Key
        2,   // Value
        4,   // RamLoad
        5,   // MaxNotSortedSize,
        2,   // KeyIndexBatchSize,
        '0', // AlphabetBegin
        '2'  // AlphabetEnd
    >(2000, timeSeed());
}

TEST_CASE("Supermap Stress 3") {
    stressTestSupermap<
        3,   // Key
        6,   // Value
        5,   // RamLoad
        6,   // MaxNotSortedSize,
        4,   // KeyIndexBatchSize,
        '0', // AlphabetBegin
        '1'  // AlphabetEnd
    >(2000, timeSeed());
}

TEST_CASE("Supermap Stress 4") {
    stressTestSupermap<
        3,   // Key
        6,   // Value
        10,  // RamLoad
        40,  // MaxNotSortedSize,
        10,  // KeyIndexBatchSize,
        '0', // AlphabetBegin
        '3'  // AlphabetEnd
    >(2000, timeSeed());
}

TEST_CASE("Supermap Stress 5") {
    stressTestSupermap<
        10,   // Key
        30,   // Value
        942,  // RamLoad
        4003, // MaxNotSortedSize,
        52,   // KeyIndexBatchSize,
        'a',  // AlphabetBegin
        'z'   // AlphabetEnd
    >(2000, timeSeed());
}

TEST_CASE("Supermap Stress 6") {
    stressTestSupermap<
        3,    // Key
        30,   // Value
        942,  // RamLoad
        2013, // MaxNotSortedSize,
        52,   // KeyIndexBatchSize,
        'a',  // AlphabetBegin
        'z'   // AlphabetEnd
    >(10000, timeSeed());
}

TEST_CASE("Supermap Stress 7") {
    stressTestSupermap<
        5,    // Key
        30,   // Value
        123,  // RamLoad
        912,  // MaxNotSortedSize,
        11,   // KeyIndexBatchSize,
        '0',  // AlphabetBegin
        '1'   // AlphabetEnd
    >(10000, timeSeed());
}

TEST_CASE("Supermap Stress 8") {
    stressTestSupermap<
        171,  // Key
        1031, // Value
        212,  // RamLoad
        72,   // MaxNotSortedSize,
        13,   // KeyIndexBatchSize,
        'a',  // AlphabetBegin
        'z'   // AlphabetEnd
    >(10000, timeSeed());
}

}
