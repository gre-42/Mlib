#pragma once
#include <string>

namespace Mlib {

class SceneNode;

struct WeaponCycleKeyBinding {
    std::string id;
    SceneNode* node;
    int direction;
};

}
