#pragma once
#include <Mlib/Math/Interp.hpp>

namespace Mlib {

struct TrailSequence {
    float u_offset;
    float u_scale;
    Interp<float> times_to_w;
};

}
