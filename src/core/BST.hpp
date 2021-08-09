#pragma once

#include <map>
#include <memory>

#include "ExtractibleKeyValueStorage.hpp"
#include "primitive/Bounds.hpp"

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
     * @return The most relevant @p Value associated with @p key.
     */
    std::optional<Value> getValue(const Key &key) override {
        if (map_.find(key) == map_.end()) {
            return std::nullopt;
        }
        return std::optional{map_[key]};
    }

    /**
     * @return Current number of keys in BST.
     */
    IndexT getUpperSizeBound() const override {
        return map_.size();
    }

    /**
     * @brief Move all key-value pairs to the vector, emptying the BST itself.
     */
    std::vector<KeyValue<Key, Value>> extract() && override {
        std::vector<KeyValue<Key, Value>>
            extracted;
        for (auto &&[k, v] : map_) {
            extracted.emplace_back(KeyValue(std::move(k), std::move(v)));
        }
        map_.clear();
        return extracted;
    }

    void remove(const Key &key) override {
        map_.erase(key);
    }

    std::unique_ptr<ExtractibleKeyValueStorage<Key, Value, IndexT>> createLikeThis() const override {
        return std::make_unique<BST<Key, Value, IndexT>>();
    }

  private:
    std::map<Key, Value> map_;
};

} // supermap
