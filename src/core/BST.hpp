#pragma once

#include <map>

#include "ExtractibleKeyValueStorage.hpp"

namespace supermap {

/**
 * @brief Implementation of Binary Search Tree with @p std::map.
 * @tparam Key key type.
 * @tparam Value value type.
 * @tparam IndexT storage index type.
 */
template <typename Key, typename Value, typename IndexT>
class BST : public ExtractibleKeyValueStorage<Key, Value, IndexT> {
  public:
    /**
     * @brief Add key-value pair to BST.
     * @param key Key to add.
     * @param value Value to add.
     */
    void add(const Key &key, Value &&value) override {
        map_[key] = std::move(value);
    }

    /**
     * @return If BST contains @p key.
     */
    bool containsKey(const Key &key) override {
        return map_.find(key) != map_.end();
    }

    /**
     * @return The most relevant @p Value associated with @p key.
     */
    Value getValue(const Key &key) override {
        if (!containsKey(key)) {
            throw IllegalArgumentException("Key is not in BST");
        }
        return map_[key];
    }

    /**
     * @return Current number of keys in BST.
     */
    IndexT getSize() const noexcept override {
        return static_cast<IndexT>(map_.size());
    }

    /**
     * @brief Move all key-value pairs to the vector, emptying the BST itself.
     */
    std::vector<KeyValue < Key, Value>> extract() && override {
        std::vector<KeyValue < Key, Value>>
        extracted;
        for (auto &&[k, v] : map_) {
            extracted.emplace_back(KeyValue(std::move(k), std::move(v)));
        }
        map_.clear();
        return extracted;
    }

  private:
    std::map<Key, Value> map_;
};

} // supermap
