#include "DiskDataStorage.hpp"

namespace supermap {

void DiskDataStorage::shrink() {
    throw NotImplementedException();
}

std::string DiskDataStorage::getValue(std::uint64_t offset) {
    throw NotImplementedException();
}

DiskDataStorage::AddInfo DiskDataStorage::add(const std::string &key, std::string&& value) {
    throw NotImplementedException();
}

std::vector<KeyOffset> DiskDataStorage::getRecords() {
    throw NotImplementedException();
}

DiskDataStorage::DiskDataStorage(double maxOccupancy)
    : maxOccupancy_(maxOccupancy) {}

} // supermap
