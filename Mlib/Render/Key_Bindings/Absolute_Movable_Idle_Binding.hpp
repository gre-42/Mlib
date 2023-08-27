#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <string>

namespace Mlib {

class SceneNode;

struct AbsoluteMovableIdleBinding {
    DanglingPtr<SceneNode> node;
    FixedArray<float, 3> tires_z;
};

}
