#pragma once
#include <Mlib/Scene_Graph/Animation/Animation_Frame.hpp>
#include <cmath>

namespace Mlib {

struct AnimationState {
    const std::string periodic_skelletal_animation_name = "";
    const std::string aperiodic_skelletal_animation_name = "";
    PeriodicAnimationFrame periodic_skelletal_animation_frame {
        AnimationFrame{
            .begin = NAN,
            .end = NAN,
            .time = NAN}};
    AperiodicAnimationFrame aperiodic_animation_frame {
        AnimationFrame{
            .begin = NAN,
            .end = NAN,
            .time = NAN}};
    const bool delete_node_when_aperiodic_animation_finished = false;
};

}
