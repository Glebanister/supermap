#include "DiskIndex.hpp"

namespace supermap {

DiskIndex::DiskIndex(const std::vector<KeyOffset> &records) {

}

std::uint64_t DiskIndex::getOffset(const std::string &key) {
    throw NotImplementedException();
}

void DiskIndex::clear() {
    throw NotImplementedException();
}

void DiskIndex::addAll(std::vector<KeyOffset> &records) {
    throw NotImplementedException();
}

} // supermap
