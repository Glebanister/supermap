#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <filesystem>
#include <vector>

#include "io/InputStream.hpp"
#include "io/Parser.hpp"
#include "io/StackMemorySerializer.hpp"
#include "io/OutputStream.hpp"
#include "io/Writer.hpp"
#include "io/RamFileManager.hpp"

#include "exception/IllegalArgumentException.hpp"

#include "core/Key.hpp"

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
struct DeserializeHelper<int> : Deserializable<true, 1> {
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

TEST_CASE ("Parser int") {
    auto intIterator = supermap::io::Parser<int>::fromString("1 2 3", 0);
    std::vector<int> parsed;
    while (intIterator.hasNext()) {
        parsed.push_back(intIterator.next());
    }
    CHECK_EQ(std::vector{1, 2, 3}, parsed);
}

TEST_CASE ("Parser MockStruct") {
    std::stringstream ss;
    std::vector<MockStruct> before = {
        {12343, 'a', {312, 0, 1122}},
        {32, 'b', {13, 3, 9441}},
        {18, 'c', {4010, 31, 321123}},
    };
    for (const auto &s : before) {
        supermap::io::serialize(s, ss);
    }
    auto intIterator = supermap::io::Parser<MockStruct>::fromString(ss.str());
    std::vector<MockStruct> parsed;
    while (intIterator.hasNext()) {
        parsed.push_back(intIterator.next());
    }
    CHECK(before == parsed);
}

TEST_CASE ("Writer int") {
    std::string buffer;
    std::vector<int> ints = {1, 2, 3, 4, 5};
    auto writeIterator = supermap::io::Writer<int>::toString(buffer, false);
    writeIterator.writeAll(ints);
    writeIterator.flush();
    CHECK_EQ(buffer, "1 2 3 4 5 ");
}

TEST_CASE ("Writer MockStruct") {
    std::string buffer;
    std::vector<MockStruct> mocks = {{1, 'x', {4, 15, 6}},
                                     {2, 'y', {91, 905, 6}},
                                     {3, 'z', {41012, 5, 236}}};
    auto writeIterator = supermap::io::Writer<MockStruct>::toString(buffer, false);
    writeIterator.writeAll(mocks);
    writeIterator.flush();
    std::vector<MockStruct> parsedMocks;
    auto readIterator = supermap::io::Parser<MockStruct>::fromString(buffer, 0);
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
