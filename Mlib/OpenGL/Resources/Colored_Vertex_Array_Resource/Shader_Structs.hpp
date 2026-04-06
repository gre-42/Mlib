#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <cstddef>

namespace Mlib {

static const size_t ANIMATION_NINTERPOLATED = 4;
struct ShaderBoneWeight {
    unsigned char indices[ANIMATION_NINTERPOLATED];
    float weights[ANIMATION_NINTERPOLATED];
};

struct ShaderInteriorMappedFacade {
    FixedArray<float, 3> bottom_left;
    FixedArray<float, 4> uvmap;
};

}
