#pragma once
#include <Mlib/Array/Fixed_Array.hpp>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
enum class WrapMode;

template <class TPos>
struct UvShifter3 {
    static void shift(
        TPos period,
        FixedArray<TPos, 2>& u0,
        FixedArray<TPos, 2>& u1,
        FixedArray<TPos, 2>& u2,
        FixedArray<WrapMode, 2> wrap_mode);
    UvShifter3(
        TPos period,
        const FixedArray<TPos, 2>& u0,
        const FixedArray<TPos, 2>& u1,
        const FixedArray<TPos, 2>& u2);
    FixedArray<float, 2> u0;
    FixedArray<float, 2> u1;
    FixedArray<float, 2> u2;
};

template <class TPos>
struct UvShifter4 {
    static void shift(
        TPos period,
        FixedArray<TPos, 2>& u0,
        FixedArray<TPos, 2>& u1,
        FixedArray<TPos, 2>& u2,
        FixedArray<TPos, 2>& u3,
        FixedArray<WrapMode, 2> wrap_mode);
    UvShifter4(
        TPos period,
        const FixedArray<TPos, 2>& u0,
        const FixedArray<TPos, 2>& u1,
        const FixedArray<TPos, 2>& u2,
        const FixedArray<TPos, 2>& u3);
    FixedArray<float, 2> u0;
    FixedArray<float, 2> u1;
    FixedArray<float, 2> u2;
    FixedArray<float, 2> u3;
};

}
