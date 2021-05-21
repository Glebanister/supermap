#include "Supermap.hpp"
#include "BloomFilter.hpp"
#include "BST.hpp"

namespace supermap {

Supermap::Supermap(std::size_t memTableMaxCapacity, double diskDataStorageMaxOccupancy)
    : filter_(new BloomFilter()),
      memTable_(std::make_unique<MemTable>(std::make_unique<BST>(),
                                           std::make_unique<DiskDataStorage>(diskDataStorageMaxOccupancy),
                                           std::make_unique<DiskIndex>(std::vector<KeyOffset>()),
                                           memTableMaxCapacity)) {}

void Supermap::add(const std::string &key, std::string &&value) {
    filter_->addKey(key);
    memTable_->add(key, std::move(value));
}

void Supermap::remove(const std::string &key) {
    memTable_->remove(key);
}

bool Supermap::containsKey(const std::string &key) {
    if (!filter_->mightContain(key)) {
        return false;
    }
    return memTable_->containsKey(key);
}

std::string Supermap::getValue(const std::string &key) {
    if (!filter_->mightContain(key)) {
        return "";
    }
    return memTable_->getValue(key);
}

} // supermap
