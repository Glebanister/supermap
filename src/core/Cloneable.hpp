#pragma once

#include <memory>

namespace supermap {

/**
 * @brief Object which can produce cloned versions.
 * @tparam T @p conteined object type.
 */
template <typename T>
class Cloneable {
  public:
    /**
     * @return Ownership of cloned version of @p T container.
     */
    virtual std::unique_ptr<T> clone() const = 0;

    virtual ~Cloneable() = default;
};

} // supermap
