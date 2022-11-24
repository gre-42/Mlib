#pragma once
#include <cstddef>

namespace Mlib {

struct BaseGamepadAnalogAxisBinding {
    std::string axis;
    float sign_and_scale;
};

}
