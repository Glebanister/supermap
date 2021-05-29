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
          dataFileName_(dataFileName),
          fileManager_(std::move(manager)) {
        fileManager_->create(dataFileName);
    }

    explicit IndexedData(std::string dataFileName,
                         std::shared_ptr<io::FileManager> manager)
        : IndexedData(0, std::move(dataFileName), std::move(manager)) {}

    IndexedData<T>(const IndexedData<T> &) = default;
    IndexedData<T> &operator=(IndexedData<T> &&other) noexcept = default;

    std::uint64_t append(const T &item) {
        io::OutputIterator<T> writer = fileManager_->template getOutputIterator<T>(dataFileName_, true);
        writer.write(item);
        writer.flush();
        return items_++;
    }

    void appendAll(typename std::vector<T>::const_iterator begin, typename std::vector<T>::const_iterator end) {
        io::OutputIterator<T> writer = fileManager_->template getOutputIterator<T>(dataFileName_, true);
        for (auto it = begin; it < end; ++it) {
            writer.write(*it);
            items_++;
        }
        writer.flush();
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

    [[nodiscard]] std::shared_ptr<io::FileManager> getManager() const noexcept {
        return fileManager_;
    }

    [[nodiscard]] const std::string &getDataFileName() const noexcept {
        return dataFileName_;
    }

  private:
    uint64_t items_{};

  protected:
    std::string dataFileName_;
    std::shared_ptr<io::FileManager> fileManager_;
};

} // supermap
