#pragma once
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <functional>
#include <string>

namespace Mlib {

class SceneNode;

struct PrintNodeInfoKeyBinding {
    std::string id;
    std::string role;
    DanglingPtr<SceneNode> fixed_node;
    std::function<DanglingPtr<SceneNode>()> dynamic_node;
};

}
