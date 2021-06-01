#pragma once

#include "io/SerializeHelper.hpp"
#include "io/ShallowSerializer.hpp"

namespace supermap {

template <typename T>
struct Enum {
    using Index = std::uint32_t;

    Enum() = delete;

    Enum(T &&e, Index i)
        : elem(std::move(e)), index(i) {}

    T elem{};
    Index index{};

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
        ShallowSerializer<decltype(en.index)>::serialize(en.index, os);
    }
};

template <typename T>
struct DeserializeHelper<Enum<T>>
    : Deserializable<true> {
    static Enum<T> deserialize(std::istream &is) {
        return {
            io::deserialize<T>(is),
            ShallowDeserializer<typename Enum<T>::Index>::deserialize(is)
        };
    }
};

template <typename T>
struct FixedDeserializedSizeRegister<Enum<T>> : FixedDeserializedSize<
    FixedDeserializedSizeRegister<T>::exactDeserializedSize + sizeof(typename Enum<T>::Index)> {
};

} // io

} // supermap
