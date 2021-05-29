#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <filesystem>
#include <vector>

#include "io/InputStream.hpp"
#include "io/InputIterator.hpp"
#include "io/StackMemorySerializer.hpp"
#include "io/OutputStream.hpp"
#include "io/OutputIterator.hpp"
#include "io/RamFileManager.hpp"

#include "exception/IllegalArgumentException.hpp"

#include "core/Key.hpp"
#include "core/ByteArray.hpp"
#include "core/IndexedData.hpp"
#include "core/KeyIndex.hpp"
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
struct SerializeHelper<MockStruct> : StackMemorySerializer<MockStruct> {};

template <>
struct DeserializeHelper<MockStruct> : StackMemoryDeserializer<MockStruct> {};

template <>
struct SerializeHelper<int> : Serializable<true> {
    static void serialize(const int &value, std::ostream &os) {
        os << value << ' ';
    }
};

template <>
struct DeserializeHelper<int> : Deserializable<true, 2> {
    static int deserialize(std::istream &is) {
        int value;
        is >> value;
        return value;
    }
};

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

TEST_CASE ("InputIterator int") {
    auto intIterator = supermap::io::InputIterator<int>::fromString("1 2 3", 0);
    std::vector<int> parsed;
    while (intIterator.hasNext()) {
        parsed.push_back(intIterator.next());
    }
    CHECK_EQ(std::vector{1, 2, 3}, parsed);
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

TEST_CASE ("OutputIterator int") {
    std::string buffer;
    std::vector<int> ints = {1, 2, 3, 4, 5};
    auto writeIterator = supermap::io::OutputIterator<int>::toString(buffer, false);
    writeIterator.writeAll(ints);
    writeIterator.flush();
    CHECK_EQ(buffer, "1 2 3 4 5 ");
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

TEST_CASE ("IndexedData KeyIndex") {
    using namespace supermap;
    using namespace io;

    auto manager = std::make_shared<RamFileManager>();
    const std::string filename = "indexed_data.txt";
    IndexedData<KeyIndex<2>> indexedData(filename, manager);

    std::vector<KeyIndex<2>> keys{
        {Key<2>::fromString("ab"), 0},
        {Key<2>::fromString("cd"), 1},
        {Key<2>::fromString("ef"), 2},
        {Key<2>::fromString("gh"), 3},
    };

    for (std::size_t i = 0; i < keys.size(); ++i) {
        CHECK_EQ(i, indexedData.append(keys[i]));
    }
    std::vector<KeyIndex<2>> parsedData;
    auto keyIndexParser = indexedData.getDataParser();
    while (keyIndexParser.hasNext()) {
        parsedData.push_back(keyIndexParser.next());
    }
    CHECK_EQ(keys, parsedData);
}

TEST_CASE ("IndexedData KeyValue") {
    using namespace supermap;
    using namespace io;

    using KeyValue = KeyValue<2, 4>;

    auto manager = std::make_shared<RamFileManager>();
    const std::string filename = "indexed_data.txt";
    IndexedData<KeyValue> indexedData(filename, manager);

    std::vector<KeyValue> keyValues{
        {Key<2>::fromString("ab"), ByteArray<4>::fromString("1234")},
        {Key<2>::fromString("cd"), ByteArray<4>::fromString("1831")},
        {Key<2>::fromString("ef"), ByteArray<4>::fromString("4923")},
        {Key<2>::fromString("gh"), ByteArray<4>::fromString("3482")},
    };

    for (std::size_t i = 0; i < keyValues.size(); ++i) {
        CHECK_EQ(i, indexedData.append(keyValues[i]));
    }
    std::vector<KeyValue> parsedData;
    auto keyIndexParser = indexedData.getDataParser();
    while (keyIndexParser.hasNext()) {
        parsedData.push_back(keyIndexParser.next());
    }
    CHECK_EQ(keyValues, parsedData);
}

TEST_CASE ("KeyValueShrinkableStorage") {
    using namespace supermap;
    using namespace io;

    auto manager = std::make_shared<RamFileManager>();
    const std::string filename = "indexed_data.txt";
    KeyValueShrinkableStorage<2, 4> storage(filename, manager);

    std::vector<KeyValue<2, 4>> keyValues{
        {Key<2>::fromString("ab"), ByteArray<4>::fromString("1234")},
        {Key<2>::fromString("cd"), ByteArray<4>::fromString("1831")},
        {Key<2>::fromString("ef"), ByteArray<4>::fromString("4923")},
        {Key<2>::fromString("gh"), ByteArray<4>::fromString("3482")},
    };

    std::vector<Key<2>> initialKeys;

    std::transform(keyValues.begin(), keyValues.end(), std::back_inserter(initialKeys),
                   [](const KeyValue<2, 4> &kv) { return kv.key; });

    for (std::size_t i = 0; i < keyValues.size(); ++i) {
        CHECK_EQ(i, storage.add(keyValues[i].key, ByteArray(keyValues[i].value)));
    }

    std::vector<Key<2>> parsedKeys;
    auto keyIndexParser = storage.getKeys();
    while (keyIndexParser.hasNext()) {
        parsedKeys.push_back(keyIndexParser.next().key);
    }
    CHECK_EQ(initialKeys, parsedKeys);
}

TEST_CASE("FunctorIterator") {
    using namespace supermap;
    using namespace io;

    auto manager = std::make_shared<RamFileManager>();
    const std::string filename = "indexed_data.txt";
    KeyValueShrinkableStorage<2, 4> storage(filename, manager);

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
        CHECK_EQ(i, storage.add(keyValues[i].key, ByteArray(keyValues[i].value)));
    }

    auto keyIndexParser = storage.getKeys();
    auto parsedKeys = keyIndexParser
        .map<Key<2>>([](StorageValueIgnorer<2, 4> &&svi) { return svi.key; })
        .collectToVector();
    CHECK_EQ(initialKeys, parsedKeys);
}
