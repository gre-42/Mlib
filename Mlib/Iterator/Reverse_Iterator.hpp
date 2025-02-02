#pragma once
#include <iterator>

namespace Mlib {

template <typename T>
struct reversion_wrapper {
    T iterable;
    auto begin() const {
        return std::rbegin(iterable);
    }
    auto end() const {
        return std::rend(iterable);
    }
};

template <typename T>
reversion_wrapper<T> reverse (T&& iterable) { return { iterable }; }

}
