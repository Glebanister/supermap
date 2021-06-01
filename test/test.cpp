#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <filesystem>
#include <vector>

#include "io/InputStream.hpp"
#include "io/InputIterator.hpp"
#include "io/ShallowSerializer.hpp"
#include "io/OutputStream.hpp"
#include "io/OutputIterator.hpp"
#include "io/RamFileManager.hpp"
#include "io/DiskFileManager.hpp"

#include "exception/IllegalArgumentException.hpp"

#include "primitive/Key.hpp"
#include "primitive/Enum.hpp"
#include "primitive/ByteArray.hpp"
#include "core/SingleFileIndexedStorage.hpp"
#include "core/KeyValueShrinkableStorage.hpp"

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

template <>
struct SerializeHelper<char> : ShallowSerializer<char> {};

template <>
struct DeserializeHelper<char> : ShallowDeserializer<char> {};

template <>
struct FixedDeserializedSizeRegister<char> : ShallowDeserializedSize<char> {};

} // supermap::io

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
    auto intIterator = supermap::io::InputIterator<MockStruct>::fromString(ss.str());
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
    writeIterator.writeAll(mocks);
    writeIterator.flush();
    std::vector<MockStruct> parsedMocks;
    auto readIterator = supermap::io::InputIterator<MockStruct>::fromString(buffer, 0);
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
    CHECK_EQ(key6.format(), "123456");
    CHECK_THROWS_AS(supermap::Key<6>::fromString("1234567"), const supermap::IllegalArgumentException &);
    CHECK_THROWS_AS(supermap::Key<6>::fromString("12"), const supermap::IllegalArgumentException &);
}

TEST_CASE ("ByteArray") {
    auto key6 = supermap::ByteArray<6>::fromString("123456");
    CHECK_EQ(key6.toString(), "123456");
    CHECK_THROWS_AS(supermap::ByteArray<6>::fromString("1234567"), const supermap::IllegalArgumentException &);
    CHECK_THROWS_AS(supermap::ByteArray<6>::fromString("12"), const supermap::IllegalArgumentException &);
}

TEST_CASE ("SingleFileIndexedStorage KeyIndex") {
    using namespace supermap;
    using namespace io;

    auto manager = std::make_shared<RamFileManager>();
    const std::string filename = "indexed_data.txt";
    SingleFileIndexedStorage<Enum<Key<2>>> indexedData(filename, manager, 0);

    std::vector<Enum<Key<2>>> keys{
        {Key<2>::fromString("ab"), 0},
        {Key<2>::fromString("cd"), 1},
        {Key<2>::fromString("ef"), 2},
        {Key<2>::fromString("gh"), 3},
    };

    for (std::size_t i = 0; i < keys.size(); ++i) {
        CHECK_EQ(i, indexedData.append(keys[i]));
    }
    std::vector<Enum<Key<2>>> parsedData;
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
    KeyValueShrinkableStorage<2, 4> indexedData("storage-not-sorted", "storage-sorted", manager);

    std::vector<KeyValue<2, 4>> keyValues{
        {Key<2>::fromString("ab"), ByteArray<4>::fromString("1234")},
        {Key<2>::fromString("cd"), ByteArray<4>::fromString("1831")},
        {Key<2>::fromString("ef"), ByteArray<4>::fromString("4923")},
        {Key<2>::fromString("gh"), ByteArray<4>::fromString("3482")},
    };

    for (std::size_t i = 0; i < keyValues.size(); ++i) {
        CHECK_EQ(i, indexedData.append(keyValues[i]));
    }
    std::vector<KeyValue<2, 4>> parsedData;
    auto keyIndexParser = indexedData.getNotSortedEntries();
    while (keyIndexParser.hasNext()) {
        parsedData.push_back(keyIndexParser.next());
    }
    CHECK_EQ(keyValues, parsedData);
}

TEST_CASE("collectWith") {
    using namespace supermap;
    using namespace io;

    auto manager = std::make_shared<RamFileManager>();
    const std::string filename = "indexed_data.txt";
    SingleFileIndexedStorage<KeyValue<2, 4>> storage(filename, manager, 0);

    std::vector<KeyValue<2, 4>> keyValues{
        {Key<2>::fromString("ab"), ByteArray<4>::fromString("1234")},
        {Key<2>::fromString("cd"), ByteArray<4>::fromString("1831")},
        {Key<2>::fromString("ef"), ByteArray<4>::fromString("4923")},
        {Key<2>::fromString("gh"), ByteArray<4>::fromString("3482")},
    };

    std::vector<Key<2>> initialKeys;
    std::transform(keyValues.begin(),
                   keyValues.end(),
                   std::back_inserter(initialKeys),
                   [](const KeyValue<2, 4> &kv) { return kv.key; });

    for (std::size_t i = 0; i < keyValues.size(); ++i) {
        CHECK_EQ(i, storage.append({keyValues[i].key, ByteArray(keyValues[i].value)}));
    }

    auto keyIndexParser = storage.getCustomDataIterator<StorageValueIgnorer<2, 4>>();
    std::vector<Enum<Key<2>>> parsedKeys = keyIndexParser.collectWith(
        [](StorageValueIgnorer<2, 4> &&svi, std::size_t index) {
            return Enum<Key<2>>(std::move(svi.key), index);
        });
    CHECK_EQ(initialKeys.size(), parsedKeys.size());
    for (std::size_t i = 0; i < initialKeys.size(); ++i) {
        CHECK_EQ(initialKeys[i], parsedKeys[i].elem);
        CHECK_EQ(i, parsedKeys[i].index);
    }
}

TEST_CASE("KeyValueShrinkableStorage shrink smoke test") {
    using namespace supermap;

    std::shared_ptr<io::FileManager> manager = std::make_shared<io::DiskFileManager>();
    KeyValueShrinkableStorage<2, 4> storage(
        "not-sorted",
        "sorted",
        manager);
    CHECK_NOTHROW(auto newIndex = storage.shrink(10, "new-index"));
}

TEST_CASE("Enum serialization") {
    using namespace supermap;

    std::string buffer;
    io::StringOutputStream out(buffer, false);
    io::serialize(Enum<char>('a', 1), out.get());
    io::serialize(Enum<char>('b', 2), out.get());
    out.flush();
    io::StringInputStream in(buffer, 0);
    CHECK_EQ(io::deserialize<Enum<char>>(in.get()), Enum<char>{'a', 1});
    CHECK_EQ(io::deserialize<Enum<char>>(in.get()), Enum<char>{'b', 2});
}

TEST_CASE("Enum collect ram") {
    using namespace supermap;

    std::string buffer;
    io::StringOutputStream out(buffer, false);
    io::serialize(Enum<char>('a', 1), out.get());
    io::serialize(Enum<char>('b', 2), out.get());
    out.flush();
    io::InputIterator<Enum<char>> inpIt(std::make_unique<io::StringInputStream>(buffer, 0));
    std::vector<Enum<char>> enums = inpIt.collect();
    CHECK_EQ(enums.size(), 2);
    CHECK_EQ(enums[0], Enum<char>{'a', 1});
    CHECK_EQ(enums[1], Enum<char>{'b', 2});
}

TEST_CASE("Enum collect disk") {
    using namespace supermap;

    TempFile file("test-Enum-collect-disk");

    std::string buffer;
    io::FileOutputStream out(file.filename, false);
    io::serialize(Enum<char>('a', 1), out.get());
    io::serialize(Enum<char>('b', 2), out.get());
    out.flush();
    io::InputIterator<Enum<char>> inpIt(std::make_unique<io::FileInputStream>(file.filename, 0));
    std::vector<Enum<char>> enums = inpIt.collect();
    CHECK_EQ(enums.size(), 2);
    CHECK_EQ(enums[0], Enum<char>{'a', 1});
    CHECK_EQ(enums[1], Enum<char>{'b', 2});
}

TEST_CASE("Serialize KeyValue Disk") {
    using namespace supermap;

    std::shared_ptr<io::FileManager> manager = std::make_shared<io::DiskFileManager>();
    TempFile file("test-Serialize-KeyValue");
    auto outIt = manager->getOutputIterator<KeyValue<2, 4>>(file.filename, false);
    auto kv1 = KeyValue{Key<2>::fromString("ab"), ByteArray<4>::fromString("1234")};
    auto kv2 = KeyValue{Key<2>::fromString("cd"), ByteArray<4>::fromString("5678")};
    auto kv3 = KeyValue{Key<2>::fromString("ef"), ByteArray<4>::fromString("1023")};
    outIt.write(kv1);
    outIt.write(kv2);
    outIt.write(kv3);
    outIt.flush();
    auto inIt = manager->getInputIterator<KeyValue<2, 4>>(file.filename, 0);
    auto read = inIt.collectWith([](KeyValue<2, 4> &&kv, std::uint32_t index) {
        return Enum<KeyValue<2, 4>>(std::move(kv),
                                    index);
    });
    CHECK_EQ(read.size(), 3);
    CHECK_EQ(read[0].index, 0);
    CHECK_EQ(read[1].index, 1);
    CHECK_EQ(read[2].index, 2);
    CHECK_EQ(read[0].elem, kv1);
    CHECK_EQ(read[1].elem, kv2);
    CHECK_EQ(read[2].elem, kv3);
}

TEST_CASE("KeyValueShrinkableStorage shrink") {
    using namespace supermap;

    std::shared_ptr<io::FileManager> manager = std::make_shared<io::DiskFileManager>();
    KeyValueShrinkableStorage<2, 4> storage(
        "not-sorted",
        "sorted",
        manager
    );
    storage.append({Key<2>::fromString("bb"), ByteArray<4>::fromString("1111")});
    storage.append({Key<2>::fromString("aa"), ByteArray<4>::fromString("2222")});
    storage.append({Key<2>::fromString("cc"), ByteArray<4>::fromString("3333")});
    CHECK_EQ(storage.getItemsCount(), 3);
    {
        std::vector<Key<2>> notSortedKeys =
            storage.getNotSortedKeys().collectWith([](StorageValueIgnorer<2, 4> &&kvi, std::uint32_t) {
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
            storage.getSortedKeys().collectWith([](StorageValueIgnorer<2, 4> &&svi, std::uint32_t) {
                return svi.key;
            });
        CHECK_EQ(sortedKeys, std::vector<Key<2>>{});
    }
    SortedSingleFileIndexedStorage<Enum<Key<2>>> newIndex = storage.shrink(
        1,
        "new-index"
    );
    CHECK_EQ(newIndex.getItemsCount(), 3);
    CHECK_EQ(newIndex.getStorageFilePath(), "new-index");
    CHECK_EQ(newIndex.getFileManager(), manager);
}
