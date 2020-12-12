#pragma once
#include <Mlib/Array/Fixed_Array.hpp>

namespace Mlib {

struct SpawnPoint {
    FixedArray<float, 3> position;
    FixedArray<float, 3> rotation;
};

}
