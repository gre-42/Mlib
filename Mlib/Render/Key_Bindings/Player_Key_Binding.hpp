#pragma once
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Render/Key_Bindings/Base_Key_Combination.hpp>
#include <Mlib/Render/Ui/Button_Press.hpp>

namespace Mlib {

class SceneNode;

struct PlayerKeyBinding {
    DanglingPtr<SceneNode> node;
    bool select_next_opponent;
    bool select_next_vehicle;
    ButtonPress button_press;
};

}
