#pragma once

#include "io/SerializeHelper.hpp"
#include "io/ShallowSerializer.hpp"

namespace supermap {

template <typename T, typename IndexT>
struct Enum {
    Enum() = delete;

    Enum(T &&e, IndexT i)
        : elem(std::move(e)), index(i) {}

    T elem{};
    IndexT index{};

    Enum &operator=(const Enum &) = default;
    Enum &operator=(Enum &&) noexcept = default;
    Enum(const Enum &) = default;
    Enum(Enum &&) noexcept = default;

    bool operator==(const Enum<T, IndexT> &other) const {
        return elem == other.elem;
    }

    bool equals(const Enum<T, IndexT> &other) const {
        return elem == other.elem && index == other.index;
    }

    bool operator<(const Enum<T, IndexT> &other) const {
        return elem < other.elem;
    }
};

namespace io {

template <typename T, typename IndexT>
struct SerializeHelper<Enum<T, IndexT>> : Serializable<true> {
    static void serialize(const Enum<T, IndexT> &en, std::ostream &os) {
        io::serialize(en.elem, os);
        ShallowSerializer<IndexT>::serialize(en.index, os);
    }
};

template <typename T, typename IndexT>
struct DeserializeHelper<Enum<T, IndexT>>
    : Deserializable<true> {
    static Enum<T, IndexT> deserialize(std::istream &is) {
        return {
            io::deserialize<T>(is),
            ShallowDeserializer<IndexT>::deserialize(is)
        };
    }
};

template <typename T, typename IndexT>
struct FixedDeserializedSizeRegister<Enum<T, IndexT>> : FixedDeserializedSize<
    FixedDeserializedSizeRegister<T>::exactDeserializedSize + sizeof(IndexT)> {
};

} // io

} // supermap
