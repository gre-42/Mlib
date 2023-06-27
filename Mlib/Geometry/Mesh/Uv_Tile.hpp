#pragma once
#include <Mlib/Array/Fixed_Array.hpp>

namespace Mlib {

struct UvTile {
    FixedArray<float, 2> position;
    FixedArray<float, 2> size;
};

}
