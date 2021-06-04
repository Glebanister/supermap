#pragma once

#include <cstdint>
#include <type_traits>
#include <utility>

#include "io/FileManager.hpp"
#include "io/TemporaryFile.hpp"
#include "IndexedStorage.hpp"

namespace supermap {

template <typename T, typename IndexT>
class SingleFileIndexedStorage : public IndexedStorage<T, IndexT> {
  public:
    using IndexedStorage<T, IndexT>::getFileManager;
    using IndexedStorage<T, IndexT>::getItemsCount;

    explicit SingleFileIndexedStorage(const std::filesystem::path &storageFilePath,
                                      std::shared_ptr<io::FileManager> fileManager,
                                      IndexT itemsCount)
        : storageFile_(std::make_shared<io::TemporaryFile>(storageFilePath, std::move(fileManager))),
          itemsCount_(itemsCount) {
        storageFile_->getFileManager()->create(storageFilePath);
    }

    [[nodiscard]] std::string getStorageFilePath() const noexcept {
        return storageFile_->getPath();
    }

    [[nodiscard]] std::shared_ptr<io::FileManager> getFileManager() const noexcept override {
        return storageFile_->getFileManager();
    }

    [[nodiscard]] std::shared_ptr<io::TemporaryFile> shareStorageFile() const noexcept {
        return storageFile_;
    }

    void resetWith(SingleFileIndexedStorage<T, IndexT> &&other) noexcept {
        itemsCount_ = other.itemsCount_;
        auto thisFileManager = getFileManager();
        assert(other.getFileManager() == thisFileManager);
        thisFileManager->swap(other.getStorageFilePath(), getStorageFilePath());
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
        typename Result = std::invoke_result_t<Functor, typename std::iterator_traits<IteratorT>::value_type>,
        typename = std::enable_if_t<std::is_same_v<Result, T>>
    >
    void appendAll(IteratorT begin, IteratorT end, Functor func) {
        io::OutputIterator<Result>
            writer = getFileManager()->template getOutputIterator<Result>(getStorageFilePath(), true);
        writer.writeAll(begin, end, func);
        itemsCount_ += std::distance(begin, end);
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
        assert(index < getItemsCount());
        return getFileManager()->template getInputIterator<T, IndexT>(
            getStorageFilePath(),
            index * io::FixedDeserializedSizeRegister<T>::exactDeserializedSize
        ).next();
    }

    io::InputIterator<T, IndexT> getDataIterator() const {
        return getFileManager()->template getInputIterator<T, IndexT>(getStorageFilePath(), 0);
    }

    template <typename Out>
    io::InputIterator<Out, IndexT> getCustomDataIterator() const {
        return io::InputIterator<Out, IndexT>(getFileManager()->getInputStream(getStorageFilePath(), 0));
    }

    IndexT getItemsCount() const noexcept override {
        return itemsCount_;
    }

  protected:
    IndexT increaseItemsCount() noexcept {
        return itemsCount_++;
    }

    std::shared_ptr<io::TemporaryFile> storageFile_;
    IndexT itemsCount_;
};

} // supermap
