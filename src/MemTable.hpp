#pragma once

#include <memory>

#include "KeyValueStorage.hpp"
#include "DiskDataStorage.hpp"
#include "DiskIndex.hpp"

namespace supermap {

class MemTable : public KeyValueStorage {
  public:
    explicit MemTable(std::unique_ptr<KeyValueStorage> &&innerStorage,
                      std::unique_ptr<DiskDataStorage> &&diskDataStorage,
                      std::unique_ptr<DiskIndex> &&diskIndex,
                      const std::size_t maxCapacity)
        : maxCapacity_(maxCapacity),
          innerStorage_(std::move(innerStorage)),
          diskDataStorage_(std::move(diskDataStorage)),
          diskIndex_(std::move(diskIndex)) {}

    [[nodiscard]] std::size_t getMaximumCapacity() const noexcept;

    void add(const std::string &key, std::string&& value) override;

    void remove(const std::string &key) override;

    bool containsKey(const std::string &key) override;

    std::string getValue(const std::string &key) override;

  protected:
    void flushToDisk();

    const std::size_t maxCapacity_;
    std::unique_ptr<KeyValueStorage> innerStorage_;
    std::unique_ptr<DiskDataStorage> diskDataStorage_;
    std::unique_ptr<DiskIndex> diskIndex_;
};

} // supermap
