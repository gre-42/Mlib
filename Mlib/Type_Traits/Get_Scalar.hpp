#pragma once
#include <Mlib/Math/Fixed_Point_Number.hpp>
#include <type_traits>

namespace Mlib {

struct IsScalarHelper {
    static const bool value = true;
};

struct IsNotScalarHelper {
    static const bool value = false;
};

template <class T>
IsNotScalarHelper is_scalar_helper(const T&);

template <class TInt, std::intmax_t denominator>
IsScalarHelper is_scalar_helper(const FixedPointNumber<TInt, denominator>&);

template <class T>
IsScalarHelper is_scalar_helper(const T&) requires std::is_scalar_v<T>;

template <class T>
struct ScalarType;

template <class T, bool is_sclr>
struct ScalarTypeRecursion;

template <class T>
struct ScalarTypeRecursion<T, true> {
    using value_type = T;
};

template <class T>
struct ScalarTypeRecursion<T, false> {
    using value_type = T::value_type;
};

template <class T>
struct ScalarType {
    static const T& func();
    using value_type = ScalarTypeRecursion<T, decltype(is_scalar_helper(func()))::value>::value_type;
};

template <class T>
using scalar_type_t = typename ScalarType<T>::value_type;

}
