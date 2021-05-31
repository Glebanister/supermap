#pragma once

#include <memory>
#include <vector>

#include "InputStream.hpp"
#include "SerializeHelper.hpp"
#include "exception/IteratorException.hpp"
#include "primitive/Enum.hpp"

namespace supermap::io {

template <typename T, typename = std::enable_if_t<DeserializeHelper<T>::isDeserializable>>
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
        return input_->availableBytes() >= DeserializeHelper<T>::minimalDeserializedSize;
    }

    T next() {
        ++index_;
        return deserialize<T>(input_->get());
    }

    template <
        typename To,
        typename Functor,
        typename = std::enable_if_t<std::is_invocable_r_v<To, Functor, T &&>>
    >
    std::vector<Enum<To>> collectWith(Functor functor, std::uint32_t collectionSizeLimit = 0) {
        std::vector<Enum<To>> collection;
        if (collectionSizeLimit != 0) {
            collection.reserve(collectionSizeLimit);
        }
        while (hasNext()) {
            if (collectionSizeLimit != 0 && collection.size() == collectionSizeLimit) {
                break;
            }
            collection.push_back(Enum{functor(next()), index_ - 1});
        }
        return collection;
    }

    std::vector<Enum<T>> collect(std::uint32_t collectionSizeLimit = 0) {
        return collectWith([](auto &&x) { return x; }, collectionSizeLimit);
    }

    std::unique_ptr<InputStream> input_;
    std::uint32_t index_ = 0;
};

} // supermap::io
