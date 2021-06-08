#include "doctest.h"

#include <random>

#include "core/Supermap.hpp"
#include "core/BST.hpp"
#include "primitive/ByteArray.hpp"
#include "primitive/Key.hpp"
#include "io/EncapsulatedFileManager.hpp"
#include "io/TemporaryFolder.hpp"
#include "io/DiskFileManager.hpp"
#include "core/MockFilter.hpp"
#include "core/FilteringRegister.hpp"
#include "core/FilteredStorage.hpp"
#include "builder/DefaultSupermap.hpp"

extern std::uint32_t timeSeed();

template <
    std::size_t KeyLen,
    std::size_t ValueLen
>
void stressTestSupermap(std::size_t iterations,
                        std::size_t seed,
                        std::size_t batchSize,
                        double part,
                        char alphabetBegin,
                        char alphabetEnd,
                        bool check) {
    using namespace supermap;

    using K = Key<KeyLen>;
    using V = ByteArray<ValueLen>;
    using I = std::size_t;

    using SupermapBuilder = DefaultSupermap<K, V, I>;

    auto kvs = SupermapBuilder::build(
        std::make_unique<BST<K, I, I>>(),
        typename SupermapBuilder::BuildParameters{
            batchSize,
            part,
            "supermap"
        }
    );

    auto expectedKvs = check
        ? std::make_unique<BST<K, V, I>>()
        : SupermapBuilder::build(
            std::make_unique<BST<K, I, I>>(),
            typename SupermapBuilder::BuildParameters{
                batchSize,
                part,
                "supermap-other"
            }
        );

    std::mt19937 rand(seed);
    auto randValue = [&]() {
        std::string v;
        v.reserve(ValueLen);
        for (std::size_t i = 0; i < ValueLen; ++i) {
            v += static_cast<char>(rand() % (alphabetEnd - alphabetBegin + 1) + alphabetBegin);
        }
        return ByteArray<ValueLen>::fromString(v);
    };
    auto randKey = [&]() {
        std::string k;
        k.reserve(KeyLen);
        for (std::size_t i = 0; i < KeyLen; ++i) {
            k += static_cast<char>(rand() % (alphabetEnd - alphabetBegin + 1) + alphabetBegin);
        }
        return Key<KeyLen>::fromString(k);
    };

    for (std::size_t iter = 0; iter < iterations; ++iter) {
        switch (rand() % 3) {
            case 0: {
                auto key = randKey();
                auto value = randValue();
                kvs->add(key, ByteArray<ValueLen>(value));
                expectedKvs->add(key, ByteArray<ValueLen>(value));
            }
                break;
            case 1: {
                auto key = randKey();
                bool expectedContains = expectedKvs->contains(key);
                CHECK_EQ(expectedContains, kvs->contains(key));
                if (expectedContains) {
                    CHECK_EQ(expectedKvs->getValue(key), kvs->getValue(key));
                }
            }
                break;
            case 2: {
                auto key = randKey();
                CHECK_EQ(expectedKvs->contains(key), kvs->contains(key));
            }
                break;
            default: break;
        }
    }
}

TEST_SUITE("Supermap Stress") {

TEST_CASE("Supermap Stress 1") {
    stressTestSupermap<1, 1>(10000, timeSeed(), 1, 0.8, '0', '0', true);
}

TEST_CASE("Supermap Stress 2") {
    stressTestSupermap<1, 1>(10000, timeSeed(), 1, 0.5, '0', '1', true);
}

TEST_CASE("Supermap Stress 3") {
    stressTestSupermap<3, 4>(10000, timeSeed(), 7, 0.12, '0', '3', true);
}

TEST_CASE("Supermap Stress 4") {
    stressTestSupermap<128, 4096>(1000, timeSeed(), 50, 0.3, 'a', 'z', true);
}

TEST_CASE("Supermap Stress 4") {
    stressTestSupermap<2, 2>(20000, timeSeed(), 507, 0.02, 'a', 'z', true);
}

}

//TEST_SUITE("Supermap Stress Profiling") {
//
////TEST_CASE("Supermap Stress Profiling") {
////    stressTestSupermap<16, 64>(100000, timeSeed(), 5000, 1.0, 'a', 'z', false);
////}
//
//}
