#pragma once
#include <string>

namespace Mlib {

class SceneNode;

struct WeaponCycleKeyBinding {
    std::string id;
    std::string role;
    SceneNode* node;
    int direction;
};

}
