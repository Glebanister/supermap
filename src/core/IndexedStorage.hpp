#pragma once

#include <cstdint>
#include <type_traits>

#include "io/FileManager.hpp"

namespace supermap {

template <typename T, typename IndexT>
class IndexedStorage {
  public:
    [[nodiscard]] virtual std::shared_ptr<io::FileManager> getFileManager() const noexcept = 0;

    virtual IndexT append(const T &item) = 0;

    virtual T get(IndexT index) const = 0;

    virtual IndexT getItemsCount() const noexcept = 0;

    virtual ~IndexedStorage() = default;
};

} // supermap
