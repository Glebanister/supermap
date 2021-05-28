#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"

#include <filesystem>
#include <vector>

#include "stream/InputStream.hpp"
#include "stream/ParsingIterator.hpp"
#include "stream/StackMemorySerializer.hpp"
#include "stream/OutputStream.hpp"
#include "stream/WritingIterator.hpp"

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

namespace supermap::stream {

template <>
struct SerializeHelper<MockStruct> : StackMemorySerializer<MockStruct> {};

template <>
struct SerializeHelper<int> : Serializable<true> {
    static void serialize(const int &value, std::ostream &os) {
        os << value << ' ';
    }

    static int deserialize(std::istream &is) {
        int value;
        is >> value;
        return value;
    }
};

} // supermap::stream

TEST_CASE ("StringInputStream") {
    supermap::stream::StringInputStream iss("1 2 3", 0);
    int x, y, z;
    iss.get() >> x >> y >> z;
        CHECK_EQ(x, 1);
        CHECK_EQ(y, 2);
        CHECK_EQ(z, 3);
}

TEST_CASE ("FileInputStream") {
    TempFile file("1 2 3");
    supermap::stream::FileInputStream ifs(file.filename, 0);
    int x, y, z;
    ifs.get() >> x >> y >> z;
        CHECK_EQ(x, 1);
        CHECK_EQ(y, 2);
        CHECK_EQ(z, 3);
}

TEST_CASE ("StringOutputStream") {
    std::string buffer;
    supermap::stream::StringOutputStream os(buffer, true);
    os.get() << 1 << ' ' << 2 << ' ' << 3;
    os.flush();
        CHECK_EQ(buffer, "1 2 3");
}

TEST_CASE ("FileOutputStream") {
    TempFile file("1 2 ");
    supermap::stream::FileOutputStream ofs(file.filename, true);
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
    supermap::stream::FileInputStream ifs(file.filename, 2);
    int y, z;
    ifs.get() >> y >> z;
        CHECK_EQ(y, 2);
        CHECK_EQ(z, 3);
}

TEST_CASE ("ParsingIterator int") {
    auto intIterator = supermap::stream::ParsingIterator<int>::fromString("1 2 3", 0);
    std::vector<int> parsed;
    while (intIterator.hasNext()) {
        parsed.push_back(intIterator.next());
    }
        CHECK_EQ(std::vector{1, 2, 3}, parsed);
}

TEST_CASE ("ParsingIterator MockStruct") {
    std::stringstream ss;
    std::vector<MockStruct> before = {
        {12343, 'a', {312, 0, 1122}},
        {32, 'b', {13, 3, 9441}},
        {18, 'c', {4010, 31, 321123}},
    };
    for (const auto &s : before) {
        supermap::stream::serialize(s, ss);
    }
    auto intIterator = supermap::stream::ParsingIterator<MockStruct>::fromString(ss.str());
    std::vector<MockStruct> parsed;
    while (intIterator.hasNext()) {
        parsed.push_back(intIterator.next());
    }
        CHECK(before == parsed);
}

TEST_CASE ("WritingIterator int") {
    std::string buffer;
    std::vector<int> ints = {1, 2, 3, 4, 5};
    auto writeIterator = supermap::stream::WritingIterator<int>::toString(buffer, false);
    writeIterator.writeAll(ints);
    writeIterator.flush();
        CHECK_EQ(buffer, "1 2 3 4 5 ");
}

TEST_CASE ("WritingIterator MockStruct") {
    std::string buffer;
    std::vector<MockStruct> mocks = {{1, 'x', {4, 15, 6}},
                                     {2, 'y', {91, 905, 6}},
                                     {3, 'z', {41012, 5, 236}}};
    auto writeIterator = supermap::stream::WritingIterator<MockStruct>::toString(buffer, false);
    writeIterator.writeAll(mocks);
    writeIterator.flush();
    std::vector<MockStruct> parsedMocks;
    auto readIterator = supermap::stream::ParsingIterator<MockStruct>::fromString(buffer, 0);
    while (readIterator.hasNext()) {
        parsedMocks.push_back(readIterator.next());
    }
        CHECK_EQ(parsedMocks, mocks);
}
