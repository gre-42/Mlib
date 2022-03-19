#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Physics/Units.hpp>

namespace Mlib {

static const float gravity_magnitude = 9.8f * meters / (s * s);
static const FixedArray<float, 3> gravity_direction{ 0.f, -1.f, 0.f };
static const FixedArray<float, 3> gravity_vector = gravity_direction * gravity_magnitude;

}
