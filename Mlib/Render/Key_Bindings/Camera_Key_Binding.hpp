#pragma once
#include <string>

namespace Mlib {

enum class CameraCycleType;

struct CameraKeyBinding {
    std::string id;
    std::string role;
    CameraCycleType tpe;
};

}
