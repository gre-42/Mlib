#pragma once

namespace Mlib {

enum class ScreenUnits;

struct TapButtonState {
    float left;
    float right;
    float bottom;
    float top;
    ScreenUnits units;
    bool pressed;
};

}
