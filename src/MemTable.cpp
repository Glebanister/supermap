#include "MemTable.hpp"
#include "exception/SupermapException.hpp"

namespace supermap {

void MemTable::flushToDisk() {
    throw NotImplementedException();
}

std::size_t MemTable::getMaximumCapacity() const noexcept {
    return maxCapacity_;
}
void MemTable::add(const std::string &key, std::string&& value) {
    throw NotImplementedException();
}
bool MemTable::containsKey(const std::string &key) {
    throw NotImplementedException();
}
void MemTable::remove(const std::string &key) {
    throw NotImplementedException();
}
std::string MemTable::getValue(const std::string &key) {
    throw NotImplementedException();
}

} // supermap
