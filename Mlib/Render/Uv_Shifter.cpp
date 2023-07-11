#include "Uv_Shifter.hpp"

using namespace Mlib;

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
