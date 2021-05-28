#pragma once

#include <array>

#include "io/SerializeHelper.hpp"
#include "io/StackMemorySerializer.hpp"

#include "Key.hpp"

namespace supermap {

template <std::size_t KeyLen>
struct KeyIndex {
    KeyIndex() = default;

    const Key<KeyLen> key{};
    const std::uint64_t index{};

    bool operator==(const KeyIndex &other) const {
        return key == other.key && index == other.index;
    }
};

namespace io {

template <std::size_t KeyLen>
struct SerializeHelper<KeyIndex<KeyLen>> : StackMemorySerializer<KeyIndex<KeyLen>> {};

template <std::size_t KeyLen>
struct DeserializeHelper<KeyIndex<KeyLen>> : StackMemoryDeserializer<KeyIndex<KeyLen>> {};

} // io

} // supermap
