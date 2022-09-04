#pragma once
#include <Mlib/Render/Key_Bindings/Base_Key_Combination.hpp>
#include <string>

namespace Mlib {

class SceneNode;

struct GunKeyBinding {
    BaseKeyCombination base_combo;
    SceneNode* node;
    Player* player;
};

}
