#include "doctest.h"

#include <filesystem>
#include <vector>
#include <chrono>
#include <functional>

#include "io/InputStream.hpp"
#include "io/InputIterator.hpp"
#include "io/ShallowSerializer.hpp"
#include "io/OutputStream.hpp"
#include "io/OutputIterator.hpp"
#include "io/RamFileManager.hpp"
#include "io/DiskFileManager.hpp"
#include "io/EncapsulatedFileManager.hpp"

#include "exception/IllegalArgumentException.hpp"

#include "primitive/Key.hpp"
#include "primitive/ByteArray.hpp"
#include "core/SingleFileIndexedStorage.hpp"
#include "core/KeyValueShrinkableStorage.hpp"
#include "core/BinaryCollapsingSortedStoragesList.hpp"
#include "core/Supermap.hpp"
#include "core/BST.hpp"
#include "core/FilteredStorage.hpp"
#include "core/MockFilter.hpp"
#include "builder/DefaultSupermap.hpp"

using CharKV = supermap::KeyValue<char, char>;

namespace supermap {
template <typename Key, typename Value>
bool operator==(const KeyValue<Key, Value> &a, const KeyValue<Key, Value> &b) {
    return a.key == b.key && a.value == b.value;
}
}

struct TempFile {
    const std::string filename = ".supermap-test-file";

    explicit TempFile(const std::string &data = "") {
        std::ofstream file(filename);
        file << data;
        file.close();
    }

    ~TempFile() {
        std::filesystem::remove(filename);
    }
};

struct TempDir {
    const std::string folderName = "test-folder";

    TempDir() {
        std::filesystem::create_directory(folderName);
    }

    ~TempDir() {
        std::filesystem::remove(folderName);
    }
};

struct MockStruct {
    MockStruct() = default;

    int a = 12;
    char b = 'x';
    std::array<int, 3> c{12, 3, 2};

    bool operator==(const MockStruct &other) const {
        return a == other.a && b == other.b && c == other.c;
    }
};

namespace supermap::io {

template <>
struct SerializeHelper<MockStruct> : ShallowSerializer<MockStruct> {};

template <>
struct DeserializeHelper<MockStruct> : ShallowDeserializer<MockStruct> {};

template <>
struct FixedDeserializedSizeRegister<MockStruct> : ShallowDeserializedSize<MockStruct> {};

} // supermap::io

TEST_SUITE("Overall Tests") {

TEST_CASE ("StringInputStream") {
    supermap::io::StringInputStream iss("1 2 3", 0);
    int x, y, z;
    iss.get() >> x >> y >> z;
    CHECK_EQ(x, 1);
    CHECK_EQ(y, 2);
    CHECK_EQ(z, 3);
}

TEST_CASE ("FileInputStream") {
    TempFile file("1 2 3");
    supermap::io::FileInputStream ifs(file.filename, 0);
    int x, y, z;
    ifs.get() >> x >> y >> z;
    CHECK_EQ(x, 1);
    CHECK_EQ(y, 2);
    CHECK_EQ(z, 3);
}

TEST_CASE ("StringOutputStream") {
    std::string buffer;
    supermap::io::StringOutputStream os(buffer, true);
    os.get() << 1 << ' ' << 2 << ' ' << 3;
    os.flush();
    CHECK_EQ(buffer, "1 2 3");
}

TEST_CASE ("FileOutputStream") {
    TempFile file("1 2 ");
    supermap::io::FileOutputStream ofs(file.filename, true);
    ofs.get() << 3 << ' ' << 4;
    ofs.flush();
    std::ifstream fileIfs(file.filename);
    int a, b, c, d;
    fileIfs >> a >> b >> c >> d;
    CHECK_EQ(a, 1);
    CHECK_EQ(b, 2);
    CHECK_EQ(c, 3);
    CHECK_EQ(d, 4);
}

TEST_CASE ("FileInputStream ints with offset") {
    TempFile file("1 2 3");
    supermap::io::FileInputStream ifs(file.filename, 2);
    int y, z;
    ifs.get() >> y >> z;
    CHECK_EQ(y, 2);
    CHECK_EQ(z, 3);
}

TEST_CASE ("InputIterator MockStruct") {
    std::stringstream ss;
    std::vector<MockStruct> before = {
        {12343, 'a', {312, 0, 1122}},
        {32, 'b', {13, 3, 9441}},
        {18, 'c', {4010, 31, 321123}},
    };
    for (const auto &s : before) {
        supermap::io::serialize(s, ss);
    }
    auto intIterator = supermap::io::InputIterator<MockStruct, std::uint32_t>::fromString(ss.str());
    std::vector<MockStruct> parsed;
    while (intIterator.hasNext()) {
        parsed.push_back(intIterator.next());
    }
    CHECK(before == parsed);
}

TEST_CASE ("OutputIterator MockStruct") {
    std::string buffer;
    std::vector<MockStruct> mocks = {{1, 'x', {4, 15, 6}},
                                     {2, 'y', {91, 905, 6}},
                                     {3, 'z', {41012, 5, 236}}};
    auto writeIterator = supermap::io::OutputIterator<MockStruct>::toString(buffer, false);
    writeIterator.writeAll(mocks.begin(), mocks.end(), [](auto &&x) { return std::forward<decltype(x)>(x); });
    writeIterator.flush();
    std::vector<MockStruct> parsedMocks;
    auto readIterator = supermap::io::InputIterator<MockStruct, std::uint32_t>::fromString(buffer, 0);
    while (readIterator.hasNext()) {
        parsedMocks.push_back(readIterator.next());
    }
    CHECK_EQ(parsedMocks, mocks);
}

TEST_CASE ("RamFileManager") {
    supermap::io::RamFileManager manager;
    std::filesystem::path root = ".";
    std::vector<std::filesystem::path> paths = {
        root / "cats" / "cat1.png",
    };

    auto f1o = manager.getOutputStream(paths[0], false);
    f1o->get() << "someCatData";
    f1o->flush();
    std::string catData;
    manager.getInputStream(paths[0], 0)->get() >> catData;
    CHECK_EQ(catData, "someCatData");
    CHECK_THROWS_AS(manager.getInputStream(root / "non" / "existent" / "path", 0), const supermap::FileException &);
    manager.remove(paths[0]);
    CHECK_THROWS_AS(manager.getInputStream(paths[0], 0), const supermap::FileException &);
}

TEST_CASE ("Key") {
    auto key6 = supermap::Key<6>::fromString("123456");
    CHECK_EQ(key6.toString(), "123456");
}

TEST_CASE ("ByteArray") {
    auto key6 = supermap::ByteArray<6>::fromString("123456");
    CHECK_EQ(key6.toString(), "123456");
}

TEST_CASE ("SingleFileIndexedStorage KeyIndex") {
    using namespace supermap;
    using namespace io;

    using KeyIndex = KeyValue<Key<2>, std::uint32_t>;

    auto manager = std::make_shared<RamFileManager>();
    const std::string filename = "indexed_data.txt";

    SingleFileIndexedStorage<KeyIndex, std::uint32_t, void> indexedData(
        filename,
        manager,
        []() { return std::make_unique<VoidRegister<KeyIndex>>(); }
    );

    std::vector<KeyIndex> keys{
        {Key<2>::fromString("ab"), 0},
        {Key<2>::fromString("cd"), 1},
        {Key<2>::fromString("ef"), 2},
        {Key<2>::fromString("gh"), 3},
    };

    for (std::size_t i = 0; i < keys.size(); ++i) {
        indexedData.appendCopy(keys[i]);
        CHECK_EQ(i, indexedData.getLastElementIndex());
    }
    std::vector<KeyIndex> parsedData;
    auto keyIndexParser = indexedData.getDataIterator();
    while (keyIndexParser.hasNext()) {
        parsedData.push_back(keyIndexParser.next());
    }
    CHECK_EQ(keys, parsedData);
}

TEST_CASE ("KeyValueShrinkableStorage notSortedStorage") {
    using namespace supermap;
    using namespace io;

    auto manager = std::make_shared<RamFileManager>();
    KeyValueShrinkableStorage<
        Key<2>,
        ByteArray<4>,
        std::uint32_t,
        void
    > indexedData(
        "keys-not-sorted",
        "keys-sorted",
        manager,
        []() { return std::make_unique<VoidRegister<KeyValue<Key<2>, std::uint32_t>>>(); }
    );

    std::vector<KeyValue<Key<2>, ByteArray<4>>> keyValues{
        {Key<2>::fromString("ab"), ByteArray<4>::fromString("1234")},
        {Key<2>::fromString("cd"), ByteArray<4>::fromString("1831")},
        {Key<2>::fromString("ef"), ByteArray<4>::fromString("4923")},
        {Key<2>::fromString("gh"), ByteArray<4>::fromString("3482")},
    };

    for (std::size_t i = 0; i < keyValues.size(); ++i) {
        indexedData.appendCopy(keyValues[i]);
        CHECK_EQ(i, indexedData.getLastElementIndex());
    }
    std::vector<KeyValue<Key<2>, ByteArray<4>>> parsedData;
    auto keyIndexParser = indexedData.getNotSortedEntries();
    while (keyIndexParser.hasNext()) {
        parsedData.push_back(keyIndexParser.next());
    }
    CHECK_EQ(keyValues, parsedData);
}

TEST_CASE("collectWith") {
    using namespace supermap;
    using namespace io;

    using KeyIndex = KeyValue<Key<2>, std::uint32_t>;
    using KeyVal = KeyValue<Key<2>, ByteArray<4>>;

    auto manager = std::make_shared<RamFileManager>();
    const std::string filename = "indexed_data.txt";
    SingleFileIndexedStorage<KeyVal, char, void> storage(
        filename,
        manager,
        []() { return std::make_unique<VoidRegister<KeyVal>>(); }
    );

    std::vector<KeyValue<Key<2>, ByteArray<4>>> keyValues{
        {Key<2>::fromString("ab"), ByteArray<4>::fromString("xxxx")},
        {Key<2>::fromString("cd"), ByteArray<4>::fromString("yyyy")},
        {Key<2>::fromString("ef"), ByteArray<4>::fromString("zzzz")},
        {Key<2>::fromString("gh"), ByteArray<4>::fromString("tttt")},
    };

    std::vector<Key<2>> initialKeys;
    std::transform(keyValues.begin(),
                   keyValues.end(),
                   std::back_inserter(initialKeys),
                   [](const KeyValue<Key<2>, ByteArray<4>> &kv) { return kv.key; });

    for (std::size_t i = 0; i < keyValues.size(); ++i) {
        storage.appendCopy({keyValues[i].key, ByteArray(keyValues[i].value)});
        CHECK_EQ(i, storage.getLastElementIndex());
        CHECK_EQ(i, storage.getItemsCount() - 1);
    }

    auto keyIndexParser = storage.getCustomDataIterator<StorageValueIgnorer<Key<2>, ByteArray<4>>>();
    std::vector<KeyIndex> parsedKeys = keyIndexParser.collectWith(
        [](StorageValueIgnorer<Key<2>, ByteArray<4>> &&svi, std::size_t index) {
            return KeyIndex(std::move(svi.key), index);
        });
    CHECK_EQ(initialKeys.size(), parsedKeys.size());
    for (std::size_t i = 0; i < initialKeys.size(); ++i) {
        CHECK_EQ(initialKeys[i], parsedKeys[i].key);
        CHECK_EQ(i, parsedKeys[i].value);
    }
}

TEST_CASE("KeyValueShrinkableStorage shrink smoke test") {
    using namespace supermap;

    std::shared_ptr<io::FileManager> manager = std::make_shared<io::DiskFileManager>();

    KeyValueShrinkableStorage<
        Key<2>,
        ByteArray<4>,
        std::uint32_t,
        void
    > storage(
        "not-sorted",
        "sorted",
        manager,
        []() { return std::make_unique<VoidRegister<KeyValue<Key<2>, std::uint32_t>>>(); }
    );
    CHECK_NOTHROW(auto newIndex = storage.shrink(10, "new-index"));
}

TEST_CASE("Enum serialization") {
    using namespace supermap;

    using KeyIndex = KeyValue<char, std::uint32_t>;

    std::string buffer;
    io::StringOutputStream out(buffer, false);
    io::serialize(KeyIndex('a', 1), out.get());
    io::serialize(KeyIndex('b', 2), out.get());
    out.flush();
    io::StringInputStream in(buffer, 0);
    CHECK_EQ(io::deserialize<KeyIndex>(in.get()), KeyIndex{'a', 1});
    CHECK_EQ(io::deserialize<KeyIndex>(in.get()), KeyIndex{'b', 2});
}

TEST_CASE("Enum collect ram") {
    using namespace supermap;

    using KeyIndex = KeyValue<char, std::uint32_t>;

    std::string buffer;
    io::StringOutputStream out(buffer, false);
    io::serialize(KeyIndex('a', 1), out.get());
    io::serialize(KeyIndex('b', 2), out.get());
    out.flush();
    io::InputIterator<KeyIndex, std::uint32_t> inpIt(std::make_unique<io::StringInputStream>(buffer, 0));
    std::vector<KeyIndex> enums = inpIt.collect();
    CHECK_EQ(enums.size(), 2);
    CHECK_EQ(enums[0], KeyIndex{'a', 1});
    CHECK_EQ(enums[1], KeyIndex{'b', 2});
}

TEST_CASE("Enum collect disk") {
    using namespace supermap;

    using KeyIndex = KeyValue<char, std::uint32_t>;

    TempFile file("test-Enum-collect-disk");

    std::string buffer;
    io::FileOutputStream out(file.filename, false);
    io::serialize(KeyIndex('a', 1), out.get());
    io::serialize(KeyIndex('b', 2), out.get());
    out.flush();
    io::InputIterator<KeyIndex, std::uint32_t> inpIt(std::make_unique<io::FileInputStream>(file.filename, 0));
    std::vector<KeyIndex> enums = inpIt.collect();
    CHECK_EQ(enums.size(), 2);
    CHECK_EQ(enums[0], KeyIndex{'a', 1});
    CHECK_EQ(enums[1], KeyIndex{'b', 2});
}

TEST_CASE("Serialize KeyValue Disk") {
    using namespace supermap;

    std::shared_ptr<io::FileManager> manager = std::make_shared<io::DiskFileManager>();
    TempFile file("test-Serialize-KeyValue");
    auto outIt = manager->getOutputIterator<KeyValue<Key<2>, ByteArray<4>>>(file.filename, false);
    auto kv1 = KeyValue{Key<2>::fromString("ab"), ByteArray<4>::fromString("1234")};
    auto kv2 = KeyValue{Key<2>::fromString("cd"), ByteArray<4>::fromString("5678")};
    auto kv3 = KeyValue{Key<2>::fromString("ef"), ByteArray<4>::fromString("1023")};
    outIt.write(kv1);
    outIt.write(kv2);
    outIt.write(kv3);
    outIt.flush();
    auto inIt = manager->getInputIterator<KeyValue<Key<2>, ByteArray<4>>, std::uint32_t>(file.filename, 0);
    auto read = inIt.collectWith([](KeyValue<Key<2>, ByteArray<4>> &&kv, std::uint32_t index) {
        return KeyValue<KeyValue<Key<2>, ByteArray<4>>, std::uint32_t>(std::move(kv),
                                                                       index);
    });
    CHECK_EQ(read.size(), 3);
    CHECK_EQ(read[0].value, 0);
    CHECK_EQ(read[1].value, 1);
    CHECK_EQ(read[2].value, 2);
    CHECK_EQ(read[0].key, kv1);
    CHECK_EQ(read[1].key, kv2);
    CHECK_EQ(read[2].key, kv3);
}

TEST_CASE("KeyValueShrinkableStorage shrink simple") {
    using namespace supermap;

    using KeyIndex = KeyValue<Key<2>, std::uint32_t>;

    std::shared_ptr<io::FileManager> manager = std::make_shared<io::DiskFileManager>();
    KeyValueShrinkableStorage<
        Key<2>,
        ByteArray<4>,
        std::uint32_t,
        void
    > storage(
        "not-sorted",
        "sorted",
        manager,
        []() { return std::make_unique<VoidRegister<KeyValue<Key<2>, std::uint32_t>>>(); }
    );
    storage.appendCopy({Key<2>::fromString("bb"), ByteArray<4>::fromString("1111")});
    storage.appendCopy({Key<2>::fromString("aa"), ByteArray<4>::fromString("2222")});
    storage.appendCopy({Key<2>::fromString("cc"), ByteArray<4>::fromString("3333")});
    CHECK_EQ(storage.getItemsCount(), 3);
    {
        std::vector<Key<2>> notSortedKeys =
            storage.getNotSortedKeys().collectWith([](StorageValueIgnorer<Key<2>, ByteArray<4>> &&kvi, std::uint32_t) {
                return kvi.key;
            });
        CHECK_EQ(notSortedKeys,
                 std::vector{
                     Key<2>::fromString("bb"),
                     Key<2>::fromString("aa"),
                     Key<2>::fromString("cc"),
                 });
    }
    {
        std::vector<Key<2>> sortedKeys =
            storage.getSortedKeys().collectWith([](StorageValueIgnorer<Key<2>, ByteArray<4>> &&svi, std::uint32_t) {
                return svi.key;
            });
        CHECK_EQ(sortedKeys, std::vector<Key<2>>{});
    }
    SortedSingleFileIndexedStorage<KeyIndex, std::uint32_t, void, Key<2>> newIndex = storage.shrink(
        1,
        "new-index"
    );
    CHECK_EQ(newIndex.getItemsCount(), 3);
    CHECK_EQ(newIndex.getStorageFilePath(), "new-index");
    CHECK_EQ(newIndex.getFileManager(), manager);
}

TEST_CASE("KeyValueShrinkableStorage shrink advanced") {
    using namespace supermap;

    std::shared_ptr<io::FileManager> manager = std::make_shared<io::DiskFileManager>();
    KeyValueShrinkableStorage<
        Key<1>,
        ByteArray<1>,
        std::uint32_t,
        void
    > storage(
        "keys-not-sorted",
        "keys-sorted",
        manager,
        []() { return std::make_unique<VoidRegister<KeyValue<Key<1>, std::uint32_t>>>(); }
    );

    auto append = [&](const std::string &k, const std::string &v) {
        storage.appendCopy({Key<1>::fromString(k), ByteArray<1>::fromString(v)});
    };

    auto keyIndex = [&](const std::string &k, int i) {
        return KeyValue<Key<1>, std::uint32_t>(Key<1>::fromString(k), i);
    };

    auto keyVal = [&](const std::string &k, const std::string &v) {
        return KeyValue<Key<1>, ByteArray<1>>{Key<1>::fromString(k), ByteArray<1>::fromString(v)};
    };

    append("5", "a");
    append("2", "h");
    append("3", "x");
    append("2", "t");

    {
        SortedSingleFileIndexedStorage<
            KeyValue<Key<1>, std::uint32_t>,
            std::uint32_t,
            void,
            Key<1>
        > newIndex = storage.shrink(
            1,
            "keys-new-index"
        );

        CHECK_EQ(newIndex.getItemsCount(), 3);
        CHECK_EQ(storage.getItemsCount(), 3);
        CHECK_EQ(newIndex.get(0).equals({Key<1>::fromString("2"), 0}), true);
        CHECK_EQ(newIndex.get(1).equals({Key<1>::fromString("3"), 1}), true);
        CHECK_EQ(newIndex.get(2).equals({Key<1>::fromString("5"), 2}), true);
        CHECK_EQ(storage.get(0), keyVal("2", "t"));
        CHECK_EQ(storage.get(1), keyVal("3", "x"));
        CHECK_EQ(storage.get(2), keyVal("5", "a"));
    }

    append("5", "x");
    append("6", "b");
    append("2", "b");
    append("2", "c");
    append("2", "c");
    append("2", "y");
    append("2", "u");
    append("3", "x");
    append("3", "t");
    append("8", "b");
    append("3", "p");

    {
        SortedSingleFileIndexedStorage<
            KeyValue<Key<1>, std::uint32_t>,
            std::uint32_t,
            void,
            Key<1>
        > newIndex = storage.shrink(
            2,
            "new-index"
        );

        CHECK_EQ(newIndex.getItemsCount(), 5);
        CHECK_EQ(storage.getItemsCount(), 5);
        CHECK_EQ(storage.get(0), keyVal("2", "u"));
        CHECK_EQ(storage.get(1), keyVal("3", "p"));
        CHECK_EQ(storage.get(2), keyVal("5", "x"));
        CHECK_EQ(storage.get(3), keyVal("6", "b"));
        CHECK_EQ(storage.get(4), keyVal("8", "b"));
        CHECK_EQ(newIndex.get(0), keyIndex("2", 0));
        CHECK_EQ(newIndex.get(1), keyIndex("3", 1));
        CHECK_EQ(newIndex.get(2), keyIndex("5", 2));
        CHECK_EQ(newIndex.get(3), keyIndex("6", 3));
        CHECK_EQ(newIndex.get(4), keyIndex("8", 4));
    }
}

TEST_CASE ("SortEndIterator 1") {
    using namespace supermap;

    std::vector<CharKV> entries = {
        {1, 3},
        {2, 9},
        {1, 4},
        {9, 2},
        {1, 2},
        {3, 3},
        {2, 8},
    };

    entries.erase(
        SortedSingleFileIndexedStorage<CharKV, int, VoidRegister<CharKV>, char>::sortedEndIterator(
            entries.begin(),
            entries.end(),
            [](const CharKV &a, const CharKV &b) { return a.key < b.key; },
            [](const CharKV &a, const CharKV &b) { return a.key == b.key; }),
        entries.end()
    );

    CHECK_EQ(entries, std::vector<CharKV>{
        {1, 2},
        {2, 8},
        {3, 3},
        {9, 2}
    });
}

TEST_CASE ("SortEndIterator 2") {
    using namespace supermap;

    std::vector<CharKV> entries = {
        {2, 5},
        {0, 2},
        {1, 5},
        {0, 4},
        {4, 0},
        {0, 5},
        {4, 5},
        {4, 0},
        {3, 5},
        {1, 2},
        {1, 5},
        {0, 3},
        {2, 3},
        {2, 1},
        {4, 0},
        {0, 5},
        {1, 4},
        {2, 2},
        {0, 0},
        {2, 1},
    };

    entries.erase(
        SortedSingleFileIndexedStorage<CharKV, int, VoidRegister<CharKV>, char>::sortedEndIterator(
            entries.begin(),
            entries.end(),
            [](const CharKV &a, const CharKV &b) { return a.key < b.key; },
            [](const CharKV &a, const CharKV &b) { return a.key == b.key; }),
        entries.end()
    );

    CHECK_EQ(entries, std::vector<CharKV>{
        {0, 0},
        {1, 4},
        {2, 1},
        {3, 5},
        {4, 0},
    });
}

TEST_CASE("SortedSingleFileIndexedStorage find int") {
    using namespace supermap;

    std::shared_ptr<io::FileManager> manager = std::make_shared<io::DiskFileManager>();
    std::vector<int> items = {4, 2, 7, 5, 5, 3, 2, 3, 4};
    SortedSingleFileIndexedStorage<int, int, void, int> storage(
        items.begin(),
        items.end(),
        false,
        "sorted-keys",
        manager,
        [](int a, int b) { return a < b; },
        [](int a, int b) { return a == b; },
        []() { return std::make_unique<VoidRegister<int>>(); }
    );
    auto findElem = [&](int elem) {
        return storage.find(
            elem,
            [](const int &storageElem, const int &myElem) { return storageElem < myElem; },
            [](const int &storageElem, const int &myElem) { return storageElem == myElem; }
        );
    };

    CHECK_EQ(findElem(12), std::nullopt);
    CHECK_EQ(findElem(4), std::optional{4});
    CHECK_EQ(findElem(1), std::nullopt);
    CHECK_EQ(findElem(6), std::nullopt);
    CHECK_EQ(findElem(3), std::optional{3});
}

TEST_CASE("SortedSingleFileIndexedStorage find CharKV") {
    using namespace supermap;

    std::shared_ptr<io::FileManager> manager = std::make_shared<io::DiskFileManager>();

    std::vector<CharKV> items = {
        {4, 0},
        {2, 1},
        {7, 2},
        {5, 3},
        {5, 4},
        {3, 5},
        {2, 6},
        {3, 7},
        {4, 8},
    };

    SortedSingleFileIndexedStorage<CharKV, int, void, char> storage(
        items.begin(),
        items.end(),
        false,
        "sorted-keys",
        manager,
        [](const CharKV &a, const CharKV &b) { return a.key < b.key; },
        [](const CharKV &a, const CharKV &b) { return a.key == b.key; },
        []() { return std::make_unique<VoidRegister<CharKV>>(); }
    );
    auto findElem = [&](char elem) {
        return storage.find(
            elem,
            [](const CharKV &x, const int &e) { return x.key < e; },
            [](const CharKV &x, const int &e) { return x.key == e; }
        );
    };

    CHECK_EQ(findElem(12), std::nullopt);
    CHECK_EQ(findElem(4), std::optional{CharKV{4, 8}});
    CHECK_EQ(findElem(1), std::nullopt);
    CHECK_EQ(findElem(6), std::nullopt);
    CHECK_EQ(findElem(3), std::optional{CharKV{3, 7}});
}

TEST_CASE("BinaryCollapsingSortedStoragesList") {
    using namespace supermap;

    std::shared_ptr<io::FileManager> manager = std::make_shared<io::DiskFileManager>();

    std::vector<CharKV> firstBlockElements = {
        {1, 1},
        {2, 1},
        {3, 0},
    };
    std::vector<CharKV> secondBlockElements = {
        {1, 2},
        {2, 1},
        {4, 2},
    };
    auto oldBlock = std::make_unique<SortedSingleFileIndexedStorage<CharKV, char, void, char>>(
        firstBlockElements.begin(),
        firstBlockElements.end(),
        true,
        "block-1",
        manager,
        [](const CharKV &a, const CharKV &b) { return a.key < b.key; },
        [](const CharKV &a, const CharKV &b) { return a.key == b.key; },
        []() { return std::make_unique<VoidRegister<CharKV>>(); }
    );

    auto newBlock = std::make_unique<SortedSingleFileIndexedStorage<CharKV, char, void, char>>(
        secondBlockElements.begin(),
        secondBlockElements.end(),
        true,
        "block-2",
        manager,
        [](const CharKV &a, const CharKV &b) { return a.key < b.key; },
        [](const CharKV &a, const CharKV &b) { return a.key == b.key; },
        []() { return std::make_unique<VoidRegister<CharKV>>(); }
    );

    BinaryCollapsingSortedStoragesList<CharKV, char, void, char> list(
        2,
        []() { return std::make_unique<VoidRegister<CharKV>>(); }
    );

    list.append(std::move(oldBlock));
    list.append(std::move(newBlock));

    auto find = [&](char x) {
        return list.find(
            x,
            [](const CharKV &a, const char &t) { return a.key < t; },
            [](const CharKV &a, const char &t) { return t == a.key; }
        );
    };

    CHECK_EQ(find(1), std::optional{CharKV{1, 2}});
    CHECK_EQ(find(2), std::optional{CharKV{2, 1}});
    CHECK_EQ(find(3), std::optional{CharKV{3, 0}});
    CHECK_EQ(find(4), std::optional{CharKV{4, 2}});
    CHECK_EQ(find(0), std::nullopt);
    CHECK_EQ(find(5), std::nullopt);
}

TEST_CASE ("Supermap simple") {
    using namespace supermap;

    using K = Key<2>;
    using V = ByteArray<3>;
    using I = std::size_t;

    using SupermapBuilder = DefaultSupermap<K, V, I>;

    auto superMap = SupermapBuilder::build(
        std::make_unique<BST<K, V, I>>(),
        SupermapBuilder::BuildParameters{
            3,
            0.5,
            "supermap"
        }
    );
    auto key = [](const std::string &s) {
        return Key<2>::fromString(s);
    };
    auto value = [](const std::string &s) {
        return ByteArray<3>::fromString(s);
    };
    CHECK_EQ(superMap->contains(key("aa")), false);
    superMap->add(key("aa"), value("111"));
    CHECK_EQ(superMap->contains(key("aa")), true);
    superMap->add(key("ab"), value("123"));
    CHECK_EQ(superMap->getValue(key("ab")), value("123"));
    CHECK_EQ(superMap->getValue(key("aa")), value("111"));
    superMap->add(key("bb"), value("222"));
    superMap->add(key("cc"), value("333"));
    superMap->add(key("dd"), value("444"));
    superMap->add(key("ee"), value("555"));
    superMap->add(key("ff"), value("666"));
    CHECK_EQ(superMap->getValue(key("aa")), value("111"));
    CHECK_EQ(superMap->getValue(key("ab")), value("123"));
    CHECK_EQ(superMap->getValue(key("bb")), value("222"));
    CHECK_EQ(superMap->getValue(key("cc")), value("333"));
    CHECK_EQ(superMap->getValue(key("dd")), value("444"));
    CHECK_EQ(superMap->getValue(key("ee")), value("555"));
    CHECK_EQ(superMap->getValue(key("ff")), value("666"));
    CHECK_LE(superMap->getUpperSizeBound(), 9);
}
}
