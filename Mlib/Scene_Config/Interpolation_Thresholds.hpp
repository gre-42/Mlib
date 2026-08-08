#pragma once
#include <Mlib/Physics/Units.hpp>

namespace Mlib {

static constexpr const float INTERPOLATION_ERROR_DISTANCE = 1.f * seconds * 200.f * kph;
static constexpr const float REMOTE_INTERPOLATION_JUMP_DISTANCE = 0.4f * seconds * 200.f * kph;
static constexpr const float REMOTE_INTERPOLATION_HALFLIFE = 0.05f * seconds;
static constexpr const float REMOTE_AIR_DAMPING_HALFLIFE = 0.2f * seconds;

}
