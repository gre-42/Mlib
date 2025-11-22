#pragma once
#include <Mlib/Array/Array_Forward.hpp>
#include <Mlib/Scene_Config/Scene_Precision.hpp>

namespace Mlib {

struct Aim {
    Aim(const FixedArray<ScenePos, 3>& gun_pos,
        const FixedArray<ScenePos, 3>& target_pos,
        ScenePos bullet_start_offset,
        ScenePos velocity,
        ScenePos gravity,
        ScenePos eps,
        size_t niterations);

    ScenePos angle0;
    ScenePos angle;
    ScenePos aim_offset;
    ScenePos time;
};

}
