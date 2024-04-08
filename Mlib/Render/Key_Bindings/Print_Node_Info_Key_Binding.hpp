#pragma once
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <Mlib/Render/Ui/Button_Press.hpp>
#include <functional>

namespace Mlib {

class SceneNode;

struct PrintNodeInfoKeyBinding {
    std::function<DanglingPtr<SceneNode>()> dynamic_node;
    ButtonPress button_press;
    DestructionFunctionsRemovalTokens on_destroy_key_bindings;
};

}
