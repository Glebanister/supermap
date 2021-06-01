#pragma once

#include <memory>
#include <vector>
#include <cassert>
#include <iostream>

#include "InputStream.hpp"
#include "SerializeHelper.hpp"
#include "exception/IteratorException.hpp"

namespace supermap::io {

template <
    typename T,
    std::size_t EachSize = FixedDeserializedSizeRegister<T>::exactDeserializedSize,
    typename = std::enable_if_t<DeserializeHelper<T>::isDeserializable>
>
class InputIterator {
  public:
    explicit InputIterator(std::unique_ptr<InputStream> &&input)
        : input_(std::move(input)) {}

    static InputIterator fromFile(const std::string &filename, std::uint64_t offset = 0) {
        return InputIterator(std::make_unique<FileInputStream>(filename, offset));
    }

    static InputIterator fromString(std::string data, std::uint64_t offset = 0) {
        return InputIterator(std::make_unique<StringInputStream>(data, offset));
    }

  public:
    InputIterator() = delete;
    InputIterator(const InputIterator &) = default;
    InputIterator(InputIterator &&) noexcept = default;

    [[nodiscard]] bool hasNext() const noexcept {
        return input_->availableBytes() >= EachSize;
    }

    T next() {
        ++index_;
        return deserialize<T>(input_->get());
    }

    template <
        typename Functor,
        typename Result = std::invoke_result_t<Functor, T &&, std::uint32_t>
    >
    std::vector<Result> collectWith(Functor functor, std::size_t collectionSizeLimit = 0) {
        std::vector<Result> collection;
        std::size_t objectsInStream;
        {
            auto bytes = input_->availableBytes();
            assert(bytes % EachSize == 0);
            objectsInStream = bytes / EachSize;
        }
        std::size_t objectsToRead;
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
        for (std::size_t objectI = 0; objectI < objectsToRead; ++objectI) {
            collection.push_back(functor(deserialize<T>(iss), index_++));
        }
        return collection;
    }

    std::vector<T> collect(std::uint32_t collectionSizeLimit = 0) {
        return collectWith([](auto &&x, std::uint32_t) { return x; }, collectionSizeLimit);
    }

    std::unique_ptr<InputStream> input_;
    std::uint32_t index_ = 0;
};

} // supermap::io
