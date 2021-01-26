#pragma once
#include <type_traits>

namespace Mlib {

template <class T>
struct ScalarType;

template <class T, bool is_sclr>
struct ScalarTypeRecursion;

template <class T>
struct ScalarTypeRecursion<T, true> {
    typedef T value_type;
};

template <class T>
struct ScalarTypeRecursion<T, false> {
    typedef typename T::value_type value_type;
};

template <class T>
struct ScalarType {
    typedef typename ScalarTypeRecursion<T, std::is_scalar<T>::value>::value_type value_type;
};

}
