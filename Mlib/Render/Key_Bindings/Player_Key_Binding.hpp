#pragma once
#include <Mlib/Render/Key_Bindings/Base_Key_Binding.hpp>

namespace Mlib {

class SceneNode;

struct PlayerKeyBinding {
    BaseKeyBinding base_key;
    SceneNode* node;
    bool select_next_opponent;
};

}
