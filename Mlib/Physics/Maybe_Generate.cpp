#include "Maybe_Generate.hpp"

using namespace Mlib;

MaybeGenerate::MaybeGenerate()
    : lifetime_{ 0.f }
{}

MaybeGenerate::~MaybeGenerate() = default;

void MaybeGenerate::advance_time(float dt) {
    lifetime_ += dt;
}

bool MaybeGenerate::operator()(float generation_dt)
{
    if (lifetime_ > generation_dt) {
        lifetime_ = 0.f;
        return true;
    }
    return false;
}
