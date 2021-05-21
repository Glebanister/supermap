#pragma once

#include "KeyValueStorage.hpp"
#include "StorageFilter.hpp"
#include "MemTable.hpp"

namespace supermap {

class Supermap : public KeyValueStorage {
  public:
    explicit Supermap(std::size_t memTableMaxCapacity, double diskDataStorageMaxOccupancy);

    void add(const std::string &key, std::string &&value) override;

    void remove(const std::string &key) override;

    bool containsKey(const std::string &key) override;

    std::string getValue(const std::string &key) override;

  private:
    std::unique_ptr<StorageFilter> filter_;
    std::unique_ptr<MemTable> memTable_;
};

} // supermap
