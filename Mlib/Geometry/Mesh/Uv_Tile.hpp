#pragma once
#include <Mlib/Array/Fixed_Array.hpp>

namespace Mlib {

struct ManualUvTile {
    FixedArray<float, 2> position;
    FixedArray<float, 2> size;
};

struct AutoUvTile {
    FixedArray<float, 2> position;
    FixedArray<float, 2> size;
    uint8_t layer;
};

}
