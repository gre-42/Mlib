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
    bool is_nan() const;
};

struct PeriodicAnimationFrame {
    AnimationFrame frame;
    void advance_time(float dt);
};

struct AperiodicAnimationFrame {
    AnimationFrame frame;
    void advance_time(float dt);
    bool active() const;
    bool ran_to_completion() const;
};

}
