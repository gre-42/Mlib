#pragma once
#include <Mlib/Scene_Graph/Animation/Animation_Frame.hpp>
#include <Mlib/Variable_And_Hash.hpp>
#include <cmath>

namespace Mlib {

struct AnimationState {
    const VariableAndHash<std::string> periodic_skelletal_animation_name;
    const VariableAndHash<std::string> aperiodic_skelletal_animation_name;
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
