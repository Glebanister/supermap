#pragma once

#include <vector>

namespace supermap::io {

template <typename From, typename To, typename Functor>
class FunctorIterator;

template <typename T>
class Iterator {
  public:
    virtual T next() = 0;

    [[nodiscard]] virtual bool hasNext() const noexcept = 0;

    virtual ~Iterator() = default;

    template <typename To, typename Functor>
    auto map(Functor f) {
        return FunctorIterator<T, To, Functor>(std::move(f), *this);
    }

    template <
        typename CollectionT,
        typename Functor,
        typename = std::enable_if_t<std::is_invocable_r_v<void, Functor, CollectionT &, T &&>>,
        typename = std::enable_if_t<std::is_default_constructible_v<CollectionT>>
    >
    CollectionT collect(Functor f) {
        CollectionT collection;
        while (hasNext()) {
            f(collection, next());
        }
        return collection;
    }

    std::vector<T> collectToVector() {
        return collect<std::vector<T>>([](std::vector<T> &v, T &&elem) { v.push_back(std::move(elem)); });
    }
};

template <
    typename From,
    typename To,
    typename Functor
>
class FunctorIterator : public Iterator<To> {
  public:
    explicit FunctorIterator(Functor f, Iterator<From> &parent)
        : func_(std::move(f)), parentIterator_(parent) {}

    std::enable_if_t<std::is_invocable_r_v<To, Functor, From &&>, To> next() override {
        return func_(parentIterator_.next());
    }

    [[nodiscard]] bool hasNext() const noexcept override {
        return parentIterator_.hasNext();
    }

  private:
    Functor func_;
    Iterator<From> &parentIterator_;
};

} // supermap::io
