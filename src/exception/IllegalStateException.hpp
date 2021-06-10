#pragma once

#include "SupermapException.hpp"

namespace supermap {

/**
 * @brief Thrown when the object has illegal state.
 */
    class IllegalStateException : public SupermapException {
    public:
        explicit IllegalStateException(const std::string &message);
    };

} // supermap
