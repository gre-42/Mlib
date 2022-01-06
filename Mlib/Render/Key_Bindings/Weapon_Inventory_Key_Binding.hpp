#pragma once
#include <Mlib/Render/Key_Bindings/Base_Cursor_Axis_Binding.hpp>
#include <Mlib/Render/Key_Bindings/Base_Key_Binding.hpp>
#include <Mlib/Render/Ui/Cursor_Movement.hpp>
#include <memory>
#include <string>

namespace Mlib {

class SceneNode;

struct WeaponInventoryKeyBinding {
    BaseKeyBinding base_key;
    BaseCursorAxisBinding base_scroll_wheel_axis;
    std::shared_ptr<CursorMovement> scroll_wheel_movement;
    SceneNode* node;
    int direction;
};

}
