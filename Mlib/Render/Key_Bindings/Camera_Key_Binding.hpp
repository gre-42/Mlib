#pragma once
#include <Mlib/Render/Ui/Button_Press.hpp>

namespace Mlib {

enum class CameraCycleType;

struct CameraKeyBinding {
    CameraCycleType tpe;
    ButtonPress button_press;
};

}
