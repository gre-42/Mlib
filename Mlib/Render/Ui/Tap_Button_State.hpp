#pragma once
#include <Mlib/Layout/IWidget.hpp>
#include <memory>

namespace Mlib {

enum class ScreenUnits;

struct TapButtonState {
    std::unique_ptr<IWidget> widget;
    bool pressed;
};

}
