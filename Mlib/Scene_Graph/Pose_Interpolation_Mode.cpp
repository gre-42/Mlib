#include "Pose_Interpolation_Mode.hpp"
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

PoseInterpolationMode Mlib::pose_interpolation_mode_from_string(const std::string& s) {
    if (s == "disabled") {
        return PoseInterpolationMode::DISABLED;
    } else if (s == "enabled") {
        return PoseInterpolationMode::ENABLED;
    } else {
        THROW_OR_ABORT("Unknown pose interpolation mode: \"" + s + '"');
    }
}
