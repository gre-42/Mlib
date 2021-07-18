#pragma once
#include <string>

namespace Mlib {

enum class AnimationWrapMode {
    PERIODIC,
    APERIODIC
};

struct AnimationFrame {
    float begin;
    float end;
    float time;
    void advance_time(float dt, AnimationWrapMode wrap_mode);
};

}
