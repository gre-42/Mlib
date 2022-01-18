#pragma once
#include <Mlib/Array/Fixed_Array.hpp>

namespace Mlib {

static const float gravity_magnitude = 9.8f;
static const FixedArray<float, 3> gravity_direction{ 0.f, -1.f, 0.f };
static const FixedArray<float, 3> gravity_vector = gravity_direction * gravity_magnitude;

}
