#pragma once

#include "StorageItemRegister.hpp"

namespace supermap {

/**
 * @brief CountingStorageItemRegister information type.
 * @tparam IndexT Type of counter.
 * @tparam AdditionalInfo Additional information type.
 */
template <typename IndexT, typename AdditionalInfo>
struct CountingStorageInfo {
    IndexT count;
    AdditionalInfo additional;
};

/**
 * @brief CountingStorageItemRegister template specialization for an empty additional data case.
 * @tparam IndexT Type of counter.
 */
template <typename IndexT>
struct CountingStorageInfo<IndexT, void> {
    IndexT count;
};

/**
 * @brief Empty register that does nothing.
 * @tparam T Registered object type.
 */
template <typename T>
class VoidRegister : public StorageItemRegister<T, void> {
  public:
    VoidRegister() noexcept = default;
    VoidRegister(VoidRegister &&) noexcept = default;
    VoidRegister(const VoidRegister &) noexcept = default;

    /**
     * @brief Registers an item in void storage.
     */
    void registerItem(const T &) override {}

    /**
     * @brief Nothing to return since it is a void storage.
     */
    void getRegisteredItemsInfo() const noexcept override {}
};

/**
 * @brief Storage item register which counts every added element to the storage.
 * @tparam T Storage content type.
 * @tparam IndexT Type of counter.
 * @tparam AdditionalInfo Type of additional register information.
 */
template <
    typename T,
    typename IndexT,
    typename InnerStorageRegister,
    typename AdditionalInfo = typename InnerStorageRegister::ItemsInfo
>
class CountingStorageItemRegister
    : public StorageItemRegister<T, CountingStorageInfo<IndexT, typename InnerStorageRegister::ItemsInfo>> {
  public:
    /**
     * @brief Creates new empty counting register.
     */
    explicit CountingStorageItemRegister()
        : innerRegister_(std::make_unique<InnerStorageRegister>()) {}

    CountingStorageItemRegister(const CountingStorageItemRegister &other)
        : count_(other.count_),
          innerRegister_(std::make_unique<InnerStorageRegister>(*(other.innerRegister_))) {}

    CountingStorageItemRegister &operator=(const CountingStorageItemRegister &other) {
        count_ = other.count_;
        innerRegister_ = std::make_unique<InnerStorageRegister>(*(other.innerRegister_));
    }

    CountingStorageItemRegister(CountingStorageItemRegister &&other) noexcept = default;
    CountingStorageItemRegister &operator=(CountingStorageItemRegister &&other) noexcept = default;

    /**
     * @brief Increments inner counter by 1 and passes @p obj to the inner register registration.
     */
    void registerItem(const T &obj) override {
        ++count_;
        innerRegister_->registerItem(obj);
    }

    /**
     * @return Pair of count of registered items and additional information.
     */
    CountingStorageInfo<IndexT, AdditionalInfo> getRegisteredItemsInfo() const noexcept override {
        if constexpr (std::is_same_v<void, AdditionalInfo>) {
            return {count_};
        } else {
            return {count_, innerRegister_->getRegisteredItemsInfo()};
        }
    }

  private:
    IndexT count_ = 0;
    std::unique_ptr<InnerStorageRegister> innerRegister_;
};

} // supermap
