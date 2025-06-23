#pragma once
#include <Mlib/Uninitialized.hpp>
#include <concepts>
#include <type_traits>
#include <utility>

namespace Mlib {

template <class T>
class DefaultUnitialized: public T {
public:
    DefaultUnitialized()
        : T(uninitialized)
    {}
    template <class... Args>
    explicit DefaultUnitialized(Args&&... v)
        : T(std::forward<Args>(v)...)
    {}
    DefaultUnitialized& operator = (const T& other) {
        T& v = *this;
        v = other;
        return *this;
    }
    DefaultUnitialized& operator = (T&& other) {
        T& v = *this;
        v = std::move(other);
        return *this;
    }
    T& base() {
        return *this;
    }
    const T& base() const {
        return *this;
    }
};

// template <class T>
//     requires ( std::is_constructible_v<T, Uninitialized> && std::is_convertible_v<T, Uninitialized> )
// static DefaultUnitialized<T> default_uninitialized_func();
// 
// template <class T>
//     requires ( !std::is_constructible_v<T, Uninitialized> || !std::is_convertible_v<T, Uninitialized> )
// static T default_uninitialized_func();
// 
// template <class T>
// using default_uninitialized_t = decltype(default_uninitialized_func<T>());

template <class T>
using default_uninitialized_t = std::conditional<
    std::is_default_constructible_v<T>,
    T,
    DefaultUnitialized<T>>::type;

// template <class T>
// using default_uninitialized_t = std::conditional<
//     std::is_constructible_v<T, Uninitialized>,
//     DefaultUnitialized<T>,
//     T>::type;

}
