#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <string>

namespace Mlib {

class SceneNode;

struct AbsoluteMovableIdleBinding {
    SceneNode* node;
    FixedArray<float, 3> tires_z;
};

}
