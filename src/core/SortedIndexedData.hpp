#pragma once

#include <algorithm>

#include "IndexedData.hpp"

namespace supermap {

template <typename T>
class SortedIndexedData : public IndexedData<T> {
  public:
    explicit SortedIndexedData(std::vector<T> data,
                               std::string dataFileName,
                               std::shared_ptr<io::FileManager> manager)
        : IndexedData<T>(dataFileName, manager) {
        std::stable_sort(data.begin(), data.end());
        this->appendAll(data.cbegin(), std::unique(data.begin(), data.end()));
    }

    explicit SortedIndexedData(const std::vector<SortedIndexedData<T>>,
                               const std::string &dataFileName,
                               std::shared_ptr<io::FileManager> manager)
        : IndexedData<T>(dataFileName, manager) {
        throw NotImplementedException("Merge of many SortedIndexedData is not implemented");
    }

    SortedIndexedData(const SortedIndexedData &other) = default;
};

} // supermap
