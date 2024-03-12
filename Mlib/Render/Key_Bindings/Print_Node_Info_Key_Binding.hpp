#pragma once
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Render/Ui/Button_Press.hpp>
#include <functional>

namespace Mlib {

class SceneNode;

struct PrintNodeInfoKeyBinding {
    std::function<DanglingPtr<SceneNode>()> dynamic_node;
    ButtonPress button_press;
};

}
