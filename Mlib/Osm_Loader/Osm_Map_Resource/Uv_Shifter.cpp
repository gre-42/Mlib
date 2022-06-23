#include "Uv_Shifter.hpp"

using namespace Mlib;

UvShifter::UvShifter(
    const FixedArray<double, 2>& u0,
    const FixedArray<double, 2>& u1,
    const FixedArray<double, 2>& u2)
{
    auto center = (u0 + u1 + u2) / 3.0;
    center = center.applied<double>([](double v){ return std::round(v); });
    this->u0 = (u0 - center).casted<float>();
    this->u1 = (u1 - center).casted<float>();
    this->u2 = (u2 - center).casted<float>();
}
