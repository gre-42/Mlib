#pragma once
#include <Mlib/Iterator/Generator.hpp>
#include <vector>

namespace Mlib {

template <class T>
using TextureExpression1 = std::vector<T>;

template <class T>
using TextureExpression2 = std::vector<TextureExpression1<T>>;

template <class T>
Generator<const T&> flat_iterable(const TextureExpression2<T>& e2) {
    for (const auto& e : e2) {
        for (const auto& t : e) {
            co_yield t;
        }
    }
}

template <class T>
Generator<T&> flat_iterable(TextureExpression2<T>& e2) {
    for (auto& e : e2) {
        for (auto& t : e) {
            co_yield t;
        }
    }
}

}
