#pragma once
#include <Mlib/Physics/Units.hpp>

namespace Mlib {

static constexpr const float INTERPOLATION_ERROR_DISTANCE = 0.5f * seconds * 200.f * kph;
static constexpr const float REMOTE_INTERPOLATION_JUMP_DISTANCE = 0.2f * seconds * 200.f * kph;
static constexpr const float REMOTE_INTERPOLATION_HALFLIFE = 0.05f * seconds;

}
