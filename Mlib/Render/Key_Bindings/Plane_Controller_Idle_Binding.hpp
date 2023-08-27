#pragma once
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>

namespace Mlib {

class SceneNode;

struct PlaneControllerIdleBinding {
    DanglingPtr<SceneNode> node;
};

}
