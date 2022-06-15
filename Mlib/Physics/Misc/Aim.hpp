#pragma once
#include <Mlib/Array/Array_Forward.hpp>

namespace Mlib {

struct Aim {
    Aim(const FixedArray<double, 3>& gun_pos,
        const FixedArray<double, 3>& target_pos,
        double bullet_start_offset,
        double velocity,
        double gravity,
        double eps,
        size_t niterations);

    double angle0;
    double angle;
    double aim_offset;
    double time;
};

}
