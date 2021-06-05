#pragma once

#include <cstdint>
#include <type_traits>
#include <utility>

#include "io/FileManager.hpp"
#include "io/TemporaryFile.hpp"
#include "IndexedStorage.hpp"

namespace supermap {

/**
 * @brief Indexed storage that stores all items in the single file.
 * @tparam T Contained objects type.
 * @tparam IndexT Contained objects index.
 * @tparam RegisterType Type of inner item register.
 */
template <typename T, typename IndexT, typename RegisterType>
class SingleFileIndexedStorage : public IndexedStorage<T, IndexT, RegisterType> {
  public:
    using OrderedStorage<T, IndexT, RegisterType>::getItemsCount;
    using OrderedStorage<T, IndexT, RegisterType>::getRegister;

    /**
     * @brief Creates an empty storage.
     * @param storageFilePath Path of the storage file.
     * @param fileManager Shared access to the file manager.
     * @param reg Item register.
     */
    explicit SingleFileIndexedStorage(const std::filesystem::path &storageFilePath,
                                      std::shared_ptr<io::FileManager> fileManager)
        : storageFile_(std::make_shared<io::TemporaryFile>(storageFilePath, std::move(fileManager))) {
        storageFile_->getFileManager()->create(storageFilePath);
    }

    /**
     * @return Path of the file, associated with this storage.
     */
    [[nodiscard]] std::string getStorageFilePath() const noexcept {
        return storageFile_->getPath();
    }

    /**
     * @return Shared access to the file system manager.
     */
    [[nodiscard]] std::shared_ptr<io::FileManager> getFileManager() const noexcept {
        return storageFile_->getFileManager();
    }

    /**
     * @return The file, associated with this storage.
     */
    [[nodiscard]] std::shared_ptr<io::TemporaryFile> shareStorageFile() const noexcept {
        return storageFile_;
    }

    /**
     * @brief Resets this storage with other. Storage file contents
     * are being replaced with each other.
     * @param other Storage to reset with.
     */
    virtual void resetWith(SingleFileIndexedStorage<T, IndexT, RegisterType> &&other) noexcept {
        getRegister() = std::move(other.getRegister());
        auto thisFileManager = getFileManager();
        assert(other.getFileManager() == thisFileManager);
        thisFileManager->swap(other.getStorageFilePath(), getStorageFilePath());
    }

    /**
     * @brief Appends an item to the end of storage.
     * @param item New object to be added.
     * @return Index of added item in this storage.
     */
    void append(std::unique_ptr<T> &&item) override {
        io::OutputIterator<T> writer = getFileManager()->template getOutputIterator<T>(getStorageFilePath(), true);
        writer.write(*item);
        writer.flush();
        getRegister().registerItem(*item);
    }

    /**
     * @brief Appends all items from @p begin to the @p end to this storage.
     * @tparam IteratorT Type of iterator.
     * @tparam Functor Type of @p func.
     * @tparam Result Type of @p func application to the @p *it. Must be the same as @p T.
     * @param begin Collection begin iterator.
     * @param end Collections end iterator.
     * @param func Functor which is applied to the every item in collection.
     */
    template <
        typename IteratorT,
        typename Functor,
        typename Result = std::invoke_result_t<Functor, typename std::iterator_traits<IteratorT>::value_type>,
        typename = std::enable_if_t<std::is_same_v<Result, T>>
    >
    void appendAll(IteratorT begin, IteratorT end, Functor func) {
        io::OutputIterator<Result> writer
            = getFileManager()->template getOutputIterator<Result>(getStorageFilePath(), true);
        writer.writeAll(begin, end, [&](const auto &obj) {
            auto fObj = func(obj);
            getRegister().registerItem(fObj);
            return fObj;
        });
        writer.flush();
    }

    /**
     * @brief Appends all items from @p begin to the @p end.
     * @tparam IteratorT Iterator type.
     * @param begin Collection begin iterator.
     * @param end Collection end iterator.
     */
    template <
        typename IteratorT,
        typename = std::enable_if_t<std::is_same_v<T, typename std::iterator_traits<IteratorT>::value_type>>
    >
    void appendAll(IteratorT begin, IteratorT end) {
        appendAll(begin, end, [](auto x) { return x; });
    }

    /**
     * @return Element contained by index @p index.
     */
    [[nodiscard]] T get(IndexT index) const override {
        assert(index < getItemsCount());
        return getFileManager()->template getInputIterator<T, IndexT>(
            getStorageFilePath(),
            index * io::FixedDeserializedSizeRegister<T>::exactDeserializedSize
        ).next();
    }

    /**
     * @return Associated storage elements input iterator over type @p T.
     */
    io::InputIterator<T, IndexT> getDataIterator() const {
        return getFileManager()->template getInputIterator<T, IndexT>(getStorageFilePath(), 0);
    }

    /**
     * @return Associated storage elements input iterator over type @p Out.
     */
    template <typename Out>
    io::InputIterator<Out, IndexT> getCustomDataIterator() const {
        return io::InputIterator<Out, IndexT>(getFileManager()->getInputStream(getStorageFilePath(), 0));
    }

  protected:
    std::shared_ptr<io::TemporaryFile> storageFile_;
};

} // supermap
