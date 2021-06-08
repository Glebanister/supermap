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

    /**
     * @return Cloned version of void register.
     */
    std::unique_ptr<StorageItemRegister<T, void>> clone() const override {
        return std::make_unique<VoidRegister<T>>();
    }
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
    typename AdditionalInfo
>
class CountingStorageItemRegister
    : public StorageItemRegister<T, CountingStorageInfo<IndexT, AdditionalInfo>> {

  public:
    using InnerRegister = StorageItemRegister<T, AdditionalInfo>;
    using InnerRegisterSupplier = std::function<std::unique_ptr<InnerRegister>()>;

    /**
     * @brief Creates new empty counting register.
     */
    explicit CountingStorageItemRegister(InnerRegisterSupplier innerStorageSupplier)
        : innerRegister_(innerStorageSupplier()) {}

    CountingStorageItemRegister(const CountingStorageItemRegister &other)
        : count_(other.count_),
          innerRegister_(other.innerRegister_->clone()) {}

    CountingStorageItemRegister &operator=(const CountingStorageItemRegister &other) {
        count_ = other.count_;
        innerRegister_ = other.innerRegister_->clone();
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

    /**
     * @return Ownership of counting storage with the same count and inner storage state.
     */
    std::unique_ptr<StorageItemRegister<T, CountingStorageInfo<IndexT, AdditionalInfo>>>
    clone() const override {
        return std::unique_ptr<CountingStorageItemRegister<T, IndexT, AdditionalInfo>>(
            new CountingStorageItemRegister(count_,
                                            innerRegister_->clone())
        );
    }

    /**
     * @brief Reserves inner register for @p n elements.
     * @param n Reservation size.
     */
    void reserve(std::uint64_t n) override {
        innerRegister_->reserve(n);
    }

  private:
    explicit CountingStorageItemRegister(IndexT count,
                                         std::unique_ptr<StorageItemRegister<T, AdditionalInfo>> &&innerRegister)
        : count_(count), innerRegister_(std::move(innerRegister)) {}

    IndexT count_ = 0;
    std::unique_ptr<StorageItemRegister<T, AdditionalInfo>> innerRegister_;
};

} // supermap
