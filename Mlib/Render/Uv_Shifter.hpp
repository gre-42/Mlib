#pragma once
#include <Mlib/Array/Fixed_Array.hpp>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;

struct UvShifter3 {
    UvShifter3(
        double period,
        const FixedArray<double, 2>& u0,
        const FixedArray<double, 2>& u1,
        const FixedArray<double, 2>& u2);
    FixedArray<float, 2> u0;
    FixedArray<float, 2> u1;
    FixedArray<float, 2> u2;
};

struct UvShifter4 {
    UvShifter4(
        double period,
        const FixedArray<double, 2>& u0,
        const FixedArray<double, 2>& u1,
        const FixedArray<double, 2>& u2,
        const FixedArray<double, 2>& u3);
    FixedArray<float, 2> u0;
    FixedArray<float, 2> u1;
    FixedArray<float, 2> u2;
    FixedArray<float, 2> u3;
};

}
