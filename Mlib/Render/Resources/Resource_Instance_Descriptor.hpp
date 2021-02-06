#pragma once
#include <Mlib/Array/Array_Forward.hpp>
#include <string>

namespace Mlib {

struct ObjectResourceDescriptor {
    FixedArray<float, 3> position;
    std::string name;
    float scale = 1;
};

struct ResourceInstanceDescriptor {
    FixedArray<float, 3> position;
    float scale = 1;  // Currently not used
};

}
