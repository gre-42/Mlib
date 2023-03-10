#pragma once
#include <string>

namespace Mlib {

class SceneNode;

struct GunKeyBinding {
    std::string id;
    std::string role;
    SceneNode* node;
    Player* player;
};

}
