#include "Pose_Interpolation_Mode.hpp"
#include <map>
#include <stdexcept>

using namespace Mlib;

PoseInterpolationMode Mlib::pose_interpolation_mode_from_string(const std::string& s) {
    static const std::map<std::string, PoseInterpolationMode> m{
        {"disabled", PoseInterpolationMode::DISABLED},
        {"enabled", PoseInterpolationMode::ENABLED},
        {"enabled_wo_checks", PoseInterpolationMode::ENABLED_WO_CHECKS}
    };
    auto it = m.find(s);
    if (it == m.end()) {
        throw std::runtime_error("Unknown pose interpolation mode: \"" + s + '"');
    }
    return it->second;
}
