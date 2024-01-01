#pragma once
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Render/Ui/Button_Press.hpp>
#include <Mlib/Render/Ui/Cursor_Movement.hpp>
#include <string>

namespace Mlib {

class SceneNode;

struct WeaponCycleKeyBinding {
    std::string id;
    std::string role;
    DanglingPtr<SceneNode> node;
    int direction;
    ButtonPress button_press;
    std::shared_ptr<CursorMovement> scroll_wheel_movement;
};

}
