#include "Camera_Cycle_Type.hpp"
#include <Mlib/Throw_Or_Abort.hpp>
#include <map>

using namespace Mlib;

CameraCycleType Mlib::camera_cycle_type_from_string(const std::string& s) {
    static const std::map<std::string, CameraCycleType> m{
        {"near", CameraCycleType::NEAR},
        {"far", CameraCycleType::FAR},
        {"tripod", CameraCycleType::TRIPOD},
    };
    auto it = m.find(s);
    if (it == m.end()) {
        THROW_OR_ABORT("Unknown camera cycle type: \"" + s + '"');
    }
    return it->second;
}
