#pragma once

#include <string>

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
     * @brief Checks if storage contains @p key.
     * @param key Key to find.
     * @return If there is any value associated with the given key.
     */
    virtual bool containsKey(const Key &key) = 0;

    /**
     * @brief Reads value associated with the given @p key.
     * If there is no value associated with @p key, behavior is undefined.
     * @param key Key to get value.
     * @return Value associated with the given @p key.
     */
    virtual Value getValue(const Key &key) = 0;

    /**
     * @return Size of the storage.
     */
    virtual IndexT getSize() const noexcept = 0;

    virtual ~KeyValueStorage() = default;
};

} // supermap
