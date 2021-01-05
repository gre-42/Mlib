#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Scene_Graph/Animation_Frame.hpp>
#include <regex>

namespace Mlib {

struct Style {
    std::regex selector;
    FixedArray<float, 3> ambience{-1, -1, -1};
    FixedArray<float, 3> diffusivity{-1, -1, -1};
    FixedArray<float, 3> specularity{-1, -1, -1};
    AnimationFrame animation_frame = {
        .name = "",
        .loop_begin = NAN,
        .loop_end = NAN,
        .loop_time = NAN};
};

}
