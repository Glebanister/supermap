#pragma once

#include <map>

#include "ExtractibleKeyValueStorage.hpp"

namespace supermap {

template <typename Key, typename Value, typename IndexT>
class BST : public ExtractibleKeyValueStorage<Key, Value, IndexT> {
  public:
    void add(const Key &key, Value &&value) override {
        map_[key] = std::move(value);
    }

    bool containsKey(const Key &key) override {
        return map_.find(key) != map_.end();
    }

    const Value &getValue(const Key &key) override {
        if (!containsKey(key)) {
            throw IllegalArgumentException("Key is not in BST");
        }
        return map_[key];
    }

    IndexT getSize() const noexcept override {
        return static_cast<IndexT>(map_.size());
    }

    std::vector<KeyValue<Key, Value>> extract() && override {
        std::vector<KeyValue<Key, Value>> extracted;
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
