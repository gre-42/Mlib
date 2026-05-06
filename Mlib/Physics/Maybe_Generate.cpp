#include "Maybe_Generate.hpp"
#include <Mlib/Memory/Float_To_Integral.hpp>

using namespace Mlib;

MaybeGenerate::MaybeGenerate()
    : lifetime_{ 0.f }
{}

MaybeGenerate::~MaybeGenerate() = default;

void MaybeGenerate::advance_time(float dt) {
    lifetime_ += dt;
}

uint32_t MaybeGenerate::operator()(float generation_dt)
{
    if (lifetime_ >= generation_dt) {
        auto res = std::floor(lifetime_ / generation_dt);
        lifetime_ -= res * generation_dt;
        return float_to_integral<uint32_t>(res);
    }
    return 0;
}
