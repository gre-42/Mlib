#pragma once
#include <Mlib/Render/Ui/Button_Press.hpp>
#include <string>

namespace Mlib {

enum class CameraCycleType;

struct CameraKeyBinding {
    std::string id;
    std::string role;
    CameraCycleType tpe;
    ButtonPress button_press;
};

}
