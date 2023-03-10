#pragma once
#include <Mlib/Render/Key_Bindings/Base_Key_Combination.hpp>

namespace Mlib {

class SceneNode;

struct PlayerKeyBinding {
    std::string id;
    std::string role;
    SceneNode* node;
    bool select_next_opponent;
    bool select_next_vehicle;
};

}
