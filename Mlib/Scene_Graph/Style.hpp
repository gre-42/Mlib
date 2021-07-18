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
    std::string skelletal_animation_name = "";
    AnimationFrame skelletal_animation_frame = {
        .begin = NAN,
        .end = NAN,
        .time = NAN};
    AnimationFrame texture_animation = {
        .begin = NAN,
        .end = NAN,
        .time = NAN};
};

}
