#pragma once

#include "io/FileManager.hpp"

#include "ByteArray.hpp"

namespace supermap {

template <typename T>
class IndexedData {
  public:
    static constexpr std::size_t EACH_SIZE = sizeof(T);

  public:
    explicit IndexedData(std::string dataFileName,
                         std::shared_ptr<io::FileManager> manager)
        : items_(0),
          dataFileName_(std::move(dataFileName)),
          fileManager_(std::move(manager)) {}

    void append(const T &item) {
        io::Writer<T> writer = fileManager_->template getWriter<T>(dataFileName_, true);
        writer.write(item);
        writer.flush();
        items_ += 1;
    }

    T get(std::uint64_t index) {
        if (index >= items_) {
            throw IllegalArgumentException(std::string() +
                "Index " + std::to_string(index) +
                " is out of IndexedData of size " + std::to_string(items_));
        }
        io::Parser<T> parser = fileManager_->template getParser<T>(dataFileName_, index * EACH_SIZE);
        return parser.next();
    }

    io::Parser<T> getDataParser() {
        return fileManager_->template getParser<T>(dataFileName_, 0);
    }

    template <typename Out>
    io::Parser<Out> getCustomDataParser() const {
        return io::Parser<Out>(fileManager_->getInputStream(dataFileName_, 0));
    }

    [[nodiscard]] std::size_t getSize() const noexcept {
        return items_;
    }

  private:
    uint64_t items_;
    const std::string dataFileName_;
    std::shared_ptr<io::FileManager> fileManager_;
};

} // supermap
