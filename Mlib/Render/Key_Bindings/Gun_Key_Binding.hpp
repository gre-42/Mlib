#pragma once
#include <Mlib/Render/Key_Bindings/Base_Key_Binding.hpp>
#include <string>

namespace Mlib {

class SceneNode;

struct GunKeyBinding {
    BaseKeyBinding base;
    SceneNode* node;
};

}
