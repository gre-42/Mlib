#pragma once
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <string>

namespace Mlib {

class SceneNode;

struct WeaponCycleKeyBinding {
    std::string id;
    std::string role;
    DanglingPtr<SceneNode> node;
    int direction;
};

}
