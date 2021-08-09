#pragma once

namespace supermap {

/**
 * @brief A tag, that allows creation of 'like this' instance.
 * @tparam T
 */
template <typename T>
class AllowCreateLikeThis {
  public:
    /**
     * @return 'Like this' copy of this object. It could be not
     * an absolute copy. For example, for storages, it could
     * be storage with the same parameters, but empty.
     */
    virtual std::unique_ptr<T> createLikeThis() const = 0;
};

} // namespace supermap
