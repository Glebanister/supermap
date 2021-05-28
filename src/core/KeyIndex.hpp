#pragma once

#include <array>

#include "io/SerializeHelper.hpp"
#include "io/StackMemorySerializer.hpp"

#include "Key.hpp"

namespace supermap {

template <std::size_t KeyLen>
struct KeyIndex {
    const Key<KeyLen> key;
    const std::uint64_t index;
};

namespace io {

template <std::size_t KeyLen>
struct SerializeHelper<KeyIndex<KeyLen>> : StackMemorySerializer<KeyIndex<KeyLen>> {};

} // io

} // supermap
