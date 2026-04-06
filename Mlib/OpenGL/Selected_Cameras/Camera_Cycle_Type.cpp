
#include "Camera_Cycle_Type.hpp"
#include <map>
#include <stdexcept>

using namespace Mlib;

CameraCycleType Mlib::camera_cycle_type_from_string(const std::string& s) {
    static const std::map<std::string, CameraCycleType> m{
        {"near", CameraCycleType::NEAR},
        {"far", CameraCycleType::FAR},
        {"spectator", CameraCycleType::SPECTATOR},
    };
    auto it = m.find(s);
    if (it == m.end()) {
        throw std::runtime_error("Unknown camera cycle type: \"" + s + '"');
    }
    return it->second;
}
