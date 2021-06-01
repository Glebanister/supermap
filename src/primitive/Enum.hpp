#pragma once

#include "io/SerializeHelper.hpp"
#include "io/ShallowSerializer.hpp"

namespace supermap {

template <typename T>
struct Enum {
    Enum() = delete;

    Enum(T &&e, std::uint32_t i)
        : elem(std::move(e)), index(i) {}

    T elem{};
    std::uint32_t index{};

    Enum &operator=(const Enum &) = default;
    Enum &operator=(Enum &&) noexcept = default;
    Enum(const Enum &) = default;
    Enum(Enum &&) noexcept = default;

    bool operator==(const Enum<T> &other) const {
        return elem == other.elem && index == other.index;
    }

    bool operator<(const Enum<T> &other) const {
        return elem < other.elem;
    }
};

namespace io {

template <typename T>
struct SerializeHelper<Enum<T>> : Serializable<true> {
    static void serialize(const Enum<T> &en, std::ostream &os) {
        io::serialize(en.elem, os);
        ShallowSerializer<std::uint32_t>::serialize(en.index, os);
    }
};

template <typename T>
struct DeserializeHelper<Enum<T>>
    : Deserializable<true> {
    static Enum<T> deserialize(std::istream &is) {
        return {
            io::deserialize<T>(is),
            ShallowDeserializer<std::uint32_t>::deserialize(is)
        };
    }
};

template <typename T>
struct FixedDeserializedSizeRegister<Enum<T>> : FixedDeserializedSize<
    FixedDeserializedSizeRegister<T>::exactDeserializedSize + sizeof(std::uint32_t)> {
};

} // io

} // supermap
