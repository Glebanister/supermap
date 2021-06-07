#pragma once

#include "core/KeyValueStorage.hpp"
#include "core/RemovableKvs.hpp"
#include "primitive/Bounds.hpp"
#include "io/SerializeHelper.hpp"
#include "exception/SupermapException.hpp"

namespace supermap {

/**
 * @brief Optional value container.
 * @tparam Value Maybe contained value.
 */
template <typename Value>
struct MaybeRemovedValue {
    Value value;
    bool removed;

    /**
     * @brief Parses string @p MaybeValue representation.
     * @param str String representation.
     * @return Non-removed @p MaybeValue.
     */
    static MaybeRemovedValue fromString(const std::string &str) {
        return {Value::fromString(str), false};
    }

    [[nodiscard]] std::string toString() const {
        if (removed) {
            throw KeyException("Value is removed");
        }
        return value.toString();
    }
};

namespace io {

/**
 * @brief @p SerializeHelper template specialization for @p MaybeRemovedValue.
 * @tparam T Content type.
 */
template <typename T>
struct SerializeHelper<MaybeRemovedValue<T>> : Serializable<true> {
    static void serialize(const MaybeRemovedValue<T> &val, std::ostream &os) {
        io::serialize(val.value, os);
        io::serialize(val.removed, os);
    }
};

/**
 * @brief @p DeserializeHelper template specialization for @p MaybeRemovedValue.
 * @tparam T Content type.
 */
template <typename T>
struct DeserializeHelper<MaybeRemovedValue<T>> : Deserializable<true> {
    static MaybeRemovedValue<T> deserialize(std::istream &is) {
        return {
            io::deserialize<T>(is),
            io::deserialize<bool>(is)
        };
    }
};

/**
 * @brief @p FixedDeserializedSizeRegister template specialization for @p MaybeRemovedValue.
 * @tparam T Content type.
 */
template <typename T>
struct FixedDeserializedSizeRegister<MaybeRemovedValue<T>> : FixedDeserializedSize<
    FixedDeserializedSizeRegister<T>::exactDeserializedSize
        + FixedDeserializedSizeRegister<bool>::exactDeserializedSize
> {
};

} // io

template <
    typename Key,
    typename Value,
    typename Size
>
class DefaultRemovableKvs : public RemovableKvs<Key, Value, Size> {
  public:
    explicit DefaultRemovableKvs(std::unique_ptr<KeyValueStorage<Key,
                                                                 MaybeRemovedValue<Value>,
                                                                 Size>> &&innerStorage = nullptr)
        : storageOfMaybeRemoved_(std::move(innerStorage)) {}

    void setInnerStorage(std::unique_ptr<KeyValueStorage<Key, MaybeRemovedValue<Value>, Size>> &&inner) {
        storageOfMaybeRemoved_ = std::move(inner);
    }

    void add(const Key &key, Value &&value) override {
        assert(storageOfMaybeRemoved_);
        storageOfMaybeRemoved_->add(key, MaybeRemovedValue<Value>{std::move(value), false});
    }

    std::optional<Value> getValue(const Key &key) override {
        assert(storageOfMaybeRemoved_);
        std::optional<MaybeRemovedValue<Value>> optMaybeRemoved = storageOfMaybeRemoved_->getValue(key);
        if (!optMaybeRemoved.has_value()) {
            return std::nullopt;
        }
        MaybeRemovedValue<Value> maybeInnerValue = optMaybeRemoved.value();
        if (maybeInnerValue.removed) {
            return std::nullopt;
        }
        return std::optional{maybeInnerValue.value};
    }

    Size getSize() const override {
        assert(storageOfMaybeRemoved_);
        return storageOfMaybeRemoved_->getSize();
    }

    void remove(const Key &key) override {
        assert(storageOfMaybeRemoved_);
        storageOfMaybeRemoved_->add(key, MaybeRemovedValue<Value>{Value{}, true});
    }

  private:
    std::unique_ptr<KeyValueStorage<Key, MaybeRemovedValue<Value>, Size>> storageOfMaybeRemoved_;
};

} // supermap
