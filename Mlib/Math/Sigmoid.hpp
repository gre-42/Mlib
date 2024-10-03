#pragma once
#include <Mlib/Math/Pow.hpp>
#include <Mlib/Type_Traits/Get_Scalar.hpp>

namespace Mlib {

// From: https://math.stackexchange.com/a/3253471/233679
template <class T>
static T sigmoid(T t)
{
    auto c1 = scalar_type_t<T>(1);
    auto a = c1 / squared(c1 / t - c1);
    return c1 - c1 / (c1 + a);
}

// From: https://math.stackexchange.com/a/3253471/233679
// See also the comment from Dr. Timofey Prodanov,
// Dec 21, 2020 at 10:51
template <class T>
static T inv_sigmoid(T t)
{
    auto c1 = scalar_type_t<T>(1);
    auto a = squared(c1 / t - c1);
    return c1 / (c1 + a);
}

// From: https://math.stackexchange.com/a/3253471/233679
template <class T, class TK>
static T sigmoid(const T& t, const TK& k)
{
    auto c1 = scalar_type_t<T>(1);
    auto a = c1 / pow(c1 / t - c1, k);
    return c1 - c1 / (c1 + a);
}

// From: https://math.stackexchange.com/a/3253471/233679
// See also the comment from Dr. Timofey Prodanov,
// Dec 21, 2020 at 10:51
template <class T, class TK>
static T inv_sigmoid(const T& t, const TK& k)
{
    auto c1 = scalar_type_t<T>(1);
    auto a = pow(c1 / t - c1, k);
    return c1 / (c1 + a);
}

}
