#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <string>

namespace Mlib {

struct AbsoluteMovableIdleBinding {
    std::string node;
    FixedArray<float, 3> tires_z;
};

}
