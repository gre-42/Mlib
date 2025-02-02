#pragma once
#include <Mlib/Nan_Initialized.hpp>
#include <concepts>

namespace Mlib {

template <class T>
class DefaultNan: public T {
public:
    DefaultNan()
        : T(nan_initialized)
    {}
    DefaultNan(const T& v)
        : T(v)
    {}
    template<std::convertible_to<typename T::value_type>... Values>
    DefaultNan(const Values&... values)
        : T{ values... }
    {}
    DefaultNan& operator = (const T& other) {
        T& v = *this;
        v = other;
        return *this;
    }
    DefaultNan& operator = (T&& other) {
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

}
