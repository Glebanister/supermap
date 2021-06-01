#pragma once

#include <cstdint>
#include <type_traits>
#include <utility>

#include "io/FileManager.hpp"
#include "IndexedStorage.hpp"

namespace supermap {

template <typename T, typename IndexT = std::size_t>
class SingleFileIndexedStorage : public IndexedStorage<T, IndexT> {
  public:
    using IndexedStorage<T, IndexT>::getFileManager;
    using IndexedStorage<T, IndexT>::getItemsCount;

    explicit SingleFileIndexedStorage(const std::filesystem::path &storageFilePath,
                                      std::shared_ptr<io::FileManager> fileManager,
                                      IndexT itemsCount)
        : storageFilePath_(storageFilePath),
          fileManager_(std::move(fileManager)),
          itemsCount_(itemsCount) {
        fileManager_->create(storageFilePath);
    }

    [[nodiscard]] std::filesystem::path getStorageFilePath() const noexcept {
        return storageFilePath_;
    }

    [[nodiscard]] std::shared_ptr<io::FileManager> getFileManager() const noexcept override {
        return fileManager_;
    }

    void resetWith(SingleFileIndexedStorage<T, IndexT> &&other) noexcept {
        auto thisFileManager = getFileManager();
        assert(other.getFileManager() == thisFileManager);
        thisFileManager->swap(other.getStorageFilePath(), this->getStorageFilePath());
        thisFileManager->remove(getStorageFilePath());
    }

    IndexT append(const T &item) override {
        io::OutputIterator<T> writer = getFileManager()->template getOutputIterator<T>(getStorageFilePath(), true);
        writer.write(item);
        writer.flush();
        return increaseItemsCount();
    }

    template <
        typename IteratorT,
        typename Functor,
        typename Result = std::invoke_result_t<Functor, typename std::iterator_traits<IteratorT>::value_type>
    >
    void appendAll(IteratorT begin, IteratorT end, Functor func) {
        io::OutputIterator<Result>
            writer = getFileManager()->template getOutputIterator<Result>(getStorageFilePath(), true);
        for (auto it = begin; it < end; ++it) {
            writer.write(func(*it));
            increaseItemsCount();
        }
        writer.flush();
    }

    template <
        typename IteratorT,
        typename = std::enable_if_t<std::is_same_v<T, typename std::iterator_traits<IteratorT>::value_type>>
    >
    void appendAll(IteratorT begin, IteratorT end) {
        appendAll(begin, end, [](auto x) { return x; });
    }

    [[nodiscard]] T get(IndexT index) const override {
        if (index >= getItemsCount()) {
            throw IllegalArgumentException(std::string() +
                "Index " + std::to_string(index) +
                " is out of SingleFileIndexedStorage of size " +
                std::to_string(getItemsCount()));
        }
        return getFileManager()->template getInputIterator<T>(getStorageFilePath(), index * sizeof(T)).next();
    }

    io::InputIterator<T> getDataIterator() const {
        return getFileManager()->template getInputIterator<T>(getStorageFilePath(), 0);
    }

    template <typename Out>
    io::InputIterator<Out> getCustomDataIterator() const {
        return io::InputIterator<Out>(getFileManager()->getInputStream(getStorageFilePath(), 0));
    }

    IndexT getItemsCount() const noexcept override {
        return itemsCount_;
    }

  protected:
    IndexT increaseItemsCount() noexcept {
        return itemsCount_++;
    }

    std::filesystem::path storageFilePath_;
    std::shared_ptr<io::FileManager> fileManager_;
    IndexT itemsCount_;
};

} // supermap
