#include "Camera_Cycle_Type.hpp"

using namespace Mlib;

CameraCycleType Mlib::camera_cycle_type_from_string(const std::string& s) {
    if (s == "near") {
        return CameraCycleType::NEAR;
    } else if (s == "far") {
        return CameraCycleType::FAR;
    } else if (s == "tripod") {
        return CameraCycleType::TRIPOD;
    }
    throw std::runtime_error("Unknown camera cycle type: \"" + s + '"');
}
