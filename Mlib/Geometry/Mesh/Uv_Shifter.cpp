#include "Uv_Shifter.hpp"
#include <Mlib/Geometry/Material/Wrap_Mode.hpp>

using namespace Mlib;

template <class TPos>
void UvShifter3<TPos>::shift(
    TPos period,
    FixedArray<TPos, 2>& u0,
    FixedArray<TPos, 2>& u1,
    FixedArray<TPos, 2>& u2,
    FixedArray<WrapMode, 2> wrap_mode)
{
    UvShifter3<TPos> uv_shifter3{period, u0, u1, u2};
    for (size_t i = 0; i < 2; ++i) {
        if (wrap_mode(i) == WrapMode::REPEAT) {
            u0(i) = uv_shifter3.u0(i);
            u1(i) = uv_shifter3.u1(i);
            u2(i) = uv_shifter3.u2(i);
        }
    }
}

template <class TPos>
UvShifter3<TPos>::UvShifter3(
    TPos period,
    const FixedArray<TPos, 2>& u0,
    const FixedArray<TPos, 2>& u1,
    const FixedArray<TPos, 2>& u2)
{
    auto center = (u0 + u1 + u2) / TPos(3);
    center = center TEMPLATEV applied<TPos>([&period](TPos v){ return std::round(v / period) * period; });
    this->u0 = (u0 - center) TEMPLATEV casted<float>();
    this->u1 = (u1 - center) TEMPLATEV casted<float>();
    this->u2 = (u2 - center) TEMPLATEV casted<float>();
}

template <class TPos>
void UvShifter4<TPos>::shift(
    TPos period,
    FixedArray<TPos, 2>& u0,
    FixedArray<TPos, 2>& u1,
    FixedArray<TPos, 2>& u2,
    FixedArray<TPos, 2>& u3,
    FixedArray<WrapMode, 2> wrap_mode)
{
    UvShifter4<TPos> uv_shifter4{period, u0, u1, u2, u3};
    for (size_t i = 0; i < 2; ++i) {
        if (wrap_mode(i) == WrapMode::REPEAT) {
            u0(i) = uv_shifter4.u0(i);
            u1(i) = uv_shifter4.u1(i);
            u2(i) = uv_shifter4.u2(i);
            u3(i) = uv_shifter4.u3(i);
        }
    }
}

template <class TPos>
UvShifter4<TPos>::UvShifter4(
    TPos period,
    const FixedArray<TPos, 2>& u0,
    const FixedArray<TPos, 2>& u1,
    const FixedArray<TPos, 2>& u2,
    const FixedArray<TPos, 2>& u3)
{
    auto center = (u0 + u1 + u2 + u3) / TPos(4);
    center = center TEMPLATEV applied<TPos>([&period](TPos v){ return std::round(v / period) * period; });
    this->u0 = (u0 - center) TEMPLATEV casted<float>();
    this->u1 = (u1 - center) TEMPLATEV casted<float>();
    this->u2 = (u2 - center) TEMPLATEV casted<float>();
    this->u3 = (u3 - center) TEMPLATEV casted<float>();
}

namespace Mlib {
    template struct UvShifter3<float>;
    template struct UvShifter3<double>;
    template struct UvShifter4<float>;
    template struct UvShifter4<double>;
}
