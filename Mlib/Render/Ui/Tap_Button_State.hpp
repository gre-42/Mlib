#pragma once
#include <Mlib/Layout/IWidget.hpp>
#include <memory>
#include <optional>

namespace Mlib {

enum class ScreenUnits;

struct TapButtonState {
    std::optional<int> key;
    std::optional<int> joystick_xaxis;
    std::optional<int> joystick_yaxis;
    std::unique_ptr<IWidget> widget;
};

}
