#pragma once
#include <Mlib/Array/Fixed_Array.hpp>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;

struct UvShifter {
    UvShifter(
        const FixedArray<double, 2>& u0,
        const FixedArray<double, 2>& u1,
        const FixedArray<double, 2>& u2);
    FixedArray<float, 2> u0;
    FixedArray<float, 2> u1;
    FixedArray<float, 2> u2;
};

}
