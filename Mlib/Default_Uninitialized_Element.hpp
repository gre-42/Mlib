#pragma once
#include <Mlib/Uninitialized.hpp>
#include <concepts>
#include <type_traits>
#include <utility>

namespace Mlib {

template <class T>
class DefaultUnitializedElement: public T {
public:
    DefaultUnitializedElement()
        : T(uninitialized)
    {}
    DefaultUnitializedElement(const T& v)
        : T(v)
    {}
    DefaultUnitializedElement(T&& v)
        : T(std::move(v))
    {}
    template<std::convertible_to<typename T::initializer_type>... Values>
    DefaultUnitializedElement(const Values&... values)
        : T{ values... }
    {}
    DefaultUnitializedElement& operator = (const T& other) {
        T& v = *this;
        v = other;
        return *this;
    }
    DefaultUnitializedElement& operator = (T&& other) {
        T& v = *this;
        v = std::move(other);
        return *this;
    }
};

}
