#pragma once
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Render/Ui/Button_Press.hpp>
#include <string>

namespace Mlib {

class SceneNode;

struct GunKeyBinding {
    std::string id;
    std::string role;
    DanglingPtr<SceneNode> node;
    Player* player;
    ButtonPress button_press;
};

}
