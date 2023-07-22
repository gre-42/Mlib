#pragma once
#include <Mlib/Array/Fixed_Array.hpp>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
enum class WrapMode;

void shift_uv3(
    float period,
    FixedArray<float, 2>& u0,
    FixedArray<float, 2>& u1,
    FixedArray<float, 2>& u2,
    const FixedArray<WrapMode, 2>& wrap_mode);

template <class TPos>
struct UvShifter3 {
    UvShifter3(
        TPos period,
        const FixedArray<TPos, 2>& u0,
        const FixedArray<TPos, 2>& u1,
        const FixedArray<TPos, 2>& u2,
        const FixedArray<WrapMode, 2>& wrap_mode);
    FixedArray<float, 2> u0;
    FixedArray<float, 2> u1;
    FixedArray<float, 2> u2;
};

void shift_uv4(
    float period,
    FixedArray<float, 2>& u0,
    FixedArray<float, 2>& u1,
    FixedArray<float, 2>& u2,
    FixedArray<float, 2>& u3,
    const FixedArray<WrapMode, 2>& wrap_mode);

template <class TPos>
struct UvShifter4 {
    UvShifter4(
        TPos period,
        const FixedArray<TPos, 2>& u0,
        const FixedArray<TPos, 2>& u1,
        const FixedArray<TPos, 2>& u2,
        const FixedArray<TPos, 2>& u3,
        const FixedArray<WrapMode, 2>& wrap_mode);
    FixedArray<float, 2> u0;
    FixedArray<float, 2> u1;
    FixedArray<float, 2> u2;
    FixedArray<float, 2> u3;
};

}
