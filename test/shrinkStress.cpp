#include "doctest.h"

#include <random>

#include "primitive/Key.hpp"
#include "primitive/ByteArray.hpp"
#include "primitive/KeyValue.hpp"
#include "io/DiskFileManager.hpp"
#include "core/KeyValueShrinkableStorage.hpp"

extern std::uint32_t timeSeed();

namespace supermap {
template <typename Key, typename Value>
bool operator==(const KeyValue<Key, Value> &a, const KeyValue<Key, Value> &b) {
    return a.key == b.key && a.value == b.value;
}
}


template <
    std::size_t KeyLen,
    std::size_t ValueLen,
    typename IndexT,
    std::size_t BatchSize,
    std::size_t Iterations,
    std::size_t IterationLen,
    char AlphabetBegin = '0',
    char AlphabetEnd = '9'
>
void shrinkStress(std::uint32_t seed) {
    using namespace supermap;
    using K = Key<KeyLen>;
    using V = ByteArray<ValueLen>;
    using KV = KeyValue<Key<KeyLen>, ByteArray<ValueLen>>;
    using KI = KeyValue<K, IndexT>;

    const std::string NEW_INDEX_FILENAME = "new-index";

    std::shared_ptr<io::FileManager> manager = std::make_shared<io::DiskFileManager>();
    KeyValueShrinkableStorage<K, V, IndexT> storage(
        "keys-not-sorted",
        "keys-sorted",
        manager
    );

    std::mt19937 rand(seed);

    auto randValue = [&]() {
        std::string v;
        for (std::size_t i = 0; i < ValueLen; ++i) {
            v += static_cast<char>(rand() % (AlphabetEnd - AlphabetBegin + 1) + AlphabetBegin);
        }
        return V::fromString(v);
    };
    auto randKey = [&]() {
        std::string k;
        for (std::size_t i = 0; i < KeyLen; ++i) {
            k += static_cast<char>(rand() % (AlphabetEnd - AlphabetBegin + 1) + AlphabetBegin);
        }
        return K::fromString(k);
    };

    std::vector<KV> expectedNotSortedStorage;
    std::vector<KV> expectedSortedStorage;

    auto appendToExpected = [&](const KV &kv) {
        expectedNotSortedStorage.push_back(kv);
    };

    auto shrinkExpected = [&]() {
        std::vector<KV> newSorted;
        for (auto sortedIt = expectedSortedStorage.begin(); sortedIt < expectedSortedStorage.end(); ++sortedIt) {
            newSorted.push_back(std::move(*sortedIt));
        }
        for (auto notSortedIt = expectedNotSortedStorage.begin(); notSortedIt < expectedNotSortedStorage.end();
             ++notSortedIt) {
            newSorted.push_back(std::move(*notSortedIt));
        }
        auto newSortedEnd = SortedSingleFileIndexedStorage<KV, IndexT>::sortedEndIterator(
            newSorted.begin(),
            newSorted.end(),
            [](const KV &a, const KV &b) { return a.key < b.key; },
            [](const KV &a, const KV &b) { return a.key == b.key; }
        );
        expectedNotSortedStorage = {};
        expectedSortedStorage = {};
        for (auto it = newSorted.begin(); it < newSortedEnd; ++it) {
            expectedSortedStorage.push_back(std::move(*it));
        }
    };

    auto checkNotSorted = [&]() {
        std::vector<KV> actualNotSorted = storage.getNotSortedEntries().collect();
        CHECK_EQ(actualNotSorted.size(), expectedNotSortedStorage.size());
        CHECK_EQ(actualNotSorted, expectedNotSortedStorage);
    };

    auto checkSorted = [&]() {
        std::vector<KV> actualSorted = storage.getSortedEntries().collect();
        auto actualSortedSize = actualSorted.size();
        auto expectedSortedSize = expectedSortedStorage.size();
        CHECK_EQ(actualSortedSize, expectedSortedSize);
        CHECK_EQ(actualSorted, expectedSortedStorage);
    };

    auto checkStorage = [&]() {
        CHECK_EQ(storage.getItemsCount(), expectedSortedStorage.size() + expectedNotSortedStorage.size());
        CHECK_EQ(storage.getNotSortedItemsCount(), expectedNotSortedStorage.size());
        CHECK_EQ(storage.getSortedItemsCount(), expectedSortedStorage.size());
        for (IndexT i = 0; i < expectedSortedStorage.size(); ++i) {
            CHECK_EQ(storage.get(i), expectedSortedStorage[i]);
        }
        for (IndexT i = 0; i < expectedNotSortedStorage.size(); ++i) {
            CHECK_EQ(storage.get(i + expectedSortedStorage.size()), expectedNotSortedStorage[i]);
        }
    };

    auto checkNewIndex = [&](const SortedSingleFileIndexedStorage<KI, IndexT> &newIndex) {
        auto newIndexSize = newIndex.getItemsCount();
        auto sortedSize = expectedSortedStorage.size();
        CHECK_EQ(newIndexSize, sortedSize);
        for (IndexT i = 0; i < expectedSortedStorage.size(); ++i) {
            IndexT curInd = newIndex.get(i).value;
            K curKey = newIndex.get(i).key;
            K expectedKey = expectedSortedStorage[i].key;
            CHECK_EQ(curInd, i);
            CHECK_EQ(curKey, expectedKey);
        }
    };

    for (std::size_t iter = 0; iter < Iterations; ++iter) {
        for (std::size_t step = 0; step < IterationLen; ++step) {
            KV randKV(randKey(), randValue());
            appendToExpected(randKV);
            storage.append(randKV);
        }
        shrinkExpected();
        checkNewIndex(storage.shrink(BatchSize, NEW_INDEX_FILENAME));
        checkStorage();
        checkNotSorted();
        checkSorted();
    }
}

TEST_SUITE("Shrink Stress") {

TEST_CASE ("KeyValueShrinkableStorage shrink stress 1") {
    shrinkStress<
        1, // KEY,
        1, // VALUE
        std::uint32_t, // INDEX
        1, // BATCH
        20, // ITERATIONS
        20, // ITERATION LEN
        '0', // ALPHABET BEGIN
        '1' // ALPHABET END
    >(timeSeed());
}

TEST_CASE ("KeyValueShrinkableStorage shrink stress 2") {
    shrinkStress<
        1, // KEY,
        2, // VALUE
        std::uint32_t, // INDEX
        3, // BATCH
        30, // ITERATIONS
        30, // ITERATION LEN
        '0', // ALPHABET BEGIN
        '3' // ALPHABET END
    >(timeSeed());
}

TEST_CASE ("KeyValueShrinkableStorage shrink stress 3") {
    shrinkStress<
        2, // KEY,
        3, // VALUE
        std::uint64_t, // INDEX
        10, // BATCH
        300, // ITERATIONS
        300, // ITERATION LEN
        '0', // ALPHABET BEGIN
        '3' // ALPHABET END
    >(timeSeed());
}

TEST_CASE ("KeyValueShrinkableStorage shrink stress 4") {
    shrinkStress<
        4, // KEY,
        10, // VALUE
        std::uint64_t, // INDEX
        3, // BATCH
        20, // ITERATIONS
        20, // ITERATION LEN
        '\0', // ALPHABET BEGIN
        '\255' // ALPHABET END
    >(timeSeed());
}
}
