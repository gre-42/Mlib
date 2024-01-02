#pragma once
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Render/Ui/Button_Press.hpp>

namespace Mlib {

class SceneNode;

struct GunKeyBinding {
    DanglingPtr<SceneNode> node;
    Player* player;
    ButtonPress button_press;
};

}
