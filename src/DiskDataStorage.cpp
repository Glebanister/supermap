#include "DiskDataStorage.hpp"

namespace supermap {

void DiskDataStorage::shrink() {
    throw NotImplementedException();
}

std::string DiskDataStorage::getData(std::uint64_t offset) {
    throw NotImplementedException();
}

DiskDataStorage::AddInfo DiskDataStorage::addData(const std::string &key, std::string value) {
    throw NotImplementedException();
}

DiskDataStorage::DiskDataStorage(double maxOccupancy)
    : maxOccupancy_(maxOccupancy) {}

} // supermap
