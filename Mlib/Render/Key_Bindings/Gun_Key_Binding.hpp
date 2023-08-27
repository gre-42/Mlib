#pragma once
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <string>

namespace Mlib {

class SceneNode;

struct GunKeyBinding {
    std::string id;
    std::string role;
    DanglingPtr<SceneNode> node;
    Player* player;
};

}
