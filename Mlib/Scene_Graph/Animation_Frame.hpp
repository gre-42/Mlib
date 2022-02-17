#pragma once
#include <string>

namespace Mlib {

enum class AnimationWrapMode {
    PERIODIC,
    APERIODIC
};

struct AnimationFrame {
    AnimationWrapMode wrap_mode;
    float begin;
    float end;
    float time;
    void advance_time(float dt);
    bool active() const;
};

}
