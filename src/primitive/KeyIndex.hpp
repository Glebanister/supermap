#pragma once

#include <array>

#include "io/SerializeHelper.hpp"
#include "io/ShallowSerializer.hpp"

#include "Key.hpp"

namespace supermap {

template <std::size_t KeyLen>
class KeyIndex {
  public:
    KeyIndex() = default;
    KeyIndex(const KeyIndex &) = default;
    KeyIndex(KeyIndex &&) noexcept = default;
    KeyIndex &operator=(const KeyIndex &) = default;
    KeyIndex &operator=(KeyIndex &&other) noexcept = default;

    Key<KeyLen> key{};
    std::uint64_t index{};

    bool operator==(const KeyIndex &other) const noexcept {
        return key == other.key && index == other.index;
    }

    bool operator<(const KeyIndex &other) const noexcept {
        return key < other.key;
    }
};

namespace io {

template <std::size_t KeyLen>
struct SerializeHelper<KeyIndex<KeyLen>> : ShallowSerializer<KeyIndex<KeyLen>> {};

template <std::size_t KeyLen>
struct DeserializeHelper<KeyIndex<KeyLen>> : ShallowDeserializer<KeyIndex<KeyLen>> {};

} // io

} // supermap
