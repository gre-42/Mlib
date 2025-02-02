#pragma once
#include <Mlib/Ignore_Copy.hpp>
#include <Mlib/Threads/Safe_Atomic_Shared_Mutex.hpp>
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

class PeriodicAnimationFrame {
public:
    PeriodicAnimationFrame(const AnimationFrame& frame)
        : frame_{ frame }
    {}
    float time() const;
    void advance_time(float dt);
private:
    AnimationFrame frame_;
    mutable IgnoreCopy<SafeAtomicSharedMutex> mutex_;
};

class AperiodicAnimationFrame {
public:
    AperiodicAnimationFrame(const AnimationFrame& frame)
        : frame_{ frame }
    {}
    float time() const;
    void advance_time(float dt);
    bool active() const;
    bool ran_to_completion() const;
    float duration() const;
    float elapsed() const;
    bool is_nan() const;
private:
    AnimationFrame frame_;
    mutable IgnoreCopy<SafeAtomicSharedMutex> mutex_;
};

}
