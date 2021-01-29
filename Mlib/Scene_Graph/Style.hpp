#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene_Graph/Animation_Frame.hpp>
#include <regex>

namespace Mlib {

struct Style {
    Mlib::regex selector;
    FixedArray<float, 3> ambience{-1.f, -1.f, -1.f};
    FixedArray<float, 3> diffusivity{-1.f, -1.f, -1.f};
    FixedArray<float, 3> specularity{-1.f, -1.f, -1.f};
    AnimationFrame animation_frame = {
        .name = "",
        .loop_begin = NAN,
        .loop_end = NAN,
        .loop_time = NAN};
};

}
