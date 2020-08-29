#pragma once
#include <Mlib/Array/Array_Forward.hpp>

namespace Mlib {

struct Aim {
    Aim(const FixedArray<float, 3>& gun_pos,
        const FixedArray<float, 3>& target_pos,
        float bullet_start_offset,
        float velocity,
        float gravity,
        float eps,
        float niterations);

    float angle0;
    float angle;
    float aim_offset;
    float time;
};

}
