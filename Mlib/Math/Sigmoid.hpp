#pragma once
#include <Mlib/Type_Traits/Get_Scalar.hpp>

namespace Mlib {

template <class T>
static T sigmoid(T t)
{
    auto c1 = scalar_type_t<T>(1);
    // From: https://math.stackexchange.com/a/3253471/233679
    auto a = c1 / squared(c1 / t - c1);
    return c1 - c1 / (c1 + a);
}

}
