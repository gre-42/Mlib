#pragma once
#include <string>

namespace Mlib {

enum class CameraCycleType {
    NEAR,
    FAR,
    TRIPOD
};

CameraCycleType camera_cycle_type_from_string(const std::string& s);

}
