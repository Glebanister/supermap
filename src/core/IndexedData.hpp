#pragma once

#include "io/FileManager.hpp"

#include "ByteArray.hpp"

namespace supermap {

template <typename T>
class IndexedData {
  public:
    static constexpr std::size_t EACH_SIZE = sizeof(T);

  public:
    explicit IndexedData(std::uint64_t items,
                         std::string dataFileName,
                         std::shared_ptr<io::FileManager> manager)
        : items_(items),
          dataFileName_(std::move(dataFileName)),
          fileManager_(std::move(manager)) {}

    explicit IndexedData(std::string dataFileName,
                         std::shared_ptr<io::FileManager> manager)
        : IndexedData(0, std::move(dataFileName), std::move(manager)) {}

    std::uint64_t append(const T &item) {
        io::OutputIterator<T> writer = fileManager_->template getOutputIterator<T>(dataFileName_, true);
        writer.write(item);
        writer.flush();
        return items_++;
    }

    T get(std::uint64_t index) {
        if (index >= items_) {
            throw IllegalArgumentException(std::string() +
                "Index " + std::to_string(index) +
                " is out of IndexedData of size " + std::to_string(items_));
        }
        return fileManager_->template getInputIterator<T>(dataFileName_, index * EACH_SIZE).next();
    }

    io::InputIterator<T> getDataParser() {
        return fileManager_->template getInputIterator<T>(dataFileName_, 0);
    }

    template <typename Out>
    io::InputIterator<Out> getCustomDataParser() const {
        return io::InputIterator<Out>(fileManager_->getInputStream(dataFileName_, 0));
    }

    [[nodiscard]] std::size_t getSize() const noexcept {
        return items_;
    }

  private:
    uint64_t items_{};
    const std::string dataFileName_;
    std::shared_ptr<io::FileManager> fileManager_;
};

} // supermap
