#include "Uv_Shifter.hpp"
#include <Mlib/Geometry/Material/Wrap_Mode.hpp>
#include <Mlib/Geometry/Texture/Uv_Atlas_Tolerance.hpp>

using namespace Mlib;

void Mlib::shift_uv3(
    float period,
    FixedArray<float, 2>& u0,
    FixedArray<float, 2>& u1,
    FixedArray<float, 2>& u2,
    WrapMode wrap_mode,
    size_t i)
{
    auto offset = (wrap_mode == WrapMode::REPEAT)
        ? std::round((u0(i) + u1(i) + u2(i)) / 3.f / period) * period
        : std::floor((std::min({u0(i), u1(i), u2(i)}) - UV_ATLAS_MIN) / period) * period;
    u0(i) -= offset;
    u1(i) -= offset;
    u2(i) -= offset;
}

template <class TPos>
UvShifter3<TPos>::UvShifter3(
    TPos period,
    const FixedArray<TPos, 2>& u0,
    const FixedArray<TPos, 2>& u1,
    const FixedArray<TPos, 2>& u2,
    const FixedArray<WrapMode, 2>& wrap_mode)
    : u0{ uninitialized }
    , u1{ uninitialized }
    , u2{ uninitialized }
{
    for (size_t i = 0; i < 2; ++i) {
        auto offset = (wrap_mode(i) == WrapMode::REPEAT)
            ? std::round((u0(i) + u1(i) + u2(i)) / TPos(3) / period) * period
            : std::floor((std::min({u0(i), u1(i), u2(i)}) - UV_ATLAS_MIN) / period) * period;
        this->u0(i) = float(u0(i) - offset);
        this->u1(i) = float(u1(i) - offset);
        this->u2(i) = float(u2(i) - offset);
    }
}

template <class TPos>
UvShifter4<TPos>::UvShifter4(
    TPos period,
    const FixedArray<TPos, 2>& u0,
    const FixedArray<TPos, 2>& u1,
    const FixedArray<TPos, 2>& u2,
    const FixedArray<TPos, 2>& u3,
    const FixedArray<WrapMode, 2>& wrap_mode)
    : u0{ uninitialized }
    , u1{ uninitialized }
    , u2{ uninitialized }
    , u3{ uninitialized }
{
    for (size_t i = 0; i < 2; ++i) {
        auto offset = (wrap_mode(i) == WrapMode::REPEAT)
            ? std::round((u0(i) + u1(i) + u2(i) + u3(i)) / TPos(4) / period) * period
            : std::floor((std::min({u0(i), u1(i), u2(i), u3(i)}) - UV_ATLAS_MIN) / period) * period;
        this->u0(i) = float(u0(i) - offset);
        this->u1(i) = float(u1(i) - offset);
        this->u2(i) = float(u2(i) - offset);
        this->u3(i) = float(u3(i) - offset);
    }
}

namespace Mlib {
    template struct UvShifter3<float>;
    template struct UvShifter3<double>;
    template struct UvShifter4<float>;
    template struct UvShifter4<double>;
}
