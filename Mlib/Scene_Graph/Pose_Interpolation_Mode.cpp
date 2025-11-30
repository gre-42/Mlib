#include "Pose_Interpolation_Mode.hpp"
#include <Mlib/Throw_Or_Abort.hpp>
#include <map>

using namespace Mlib;

PoseInterpolationMode Mlib::pose_interpolation_mode_from_string(const std::string& s) {
    static const std::map<std::string, PoseInterpolationMode> m{
        {"disabled", PoseInterpolationMode::DISABLED},
        {"enabled", PoseInterpolationMode::ENABLED},
        {"enabled_wo_checks", PoseInterpolationMode::ENABLED_WO_CHECKS}
    };
    auto it = m.find(s);
    if (it == m.end()) {
        THROW_OR_ABORT("Unknown pose interpolation mode: \"" + s + '"');
    }
    return it->second;
}
