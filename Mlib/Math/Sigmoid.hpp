#pragma once
#include <Mlib/Math/Pow.hpp>
#include <Mlib/Type_Traits/Get_Scalar.hpp>

namespace Mlib {

// From: https://math.stackexchange.com/a/3253471/233679
// See also the comment from Dr. Timofey Prodanov,
// Dec 21, 2020 at 10:51
template <class X>
static X sigmoid(X x)
{
    auto c1 = scalar_type_t<X>(1);
    auto a = squared(c1 / x - c1);
    return c1 / (c1 + a);
}

// From: https://math.stackexchange.com/a/3253471/233679
// See also the comment from Dr. Timofey Prodanov,
// Dec 21, 2020 at 10:51
template <class X, class K>
static X sigmoid(const X& x, const K& k)
{
    auto c1 = scalar_type_t<X>(1);
    auto a = pow(c1 / x - c1, k);
    return c1 / (c1 + a);
}

// From: https://math.stackexchange.com/a/4798714/233679
template <class X, class T, class K>
static X sigmoid(const X& x, const T& t, const K& k)
{
    auto c1 = scalar_type_t<X>(1);
    auto a = pow(pow(x, std::log((T)2) / std::log(t)) - c1, k);
    return c1 / (c1 + a);
}


}
