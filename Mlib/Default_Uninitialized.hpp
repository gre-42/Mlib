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
    DefaultUnitialized(const T& v)
        : T(v)
    {}
    template<std::convertible_to<typename T::value_type>... Values>
    DefaultUnitialized(const Values&... values)
        : T{ values... }
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
};

template <class T>
using default_uninitialized_t = std::conditional<
    std::is_constructible_v<T, Uninitialized>,
    DefaultUnitialized<T>,
    T>::type;

}
