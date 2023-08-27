#pragma once
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>

namespace Mlib {

class SceneNode;

struct AvatarControllerIdleBinding {
    DanglingPtr<SceneNode> node;
};

}
