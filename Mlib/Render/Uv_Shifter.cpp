#include "Uv_Shifter.hpp"

using namespace Mlib;

UvShifter3::UvShifter3(
    double period,
    const FixedArray<double, 2>& u0,
    const FixedArray<double, 2>& u1,
    const FixedArray<double, 2>& u2)
{
    auto center = (u0 + u1 + u2) / 3.0;
    center = center.applied<double>([&period](double v){ return std::round(v / period) * period; });
    this->u0 = (u0 - center).casted<float>();
    this->u1 = (u1 - center).casted<float>();
    this->u2 = (u2 - center).casted<float>();
}

UvShifter4::UvShifter4(
    double period,
    const FixedArray<double, 2>& u0,
    const FixedArray<double, 2>& u1,
    const FixedArray<double, 2>& u2,
    const FixedArray<double, 2>& u3)
{
    auto center = (u0 + u1 + u2 + u3) / 4.0;
    center = center.applied<double>([&period](double v){ return std::round(v / period) * period; });
    this->u0 = (u0 - center).casted<float>();
    this->u1 = (u1 - center).casted<float>();
    this->u2 = (u2 - center).casted<float>();
    this->u3 = (u3 - center).casted<float>();
}
