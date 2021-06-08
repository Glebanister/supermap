#pragma once

#include "exception/SupermapException.hpp"

#include <optional>

namespace supermap {

/**
 * @brief An abstract key-value storage.
 * @tparam Key type of key.
 * @tparam Value type of value.
 * @tparam IndexT type of storage size. Could be non-integral,
 * if, for example, storage size is not easily calculable, then
 * Bounds could be a replacement.
 */
template <typename Key, typename Value, typename IndexT>
class KeyValueStorage {
  public:
    /**
     * @brief Add file to the storage. More relevant key value
     * overwrites less relevant.
     * @param key New key.
     * @param value Value of the @p key.
     */
    virtual void add(const Key &key, Value &&value) = 0;

    /**
     * @brief Reads value associated with the given @p key.
     * If there is no value associated with @p key, @p std::nullopt is returned.
     * @param key Key to get value.
     * @return Non-empty @p std::optional<Value> is value associated with the given @p key
     * exists, @p std::nullopt otherwise.
     */
    virtual std::optional<Value> getValue(const Key &key) = 0;

    /**
     * @brief Checks if storage contains @p key.
     * @param key Key to find.
     * @return If there is any value associated with the given key.
     */
    bool contains(const Key &key) {
        return getValue(key).has_value();
    }

    Value getValueNonSafe(const Key &key) {
        std::optional<Value> optValue = getValue(key);
        if (!optValue.has_value()) {
            throw KeyException(key.toString(), "Not found");
        }
        return optValue.value();
    }

    /**
     * @brief Remove @p key from storage.
     * @param key Key to remove.
     */
    virtual void remove(const Key &) {
        throw NotImplementedException("Remove for abstract KeyValueStorage");
    }

    /**
     * @return Size of the storage.
     */
    virtual IndexT getUpperSizeBound() const = 0;

    virtual ~KeyValueStorage() = default;
};

} // supermap
