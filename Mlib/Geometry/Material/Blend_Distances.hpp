#pragma once
#include <Mlib/Math/Orderable_Fixed_Array.hpp>

namespace Mlib {

// Default: No fade-in (0, 0), no fade-out (INFINITY, INFINITY)
static const OrderableFixedArray<float, 4> default_linear_distances{ 0.f, 0.f, float(INFINITY), float(INFINITY) };
static const OrderableFixedArray<float, 4> default_linear_cosines{ -1.f, -1.f, 1.f, 1.f };
static const OrderableFixedArray<float, 2> default_step_distances{ 0.f, float(INFINITY) };

}
