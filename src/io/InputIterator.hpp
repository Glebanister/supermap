#pragma once

#include <memory>
#include <vector>
#include <cassert>

#include "InputStream.hpp"
#include "SerializeHelper.hpp"
#include "exception/IteratorException.hpp"

namespace supermap::io {

/**
 * @brief Iterator which allows to read continuous objects of type @p T from input stream.
 * @tparam T Iterated object type.
 * @tparam IndexT Type of index that keeps track of the number of the read object.
 * @tparam EachSize Size of each @p T object in serialized format.
 */
template <
    typename T,
    typename IndexT,
    std::size_t EachSize = FixedDeserializedSizeRegister<T>::exactDeserializedSize,
    typename = std::enable_if_t<DeserializeHelper<T>::isDeserializable>
>
class InputIterator {
  public:
    /**
     * @brief Creates new @p InputIterator from input stream.
     * @param input @p InputStream ownership.
     */
    explicit InputIterator(std::unique_ptr<InputStream> &&input)
        : input_(std::move(input)) {}

    /**
     * @brief Creates @p T input iterator over file input stream.
     * @param filename Name of file to read from.
     * @param offset Read begin offset.
     * @return Created iterator.
     */
    static InputIterator fromFile(const std::string &filename, std::uint64_t offset = 0) {
        return InputIterator(std::make_unique<FileInputStream>(filename, offset));
    }

    /**
     * @brief Creates @p T input iterator over string input stream.
     * @param filename Name of file to read from.
     * @param offset Read begin offset.
     * @return Created iterator.
     */
    static InputIterator fromString(std::string data, std::uint64_t offset = 0) {
        return InputIterator(std::make_unique<StringInputStream>(data, offset));
    }

  public:
    InputIterator() = delete;
    InputIterator(const InputIterator &) = default;
    InputIterator(InputIterator &&) noexcept = default;

    /**
     * @brief Checks if any more objects can be received from stream.
     * @return If at least one more object can be read from input stream.
     */
    [[nodiscard]] bool hasNext() const noexcept {
        return input_->availableBytes() >= EachSize;
    }

    /**
     * @brief Reads new object from input stream. Does not check if
     * any more objects can be read, so if @p hasNext is @p false,
     * behavior is not defined.
     * @return Next object in stream.
     */
    T next() {
        ++index_;
        return deserialize<T>(input_->get());
    }

    /**
     * @brief Collects some objects from stream to @p std::vector.
     * @p functor is applied to all objects before add to resulting collection.
     * @tparam Functor type of applied function.
     * @tparam Result Type of collected collection.
     * @param functor Function which is applied to every collected object.
     * @param collectionSizeLimit limit to the number of read objects. If @p 0,
     * then all stream is being read.
     * @return Collection of read objects.
     */
    template <
        typename Functor,
        typename Result = std::invoke_result_t<Functor, T &&, IndexT>
    >
    std::vector<Result> collectWith(Functor functor, IndexT collectionSizeLimit = 0) {
        std::vector<Result> collection;
        IndexT objectsInStream;
        {
            auto bytes = input_->availableBytes();
            assert(bytes % EachSize == 0);
            objectsInStream = bytes / EachSize;
        }
        IndexT objectsToRead;
        if (collectionSizeLimit == 0) {
            objectsToRead = objectsInStream;
        } else {
            objectsToRead = std::min(objectsInStream, collectionSizeLimit);
        }
        collection.reserve(objectsToRead);
        const std::uint64_t bytesToRead = static_cast<std::uint64_t>(objectsToRead) * EachSize;
        std::unique_ptr<char[]> bytes = std::make_unique<char[]>(bytesToRead);
        input_->get().read(bytes.get(), bytesToRead);
        std::stringstream iss(std::string(bytes.get(), bytesToRead), std::ios_base::in);
        for (IndexT objectI = 0; objectI < objectsToRead; ++objectI) {
            collection.push_back(functor(deserialize<T>(iss), index_++));
        }
        return collection;
    }

    /**
     * @brief Collects all from input stream to @p std::vector.
     * @param collectionSizeLimit limit to the number of read objects.
     * @return All collected objects.
     */
    std::vector<T> collect(IndexT collectionSizeLimit = 0) {
        return collectWith([](auto &&x, IndexT) { return std::move(x); }, collectionSizeLimit);
    }

    std::unique_ptr<InputStream> input_;
    IndexT index_ = 0;
};

} // supermap::io
