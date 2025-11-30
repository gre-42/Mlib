#pragma once
#include <string>

namespace Mlib {

enum class PoseInterpolationMode {
    UNDEFINED,
    DISABLED,
    ENABLED,
    ENABLED_WO_CHECKS
};

PoseInterpolationMode pose_interpolation_mode_from_string(const std::string& s);

}
